#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <string.h>
//--------------------------
#include <pthread.h>
//#include "public.h"
#define BUFLEN 1024

typedef struct
{
    unsigned long long id; //package's id
    unsigned char data[BUFLEN];    //audio data
} Package;

int sock_phone,sock_home;     //socket
struct sockaddr_in addr_phone,addr_home;  //address
struct sockaddr_in addr_phone_temp,addr_home_temp;  //address cache

void *thread_phone_to_home_main()
{
    Package pack;
    int pack_len = sizeof(Package);
    char buf_sock[pack_len];

    int n;
    int len = sizeof(addr_phone_temp);
    while(1)
    {
        n = recvfrom(sock_phone, buf_sock, pack_len, 0, (struct sockaddr *)&addr_phone_temp, &len);
        //printf("%s\n",inet_ntoa(addr_phone.sin_addr.s_addr));
        if (n == -1)
        {
            perror("recvfrom error");
        }
        else if(n > 0)
        {
            memcpy(&pack,buf_sock,pack_len);
            //printf("phone to home%llu:%s\n",pack.id,pack.data);
            //printf("1");
            sendto(sock_home, buf_sock, pack_len, 0, (struct sockaddr *)&addr_home_temp, sizeof(addr_home_temp));
        }
    }
}

void *thread_home_to_phone_main()
{
    Package pack;
    int pack_len = sizeof(Package);
    char buf_sock[pack_len];

    int n;
    int len;
    len = sizeof(addr_home_temp);
    while(1)
    {
        n = recvfrom(sock_home, buf_sock, pack_len, 0, (struct sockaddr *)&addr_home_temp, &len);
        if (n == -1)
        {
            perror("recvfrom error");
        }
        else if(n > 0)
        {
            memcpy(&pack,buf_sock,pack_len);
            //printf("home_to_phone%llu:%s\n",pack.id,pack.data);
            //printf("%d %d %d %d %d %d %d %d\n",buf_sock[0],buf_sock[1],buf_sock[2],buf_sock[3],buf_sock[4],buf_sock[5],buf_sock[6],buf_sock[7]);
            sendto(sock_phone, buf_sock, pack_len, 0, (struct sockaddr *)&addr_phone_temp, sizeof(addr_phone_temp));
        }
    }
}

void initSock()
{
    if ((sock_phone = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("create sock_phone failed!");

    memset(&addr_phone, 0, sizeof(addr_phone));
    addr_phone.sin_family = AF_INET;
    addr_phone.sin_port = htons(8081);
    addr_phone.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_phone, (struct sockaddr *)&addr_phone, sizeof(addr_phone)) < 0)
        perror("bind error");


    if ((sock_home = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("create sock_home failed!");

    memset(&addr_home, 0, sizeof(addr_home));
    addr_home.sin_family = AF_INET;
    addr_home.sin_port = htons(8082);
    addr_home.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_home, (struct sockaddr *)&addr_home, sizeof(addr_home)) < 0)
        perror("bind error");
}

int main()
{
    initSock();

    pthread_t thread_phone_to_home,thread_home_to_phone;
    memset(&thread_phone_to_home, 0, sizeof(thread_phone_to_home));
    memset(&thread_home_to_phone, 0, sizeof(thread_home_to_phone));

    if ((pthread_create(&thread_phone_to_home, NULL, thread_phone_to_home_main, NULL)) < 0)
        perror("create thread_phone_to_home_main failed!");
    if ((pthread_create(&thread_home_to_phone, NULL, thread_home_to_phone_main, NULL)) < 0)
        perror("create thread_home_to_phone_main failed!");


    if (thread_phone_to_home != 0)
        pthread_join(thread_phone_to_home,NULL);//等待线程退出
    if (thread_home_to_phone != 0)
        pthread_join(thread_home_to_phone,NULL);//等待线程退出

    close(sock_home);
    close(sock_phone);
    return 0;
}
