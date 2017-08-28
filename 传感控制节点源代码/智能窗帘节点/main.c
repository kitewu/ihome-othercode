#include <AT89X52.h>

unsigned char code TAB[8]=
{                          //������һ��Ҫʹ��code����������������洢����
    0x7F,                  //����1������0b01111111
    0x3F,                  //����2������0b00111111
    0xBF,                  //���� 3������0b10111111
    0x9F,                  //����4������0b10011111
    0xDF,                  //����5������0b11011111
    0xCF,                  //����6������0b11001111
    0xEF,                  //����7������0b11101111
    0x6F,                  //����8������0b01101111
};

const unsigned int circle =	1024;
unsigned char index;
long beats;
unsigned char msgIndex;
unsigned char msg[] = {0x00, 0x00, 0x00};
unsigned char g;


void ConfigUART(unsigned int baud);
void TurnMotor();
void HandleMsg();

void main(void)           
{
	
    //***��ʱ��Timer0��ʼ��***
    TMOD&=0xF0;            //��TMOD�ĵ�4λ��ʱ��0���Ʋ�������
    TMOD|=0x01;            //���ö�ʱ��0Ϊ��ʽ1
    TL0=0x99;              //���ö�ʱ��0��ֵ��8λ
    TH0=0xF1;              //���ö�ʱ��0��ֵ��8λ
    TR0=1;                 //������ʱ��0
    ET0=1;                 //Timer0�ж�����
    //**********************
	EA = 1;//�����ж�
	ConfigUART(9600);
	beats = 0;
	index = 0;
	msgIndex = 0;
	g = 0x00;

	while(1);
}

void ConfigUART(unsigned int baud)
{
	SCON = 0x50; //���ô���Ϊģʽ 1
    TMOD &= 0x0F; //���� T1 �Ŀ���λ
    TMOD |= 0x20; //���� T1 Ϊģʽ 2
    TH1 = 256 - (11059200/12/32)/baud; //���� T1 ����ֵ
    TL1 = TH1; //��ֵ��������ֵ
    ET1 = 0; //��ֹ T1 �ж�
    ES = 1; //ʹ�ܴ����ж�
    TR1 = 1; //���� T1
}

void TurnMotor()
{
	if(beats > 0) // ��ת
	{
		index++;
		beats--;
		index = index & 0x07;
		P1 = TAB[index];	
	}
	else if(beats < 0)
	{
		 index--;
		 beats++;
		 index = index&0x07;
		 P1 =	TAB[index];
	}
	else
	{
	}	
}


void Timer0(void) interrupt 1 
{                          //��ʱ2000΢��
    TL0=0x99;              //���¸�TL0����ֵ
    TH0=0xF1;              //���¸�TH0����ֵ

 	TurnMotor(); 
}

void InterruptUART() interrupt 4
{
    if (RI){ //���յ��ֽ�

		P0 = g;
		g++;

        RI = 0; //�ֶ���������жϱ�־λ
    
		msg[msgIndex] = SBUF;
		SBUF = msg[msgIndex];

		if((++msgIndex)	== 3)
		{
			msgIndex = 0;
			if(beats == 0) HandleMsg();
			msg[0] = msg[1] = msg[2] = 0;
		}
		

    }
    if (TI){ //�ֽڷ������
        TI = 0; //�ֶ����㷢���жϱ�־λ
    }
}

void HandleMsg()
{
	if(msg[2] == msg[0]^msg[1])
	{
		beats = msg[1]*512u;
		if(!msg[0]) beats = -beats; //0 respresent reverse	
	}
}