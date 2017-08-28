

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
#include "Smoke.h"

/******************************************************************************************
 *                                       MACROS
 ******************************************************************************************/

/******************************************************************************************
 *                                     CONSTANTS
 ******************************************************************************************/

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
 *                                      TYPEDEFS
 ******************************************************************************************/

/*******************************************************************************************
 *                                  GLOBAL VARIABLES
 ******************************************************************************************/

uint8 SerialApp_TaskID;    // Task ID for internal task/event processing.
devStates_t  SampleApp_NwkState;
static UART_Format UART0_Format;
static UART_Format_End1 UART0_Format1;
/******************************************************************************************
 *                                 EXTERNAL VARIABLES
 ******************************************************************************************/

/*******************************************************************************************
 *                                 EXTERNAL FUNCTIONS
 ******************************************************************************************/

/*******************************************************************************************
 *                                  LOCAL VARIABLES
 ******************************************************************************************/

static uint8 SerialApp_MsgID;
static afAddrType_t SerialApp_TxAddr;
static uint8 SerialApp_TxBuf[SERIAL_APP_TX_MAX];
static uint8 SerialApp_TxLen;
static uint8 HallStatusOld;
static uint8 HallStatusNow;
/*******************************************************************************************
 *                                  LOCAL FUNCTIONS
 ******************************************************************************************/

static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
void SerialApp_OTAData(afAddrType_t *txaddr,uint8 ID,void *p,uint8 len);
static void SerialApp_CallBack(uint8 port, uint8 event);
static uint8 CheckSum(uint8 *data,uint8 len);
static void PhotoInit(void);
extern void FLASHLED(uint8 led);
extern void LED(uint8 led,uint8 operation);
extern void ledInit(void);

/*****************************************************************************************
*函数名称 ：烟雾传感器端口初始化函数				                                 
*入口参数 ：无                                
*返 回 值 ：无                                                                                
*说    明 ：把P01设置为通用IO,输入模式                                                            
*****************************************************************************************/

static void PhotoInit(void) //P00  AD   P01   开关
{
  P0SEL &= ~0x01;  //P01设置为通用IO
  P0DIR &= ~0x01;  //将P01设置为输入模式 
  if(HALL_GPIO == HIGH)
    HallStatusNow = HALL_NO;
  else
    HallStatusNow = HALL_YES;
}

/*****************************************************************************************
*函数名称 ：串口初始化函数					                          
*入口参数 ：task_id - OSAL分配的任务ID号		                          
*返 回 值 ：无							                          
*说    明 ：在OSAL初始化的时候被调用                             
*****************************************************************************************/

void SerialApp_Init( uint8 task_id )
{
  halUARTCfg_t uartConfig;

  SerialApp_TaskID = task_id;

  afRegister( (endPointDesc_t *)&SerialApp_epDesc );

  RegisterForKeys( task_id );
  PhotoInit();
  ledInit();
  
  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = SERIAL_APP_BAUD;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = SerialApp_CallBack;
  HalUARTOpen (SERIAL_APP_PORT, &uartConfig);
  
  UART0_Format.Header   = '@';
  UART0_Format.Len      = 0x10;
  UART0_Format.NodeSeq  = 0x01;
  UART0_Format.NodeID   = Smoke;
  
  UART0_Format1.Header   = '@';
  UART0_Format1.Len      = 0x07;
  UART0_Format1.NodeSeq  = 0x01;
  UART0_Format1.NodeID   = Smoke;
  
  
  
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
        //FLASHLED(3);
        break;

      case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if(SampleApp_NwkState == DEV_END_DEVICE) //判定当前设备类型
          {
            
            osal_set_event(SerialApp_TaskID, PERIOD_EVT); //启动周期消息
            osal_set_event(SerialApp_TaskID, HALL_READ_EVT); //启动检测
            LED(1,ON);
          }
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
    UART0_Format.Data[0] = 0x00;
    
    
    num = CheckSum(&UART0_Format.Header,UART0_Format.Len);
    UART0_Format.Verify  = num; 
      
    SerialApp_OTAData(&SerialApp_TxAddr, SERIALAPP_CLUSTERID1, &UART0_Format, sizeof(UART_Format));
    osal_start_timerEx(SerialApp_TaskID, PERIOD_EVT, 5000);
    FLASHLED(2);
    return ( events ^ PERIOD_EVT );
  }
  
  if ( events & SERIALAPP_SEND_EVT ) //发送RF消息
  { 
    num = CheckSum(&UART0_Format1.Header,UART0_Format1.Len);
    UART0_Format1.Verify  = num; 
    
    SerialApp_OTAData(&SerialApp_TxAddr,SERIALAPP_CLUSTERID1, &UART0_Format1, 7);
    FLASHLED(4);
    return ( events ^ SERIALAPP_SEND_EVT );
  }

  if ( events & HALL_READ_EVT )  //查看状态
  {
    HallStatusOld = HallStatusNow;  //保存当前状态
    if(HALL_GPIO == HIGH)
    {
      HallStatusNow = HALL_NO;
    }
    else if(HALL_GPIO == LOW)
    {
      HallStatusNow = HALL_YES;
    }
    
    if(HallStatusNow != HallStatusOld) //如果当前状态和上一个状态发生变化，发送消息给网关
    {
      UART0_Format1.Command = 0x01;
      if(HallStatusNow == HALL_YES)
      {
        UART0_Format1.Data[0] = 0x00;
        
      }
      else if(HallStatusNow == HALL_NO)
      {
        UART0_Format1.Data[0] = 0x01;
          
      }
      osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
    }
    
    osal_start_timerEx(SerialApp_TaskID, HALL_READ_EVT, 200);
    
    return ( events ^ HALL_READ_EVT );
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
  uint8 num=0;
  static UART_Format *receiveData;
  switch ( pkt->clusterId )
  {
   case SERIALAPP_CLUSTERID1:  //处理各个传感器节数据    
     receiveData = (UART_Format *)(pkt->cmd.Data);
     //HalLedBlink(HAL_LED_1,1,50,200);
     
     num = CheckSum((uint8*)receiveData,receiveData->Len);
     
     if((receiveData->Header==0x40)&&(receiveData->Verify==num)) //校验包头包尾
     {
       
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
    //HalLedBlink(HAL_LED_1,1,50,200);
  }
}

/*****************************************************************************************
*函数名称 ：串口回调函数				                                 
*入口参数 ：port  -端口号
*           event -事件号
*返 回 值 ：无                                                                              
*说    明 ：把串口消息通过无线发送出去                                                             
*****************************************************************************************/

static void SerialApp_CallBack(uint8 port, uint8 event)
{
  (void)port;

  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) && !SerialApp_TxLen) //串口接收到数据包
  {
    SerialApp_TxLen = HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf, SERIAL_APP_TX_MAX); //将串口数据读入buf
    SerialApp_TxLen = 0;  
  }
}

/*****************************************************************************************
*函数名称 ：累加校验和函数				                                 
*入口参数 ：*data - 数据包地址
            len   - 数据包长度
*返 回 值 ：sum   - 累加和                                                                                
*说    明 ：对数据包进行累加和校验                                                             
*****************************************************************************************/

uint8 CheckSum(uint8 *data,uint8 len)
{
  uint8 i,sum=0;
  for(i=0;i<(len-1);i++)
  {
    sum+=data[i];
  }
  return sum;
}
/***********************************************/