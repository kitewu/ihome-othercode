#include <AT89X52.h>

unsigned char code TAB[8]=
{                          //定义表格一定要使用code，这样会做到程序存储区中
    0x7F,                  //表格第1步数据0b01111111
    0x3F,                  //表格第2步数据0b00111111
    0xBF,                  //表格第 3步数据0b10111111
    0x9F,                  //表格第4步数据0b10011111
    0xDF,                  //表格第5步数据0b11011111
    0xCF,                  //表格第6步数据0b11001111
    0xEF,                  //表格第7步数据0b11101111
    0x6F,                  //表格第8步数据0b01101111
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
	
    //***定时器Timer0初始化***
    TMOD&=0xF0;            //将TMOD的低4位定时器0控制部分清零
    TMOD|=0x01;            //设置定时器0为方式1
    TL0=0x99;              //设置定时器0初值低8位
    TH0=0xF1;              //设置定时器0初值高8位
    TR0=1;                 //启动定时器0
    ET0=1;                 //Timer0中断允许
    //**********************
	EA = 1;//打开总中断
	ConfigUART(9600);
	beats = 0;
	index = 0;
	msgIndex = 0;
	g = 0x00;

	while(1);
}

void ConfigUART(unsigned int baud)
{
	SCON = 0x50; //配置串口为模式 1
    TMOD &= 0x0F; //清零 T1 的控制位
    TMOD |= 0x20; //配置 T1 为模式 2
    TH1 = 256 - (11059200/12/32)/baud; //计算 T1 重载值
    TL1 = TH1; //初值等于重载值
    ET1 = 0; //禁止 T1 中断
    ES = 1; //使能串口中断
    TR1 = 1; //启动 T1
}

void TurnMotor()
{
	if(beats > 0) // 正转
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
{                          //定时2000微秒
    TL0=0x99;              //重新给TL0赋初值
    TH0=0xF1;              //重新给TH0赋初值

 	TurnMotor(); 
}

void InterruptUART() interrupt 4
{
    if (RI){ //接收到字节

		P0 = g;
		g++;

        RI = 0; //手动清零接收中断标志位
    
		msg[msgIndex] = SBUF;
		SBUF = msg[msgIndex];

		if((++msgIndex)	== 3)
		{
			msgIndex = 0;
			if(beats == 0) HandleMsg();
			msg[0] = msg[1] = msg[2] = 0;
		}
		

    }
    if (TI){ //字节发送完毕
        TI = 0; //手动清零发送中断标志位
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