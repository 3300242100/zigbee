
/*********************************START*****************************/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "GenericApp.h"
#include "DebugTrace.h"

#if !defined(WIN32)
#include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "ringbuffer.h"
#include "protocol.h"

#include "oled.h"

//---------------------Ӳ������--------------------------//
#define SENSOR_PORT_Beep_Active (0)
#define SENSOR_PORT_Beep (P1_2)

#define InitSensor_Beep() \
  do                      \
  {                       \
    P1SEL &= ~(1 << 2);   \
    P1DIR |= (1 << 2);    \
  } while (0)

void SetSensor_Beep(unsigned char value);

void SetSensor_Beep(unsigned char value)
{
  if (value)
  {
    SENSOR_PORT_Beep = !!SENSOR_PORT_Beep_Active;
  }
  else
  {
    SENSOR_PORT_Beep = !SENSOR_PORT_Beep_Active;
  }
}

#define SENSOR_PORT_Jidianqi (P1_7)

#define InitSensor_Jidianqi() \
  do                          \
  {                           \
    P1SEL &= ~(1 << 7);       \
    P1DIR |= (1 << 7);        \
    SENSOR_PORT_Jidianqi = 0; \
  } while (0)

void SetSensor_Jidianqi(unsigned char value);

/**************zstack�����ʼ������*******************/

// ����CLUSTERS id
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
    {
        GENERICAPP_CLUSTERS_ID};

// �ϵ�������
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
    {
        GENERICAPP_ENDPOINT,             //  int Endpoint;
        GENERICAPP_PROFID,               //  uint16 AppProfId[2];
        GENERICAPP_DEVICEID,             //  uint16 AppDeviceId[2];
        GENERICAPP_DEVICE_VERSION,       //  int   AppDevVer:4;
        GENERICAPP_FLAGS,                //  int   AppFlags:4;
        GENERICAPP_MAX_CLUSTERS,         //  byte  AppNumInClusters;
        (cId_t *)GenericApp_ClusterList, //  byte *pAppInClusterList;
        GENERICAPP_MAX_CLUSTERS,         //  byte  AppNumInClusters;
        (cId_t *)GenericApp_ClusterList  //  byte *pAppInClusterList;
};

// ��ʼ������ϵ�������
endPointDesc_t GenericApp_epDesc;

/*********************************************************************
 * ���������ʼ������
 */
byte GenericApp_TaskID; // ����ID

devStates_t GenericApp_NwkState; // ����״̬

byte GenericApp_TransID; // ���͵����ݰ�id

// ���崮�ڵĽ������ݻ�����
#define Uart_Recv_property_max_len 256
static byte RxBuf[Uart_Recv_property_max_len + 1]; // ���յ�����
static uint8 SerialApp_TxLen;                      // ���ڶ�ȡ�����鳤��

/********************�ֲ����ĺ���*****************************/

// ����������
void GenericApp_HandleKeys(byte shift, byte keys);

// ���յ�zigbee��Ϣ������
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);

// ���ڴ�����
void Uart_Recv_Callback(uint8 port, uint8 event);

// �������ݵ��ն˽ڵ�ĺ���
void sendDataToEnd(uint16 shortAddr, uint16 clusterId, uint8 *data, uint8 len);

// �����Լ�⴫�����ĺ���
void detect_sensor(void);
/********************???????:END******************************/

unsigned int Global_var_show_wendu1 = 0;
unsigned int Global_var_show_shidu1 = 0;
unsigned int Global_var_show_sudu1 = 0;
/********************************************************************
 *����Э��������� �� ����
 */

#define TRANSMISSION_diliver_pack_LEN 64
#define TRANSMISSION_CACHE_LEN 256
static unsigned char data_buffer_send[TRANSMISSION_diliver_pack_LEN + 1];
static unsigned char ringbuff_data_send[TRANSMISSION_CACHE_LEN];
static RingBuffer rb_sensor_send;

unsigned char protocol_recv_msg_buff[300];
struct protocol_pipe pipe_recv_msg;

// �������ݵ�������
void transfer_send_to_cache(unsigned char *buff, unsigned int len)
{
  int rb_len;

  if (len == 0)
    return;

  rb_len = rb_can_write(&rb_sensor_send);

  len = rb_len > len ? len : rb_len;

  rb_write(&rb_sensor_send, buff, len);
}

// �����������ݷ��ͳ�ȥ
void transfer_send_sync(void)
{
  int rb_len;

  rb_len = rb_can_read(&rb_sensor_send);

  rb_len = TRANSMISSION_diliver_pack_LEN > rb_len ? rb_len : TRANSMISSION_diliver_pack_LEN;

  rb_len = rb_read(&rb_sensor_send, data_buffer_send, rb_len);

  if (rb_len > 0)
  {
    sendDataToEnd(0, GENERICAPP_CLUSTERS_ID, data_buffer_send, rb_len);
  }
}

// �����ݷ��͵��ն˽ڵ�
void sendDataToEnd(uint16 shortAddr, uint16 clusterId, uint8 *data, uint8 len)
{
  afAddrType_t Tx_DstAddr;

  if (shortAddr == 0)
  {

    Tx_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
    Tx_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    Tx_DstAddr.addr.shortAddr = 0xffff; /*�㲥���� */
  }
  else
  {
    /* ?? */
    Tx_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    Tx_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    Tx_DstAddr.addr.shortAddr = shortAddr; /*�̵�ַ */
  }

  if (AF_DataRequest(&Tx_DstAddr, &GenericApp_epDesc,
                     clusterId,
                     len,
                     data,
                     &GenericApp_TransID,
                     AF_DISCV_ROUTE,
                     AF_DEFAULT_RADIUS) == afStatus_SUCCESS)
  {
  }
  else
  {
    /* Error occurred in request to send. */
  }
}

// �ַ���ת��ֵ
int osal_atoi(const char *str)
{
  int value = 0;
  while (*str >= '0' && *str <= '9')
  {
    value *= 10;
    value += *str - '0';
    str++;
  }
  return value;
}

// �����ն˷������Ĵ���������
void protocol_recv_id_value(const char *id, const char *value)
{

  if (protocol_pipe_compare(id, "wendu1", sizeof("wendu1") - 1))
  {
    Global_var_show_wendu1 = osal_atoi(value);
  }
  else if (protocol_pipe_compare(id, "zhendong1", sizeof("zhendong1") - 1))
  {
    Global_var_show_shidu1 = osal_atoi(value);
  }
  else if (protocol_pipe_compare(id, "sudu1", sizeof("sudu1") - 1))
  {
    Global_var_show_sudu1 = osal_atoi(value);
  }
}

// ��ʼ������
void GenericApp_Init(byte task_id)
{

  /*************��ʼ������****************/
  halUARTCfg_t uartConfig;

  uartConfig.configured = TRUE; // 2x30 don't care - see uart driver.
  uartConfig.baudRate = Uart_Recv_Send_baudRate;
  uartConfig.flowControl = FALSE;
  uartConfig.flowControlThreshold = 64; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize = 256;       // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize = 256;       // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout = 6;
  uartConfig.intEnable = TRUE; // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc = Uart_Recv_Callback;
  HalUARTOpen(0, &uartConfig);

  /************�����ʼ��**************/

  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;

  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister(&GenericApp_epDesc);

  /***********������ʼ��**************/
  // Register for all key events - This app will handle all key events
  RegisterForKeys(GenericApp_TaskID);

  /***********������ �� ����Э���ʼ��**************/
  rb_new(&rb_sensor_send, ringbuff_data_send, TRANSMISSION_CACHE_LEN);

  protocol_pipe_init(&pipe_recv_msg, protocol_recv_msg_buff, sizeof(protocol_recv_msg_buff));

  OLED_Init();

  InitSensor_Beep();

  InitSensor_Jidianqi();
}

// ���������¼�
UINT16 GenericApp_ProcessEvent(byte task_id, UINT16 events)
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID; // This should match the value sent
  (void)task_id;    // Intentionally unreferenced parameter

  if (events & SYS_EVENT_MSG)
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
    while (MSGpkt)
    {
      switch (MSGpkt->hdr.event)
      {
      /***************�ж��¼�����**********/
      case KEY_CHANGE:
        GenericApp_HandleKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
        break;

      /********???????????????***********/
      case AF_DATA_CONFIRM_CMD:
        afDataConfirm = (afDataConfirm_t *)MSGpkt;
        sentEP = afDataConfirm->endpoint;
        sentStatus = afDataConfirm->hdr.status;
        sentTransID = afDataConfirm->transID;
        (void)sentEP;
        (void)sentTransID;
        // Action taken when confirmation is received.
        if (sentStatus != ZSuccess)
        {
          // ���յ�����
        }
        break;
      /********���ݰ�ȷ��***********/
      case AF_INCOMING_MSG_CMD:
        GenericApp_MessageMSGCB(MSGpkt);
        break;

      /**********����״̬�����ı�*************/
      case ZDO_STATE_CHANGE:
        GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
        if ((GenericApp_NwkState == DEV_ZB_COORD)
            // || (GenericApp_NwkState == DEV_ROUTER)
            // || (GenericApp_NwkState == DEV_END_DEVICE)
        )
        {

          // �����ʼ���ɹ�������״̬�����ı䣬��״̬��ΪЭ����ʱ��˵��Э���������ɹ���
          //  Start sending "the" message in a regular interval.

          // HalLcdWriteString( "NETWORK-OK", HAL_LCD_LINE_3 );

          // HalUARTWrite(0, "coordinator network ok\n", strlen("coordinator network ok\n"));

          /* ע�ᶨʱ������ */
          osal_start_timerEx(GenericApp_TaskID,
                             GENERICAPP_DETECT_SENSOR_MSG_EVT,
                             GENERICAPP_DETECT_SENSOR_MSG_TIMEOUT);

          osal_start_timerEx(GenericApp_TaskID,
                             GENERICAPP_sync_MSG_EVT,
                             GENERICAPP_sync_MSG_TIMEOUT);

          osal_start_timerEx(GenericApp_TaskID,
                             GENERICAPP_1s_EVT,
                             GENERICAPP_1s_TIMEOUT);
        }
        break;

      default:
        break;
      }

      // Release the memory
      osal_msg_deallocate((uint8 *)MSGpkt);

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if (events & GENERICAPP_sync_MSG_EVT)
  {

    /*************?�����ԵĴӻ�������ͬ������ **************/

    transfer_send_sync();

    protocol_pipe_get(&pipe_recv_msg, protocol_recv_id_value);

    osal_start_timerEx(GenericApp_TaskID,
                       GENERICAPP_sync_MSG_EVT,
                       GENERICAPP_sync_MSG_TIMEOUT);

    return (events ^ GENERICAPP_sync_MSG_EVT);
  }

  if (events & GENERICAPP_DETECT_SENSOR_MSG_EVT)
  {

    /*************  **************/

    static char first_init = 1;
    if (first_init)
    {
      // wireless_config(); // ��ʼ��WIFI����
      first_init = 0;
    }

    detect_sensor();

    /*************?ѭ��������ʱ��**************/

    osal_start_timerEx(GenericApp_TaskID,
                       GENERICAPP_DETECT_SENSOR_MSG_EVT,
                       GENERICAPP_DETECT_SENSOR_MSG_TIMEOUT);

    return (events ^ GENERICAPP_DETECT_SENSOR_MSG_EVT);
  }

  if (events & GENERICAPP_1s_EVT)
  {

    /***************************/

    static unsigned int tick_count_1s = 1;

    if (tick_count_1s % 30 == 5)
    {
      // wireless_config(); // ��ʼ����������
    }

    osal_start_timerEx(GenericApp_TaskID,
                       GENERICAPP_1s_EVT,
                       GENERICAPP_1s_TIMEOUT);

    tick_count_1s++;

    return (events ^ GENERICAPP_1s_EVT);
  }

  return 0;
}

// �����Բɼ�����ʾ����
void detect_sensor(void)
{
  char buff[30];

  osal_memset(buff, 0, sizeof(buff));
  sprintf(buff, ":%d  ", Global_var_show_wendu1);
  OLED_P8x16Str(32, 0, buff);

  osal_memset(buff, 0, sizeof(buff));
  sprintf(buff, ":%d  ", Global_var_show_sudu1);
  OLED_P8x16Str(32, 2, buff);

  osal_memset(buff, 0, sizeof(buff));
  sprintf(buff, ":%d  ", Global_var_show_shidu1);
  OLED_P8x16Str(32, 4, buff);

  // �¶�ʪ�ȹ�ǿ
  OLED_ShowCHinese(0, 0, 0);
  OLED_ShowCHinese(16, 0, 1);
  OLED_ShowCHinese(0, 2, 3);
  OLED_ShowCHinese(16, 2, 1);
  OLED_ShowCHinese(0, 4, 4);
  OLED_ShowCHinese(16, 4, 5);
}

// ��������ʱ��
void GenericApp_HandleKeys(byte shift, byte keys)
{
  (void)shift; /* Intentionally unreferenced parameter */

  if (keys & HAL_USER_KEY_1)
  {
    // ����1����

    HalLedBlink(HAL_LED_1, 6, 50, 3000);

    // HalUARTWrite(0,"{\"smartlink\":\"start\"}",strlen("{\"smartlink\":\"start\"}"));
  }

  /**************************************************************/
  if (keys & HAL_USER_KEY_2)
  {
    HalLedBlink(HAL_LED_2, 6, 50, 3000);

    // HalUARTWrite(0,"{\"cfg_server_host\":\"xpkesc89aa.51http.tech/student/2034/update.php\"}",strlen("{\"cfg_server_host\":\"xpkesc89aa.51http.tech/student/2034/update.php\"}"));
  }
}

void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pkt)
{

  unsigned char *send = pkt->cmd.Data;
  unsigned int len_left = pkt->cmd.DataLength;

  protocol_pipe_put(&pipe_recv_msg, send, len_left);

  while (1)
  {
    if (len_left > 256)
    {
      HalUARTWrite(0, send, 256); /* ���ݰ��޷�����Ϊ���ڷ�����󻺴�Ϊ256�ֽ� */
      send += 256;
      len_left = len_left - 256;
    }
    else
    {
      HalUARTWrite(0, send, len_left); /* ���ݰ��޷�����Ϊ���ڷ�����󻺴�Ϊ256�ֽ� */
      break;
    }
  }
}

// ���յ���������
static void Uart_Recv_Callback(uint8 port, uint8 event)
{

  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)))
  {
    // osal_memset(RxBuf, 0, Uart_Recv_property_max_len + 1);
    SerialApp_TxLen = 0;

    // ��ȡ���ڽ��յ�������
    SerialApp_TxLen = HalUARTRead(0, RxBuf, Uart_Recv_property_max_len);

    // �����ݷŵ���������������ֱ��ת����
    transfer_send_to_cache(RxBuf, SerialApp_TxLen);
  }
}

/*********************************************************************
*********************************************************************/
