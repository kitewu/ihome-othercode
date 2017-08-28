#include "ioCC2530.h"
#include "hal_mcu.h"

#define noACK 0
#define ACK   1

#define STATUS_REG_W 0x06
#define STATUS_REG_R 0x07
#define MEASURE_TEMP 0x03
#define MEASURE_HUMI 0x05
#define RESET        0x1e

#define SCL          P1_0 
#define SDA          P1_1


#define IO_DIR_PORT_PIN(port, pin, dir)  \
   do {                                  \
      if (dir == IO_OUT)                 \
         P##port##DIR |= (0x01<<(pin));  \
      else                               \
         P##port##DIR &= ~(0x01<<(pin)); \
   }while(0)


#define IO_IN   0
#define IO_OUT  1
unsigned char d1,d2,d3,d4,d5,d6,d7;

//extern float humivalue_test(void);
//extern float tempvalue_test(void);
//extern void th_test(int *t,int *h );
//static void Wait(unsigned int ms);
static void QWait(void)  ;
extern char s_write_byte(unsigned char value);
extern char s_read_byte(unsigned char ack);
static void s_transstart(void);
static void s_connectionreset(void);
static char s_measure( unsigned char *p_checksum, unsigned char mode);
static void initIO(void);
void th_read(int *t,int *h );


//void Wait(unsigned int ms)
//{
                    
//   unsigned char g,k;
//   while(ms)
//   {
      
//	  for(g=0;g<=167;g++)
	//   {
	//     for(k=0;k<=48;k++);
	//   }
 //     ms--;                            
 //  }
//} 

void QWait()     //1us的延时
{
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");asm("NOP");
    asm("NOP");

}

void initIO(void)
{
  P1INP |= 0x03;
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  SDA = 1; SCL = 0;
}





/**************************************************************************************************
 * 函数名称：s_transstart
 *
 * 功能描述：启动SHT10，开始与SHT10通信
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void s_transstart(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
   SDA = 1; SCL = 0;
   QWait();QWait();
   SCL = 1;QWait();QWait();
   SDA = 0;QWait();QWait(); 
   SCL = 0;QWait();QWait();QWait();QWait();QWait();
   SCL = 1;QWait();QWait();
   SDA = 1;QWait();QWait();
   SCL = 0;QWait();QWait();
}
/**************************************************************************************************
 * 函数名称：s_connectionreset
 *
 * 功能描述：与SHT10通信复位
 *
 * 参    数：无
 *
 * 返 回 值：无
 **************************************************************************************************/
void s_connectionreset(void)
{
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  unsigned char i;
  SDA = 1; SCL= 0;
  for(i=0;i<9;i++)
  {
    SCL = 1;QWait();QWait();
    SCL = 0;QWait();QWait();
  }
  s_transstart();
}
/**************************************************************************************************
 * 函数名称：s_measure
 *
 * 功能描述：发送命令、读取SHT10温度或湿度数据
 *
 * 参    数：*p_checksum -- 校验和
 *           mode -- 读取数据类型（3为温度，5为湿度）
 *
 * 返 回 值：er -- 操作结果
 **************************************************************************************************/

char s_measure( unsigned char *p_checksum, unsigned char mode)
{
  unsigned er=0;
  unsigned int i,j;
  s_transstart();
  switch(mode)
  {
    case 3	:er+=s_write_byte(3);break;
    case 5	:er+=s_write_byte(5);break;
    default     :break;
  }
  IO_DIR_PORT_PIN(1, 1, IO_IN);
  for(i=0;i<65535;i++)
  {
    for(j=0;j<65535;j++)
    {if(SDA == 0)
    {break;}}
    if(SDA == 0)
    {break;}
  }
  
  if(SDA)
    
  {er += 1;}
  d1 = s_read_byte(ACK);
  d2 = s_read_byte(ACK);
  d3 = s_read_byte(noACK);
  //*(p_value) = s_read_byte(ACK);
  //*(p_value+1) = s_read_byte(ACK);
 //*p_checksum = s_read_byte(noACK);
 // d6 = *(p_value);d7=*(p_value+1);
  return er;
}
/**************************************************************************************************
 * 函数名称：th_read
 *
 * 功能描述：调用相应函数，读取温度和数据数据并校验和计算
 *
 * 参    数：*t -- 温度值
 *           *h -- 湿度值
 *
 * 返 回 值：无
 **************************************************************************************************/

void th_read(int *t,int *h )
{
  unsigned char error,checksum;
  float humi,temp;
  int tmp;
  initIO();
  s_connectionreset();
    error=0;
   error+=s_measure(&checksum,5);
    humi = d1*256+d2;
    
    error+=s_measure(&checksum,3);
    temp = d1*256+d2;
   if (humi)
   {
    if(error!=0) 
    {
      s_connectionreset();
    }
    else
    {      
       temp = temp*0.01  -  44.0 ;
       humi = (temp - 25) * (0.01 + 0.00008 * humi) -0.0000028 * humi * humi + 0.0405 * humi-4;
       if(humi>100)
       {humi = 100;}
       if(humi<0.1)
       {humi = 0.1;}
    }
    
    tmp=(int)(temp*10)%10;
    
    if(tmp>4)
    {
     temp=temp+1; 
    }
    else
    {
       temp=temp;
    }
    
  *t=(int)temp;
  
   tmp=(int)(humi*10)%10;
    
    if(tmp>4)
    {
     humi=humi+1; 
    }
    else
    {
       humi=humi;
    }
    
  *h=(int)humi;
   }
   else
   {
     *t = 0;
     *h = 0;
   }
  
  IO_DIR_PORT_PIN(1, 0, IO_OUT);
  IO_DIR_PORT_PIN(1, 1, IO_OUT);
  SDA = 1; SCL= 1;
}



//int data1 = 0,data2=0;
//void main()
//{
  //HAL_BOARD_INIT(); 
//  while(1)
//  {
//  int tempera;
//  int humidity;
//  th_read(&tempera,&humidity);
//  data1=(int)tempera;
//  data2=(int)humidity;
//  }
//}
