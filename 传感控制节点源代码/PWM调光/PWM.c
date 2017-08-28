
/******************************************************************************************
*                                       INCLUDES                                          *
******************************************************************************************/

#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
#include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"
#include "Public.h"
#include "PWM.h"
#include "hal_timer.h"
#include <stdio.h>			// for <printf>
#include <ioCC2530.h>

/*****************************************************************************************
 *                                         MACROS
 *****************************************************************************************/
#define uint unsigned int
#define uchar unsigned char

#define PWM_OUT1()          st(P0DIR|=0x08;P0_3=1;) 
#define PWM_OUT0()          st(P0DIR|=0x08;P0_3=0;)

#define INIT_IOS()    	    st(P1DIR&=~0x80;P1DIR|=0x78;)   

/******************************************************************************************
 *                                        CONSTANTS
 ******************************************************************************************/
unsigned char DutyRatio=0;
unsigned char counter=0;
float fP,fV,fI;
long  lP,lV,lI;
uint count;//用于定时器计数	


#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
#define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SERIAL_APP_LOOPBACK )
#define SERIAL_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  20
#endif

#define SERIAL_APP_RSP_CNT  4

// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[SERIALAPP_MAX_CLUSTERS] =
{
  SERIALAPP_CLUSTERID1,
  SERIALAPP_CLUSTERID2
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
  SERIALAPP_ENDPOINT,              //  int   Endpoint;
  SERIALAPP_PROFID,                //  uint16 AppProfId[2];
  SERIALAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SERIALAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SERIALAPP_FLAGS,                 //  int   AppFlags:4;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SerialApp_ClusterList,  //  byte *pAppInClusterList;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)SerialApp_ClusterList   //  byte *pAppOutClusterList;
};

const endPointDesc_t SerialApp_epDesc =
{
  SERIALAPP_ENDPOINT,
 &SerialApp_TaskID,
  (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
  noLatencyReqs
};

/******************************************************************************************
 *                                       TYPEDEFS
 ******************************************************************************************/

/*******************************************************************************************
 *                                  GLOBAL VARIABLES
 *******************************************************************************************/

uint8 SerialApp_TaskID;    // Task ID for internal task/event processing.
devStates_t  SampleApp_NwkState;
static UART_Format UART0_Format;

/*******************************************************************************************
 *                                 EXTERNAL VARIABLES
 ******************************************************************************************/

/*******************************************************************************************
 *                                 EXTERNAL FUNCTIONS
 *******************************************************************************************/

/********************************************************************************************
 *                                   LOCAL VARIABLES
 ********************************************************************************************/

static uint8 SerialApp_MsgID;
static afAddrType_t SerialApp_TxAddr;
static uint8 SerialApp_TxBuf[SERIAL_APP_TX_MAX];
static uint8 SerialApp_TxLen;

/********************************************************************************************
 *                                  LOCAL FUNCTIONS
 *********************************************************************************************/

static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SerialApp_OTAData(afAddrType_t *txaddr,uint8 ID,void *p,uint8 len);
static uint8 CheckSum(uint8 *data,uint8 len);
void Delayms(uint xms);		//延时函数
void InitPWM(void);		//初始化P1口
void InitT3();                  //初始化定时器T3
extern void ledInit(void);
extern void FLASHLED(uint8 FLASHnum);
extern void LED(uint8 led,uint8 operation);

/*****************************************************************************************
*函数名称 ：延时函数				                                 
*入口参数 ：xms - 需要延时的毫秒数                                
*返 回 值 ：无                                                                               
*说    明 ：                                                             
*****************************************************************************************/

void Delayms(uint xms)   //i=xms 即延时i毫秒
{
 uint i,j;
 for(i=xms;i>0;i--)
   for(j=587;j>0;j--);
} 
/*****************************************************************************************
*函数名称 ：PWM初始化函数				                                 
*入口参数 ：无                                
*返 回 值 ：无                                                                               
*说    明 ：无                                                             
*****************************************************************************************/
void InitPWM(void)
{
	P0DIR |= 0x08; //P0_3义为输出
	P0SEL &= ~0x08;
}

/*****************************************************************************************
*函数名称 ：定时器3初始化函数				                                 
*入口参数 ：无                                
*返 回 值 ：无                                                                                
*说    明 ：对定时器3进行初始化                                                            
*****************************************************************************************/

void InitT3()
{     
  
      IP0 |=0X08;
      IP1 |=0X08;
  
      T3CTL |= 0x08 ;             //开溢出中断     
      T3IE = 1;                   //开总中断和T3中断
      T3CTL|=0XE0;               //128分频,128/16000000*N=0.5S,N=65200
      T3CTL &= ~0X02;            //自动重装 00－>0xff  65200/256=254(次)
      T3CC0 = 0x02;
      T3CTL |=0X10;//启动
      EA = 1; 
}


/*****************************************************************************************
*函数名称 ：串口初始化函数					                          
*入口参数 ：task_id - OSAL分配的任务ID号		                          
*返 回 值 ：无							                          
*说    明 ：在OSAL初始化的时候被调用                             
*****************************************************************************************/

void SerialApp_Init( uint8 task_id )
{ 
    
    InitPWM();		
    InitT3();   
    ledInit();
    
    halUARTCfg_t uartConfig;
  
    SerialApp_TaskID = task_id;
  
    afRegister( (endPointDesc_t *)&SerialApp_epDesc );
  
    RegisterForKeys( task_id );
  
  
    UART0_Format.Header   = '@';
    UART0_Format.Len      = 0x10;
    UART0_Format.NodeSeq  = 0x01;
    UART0_Format.NodeID   = LED_PWM;
  
  
    SerialApp_TxAddr.addrMode =(afAddrMode_t)Addr16Bit;//发送地址初始化
    SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
    SerialApp_TxAddr.addr.shortAddr = 0x0000;
    TXPOWER = 0xf5;
 }

/*****************************************************************************************
*函数名称 ：用户任务事件处理函数				                          
*入口参数 ：task_id - OSAL分配的事件ID号                                      
*           events  - 事件                                      
*返 回 值 ：事件标志		                          
*说    明 ：                                                                              
*****************************************************************************************/

UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{ 
  uint8 num=0;
  (void)task_id;  // Intentionally unreferenced parameter
  
  if ( events & SYS_EVENT_MSG )
  {
    afIncomingMSGPacket_t *MSGpkt;

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SerialApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {         
      case KEY_CHANGE:
        //SerialApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
        break;

      case AF_INCOMING_MSG_CMD:
        SerialApp_ProcessMSGCmd( MSGpkt );
        FLASHLED(3);
        break;

      case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if(SampleApp_NwkState == DEV_END_DEVICE)       //判定当前设备类型
          {
            
            
            
            osal_set_event(SerialApp_TaskID, PERIOD_EVT); //启动周期消息
          }
          LED(1,ON);
        break;
      default:
        break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }
  
  if ( events & PERIOD_EVT ) //周期消息处理
  {
    UART0_Format.Command = MSG_PERIOD;
    num = CheckSum(&UART0_Format.Header,UART0_Format.Len);
    UART0_Format.Verify  = num;

    SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
    FLASHLED(2);
    return ( events ^ PERIOD_EVT );
  }

  if ( events & SERIALAPP_SEND_EVT )  //将串口数据通过RF消息发送
  { 
    num = CheckSum(&UART0_Format.Header,UART0_Format.Len);
    UART0_Format.Verify  = num;

    SerialApp_OTAData(&SerialApp_TxAddr,SERIALAPP_CLUSTERID1,SerialApp_TxBuf, SerialApp_TxLen);
    FLASHLED(4);
    return ( events ^ SERIALAPP_SEND_EVT );
  }

  return ( 0 );  // Discard unknown events.
}


/*****************************************************************************************
*函数名称 ：消息处理函数				                                 
*入口参数 ：pkt   - 指向接收到的无线消息数据包的指针                                 
*返 回 值 ：TRUE  - 如果指针被应用并释放            
*           FALSE - 其他	                                                          
*说    明 ：处理收到的无线消息，并把消息通过串口发送给网关                               
*****************************************************************************************/

void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )  //处理接收到的RF消息
{ 
  uint8 num1,num2=0;
  static UART_Format_Control *receiveData;
  
  static UART_Format_End1 Rsp;
  Rsp.Header   = '@';
  Rsp.Len      = 0x07;
  Rsp.NodeSeq  = 0x01;
  Rsp.NodeID   = LED_PWM;
  Rsp.Command  = MSG_RSP;
  
  switch ( pkt->clusterId )
  {
     case SERIALAPP_CLUSTERID1:  //处理各个传感器节数据    
     receiveData = (UART_Format_Control *)(pkt->cmd.Data);
     HalLedBlink(HAL_LED_1,1,50,200);
     num1 = CheckSum((uint8*)receiveData,receiveData->Len);
     
     if((receiveData->Header==0x40)&&(receiveData->Verify==num1)) //校验包头包尾

     { 
        if(receiveData->NodeID == LED_PWM) //地址
       {
       //占空比调节
           if(receiveData->Command != MSG_RSC)
           {
             while(DutyRatio != receiveData->Command)
             {
               DutyRatio  = receiveData->Command;
             }
               
               Rsp.Data[0]= DutyRatio;//返回当前占空比
           }
           if(receiveData->Command == MSG_RSC)
           {
               Rsp.Data[0]= DutyRatio;//返回当前占空比
           }
       
       }
       num2 = CheckSum(&Rsp.Header,Rsp.Len);
       Rsp.Verify  = num2;
       SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &Rsp, sizeof(UART_Format_End1));
       FLASHLED(4);
     }
    break;

  case SERIALAPP_CLUSTERID2:
    break;

    default:
      break;
  }
}

/*****************************************************************************************
*函数名称 ：无线消息发送函数				                                 
*入口参数 ：*txaddr - 发送地址
*           cID     - clusterID
*           *p      - 发送数据包的地址
*           len     - 发送数据包的长度
*返 回 值 ：无                                                                                
*说    明 ：把组好的数据包，通过无线发送出去                                                            
*****************************************************************************************/


void SerialApp_OTAData(afAddrType_t *txaddr, uint8 cID, void *p, uint8 len) //发送函数
{
  if (afStatus_SUCCESS != AF_DataRequest(txaddr, //发送地址
                                           (endPointDesc_t *)&SerialApp_epDesc, //endpoint描述
                                            cID, //clusterID
                                            len, p, //发送数据包的长度和地址
                                            &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
  {
  }
  else
  {
    HalLedBlink(HAL_LED_1,1,50,200);
  }
}



/*****************************************************************************************
*函数名称 ：累加校验和函数				                                 
*入口参数 ：*data - 数据包地址
            len   - 数据包长度
*返 回 值 ：sum   - 累加和                                                                                
*说    明 ：对数据包进行累加和校验                                                             
*****************************************************************************************/

static uint8 CheckSum(uint8 *data,uint8 len)
{
  uint8 i,sum=0;
  for(i=0;i<(len-1);i++)
  {
    sum+=data[i];
  }
  return sum;
}
/***********************************************/

/*****************************************************************************************
*函数名称 ：定时器3中断函数				                                 
*入口参数 ：无                                
*返 回 值 ：无                                                                               
*说    明 ：产生PWM所需的中断                                                            
*****************************************************************************************/
 
#pragma vector = T3_VECTOR //定时器T3
 __interrupt void T3_ISR(void) 
 { 
   IRCON = 0x00;                  //清中断标志, 也可由硬件自动完成 

        
        if(counter<10)
        { 
            counter++;
        }
        else
        {
            counter = 0;  //计数清零
        }
        if((counter > 0)&&(counter < DutyRatio))
        {
            PWM_OUT1();
        }
        else if (counter>=DutyRatio)
        {
            PWM_OUT0();
        }
        
 }


