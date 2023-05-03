
#ifndef GENERICAPP_H
#define GENERICAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * CONSTANTS
 */


#define GENERICAPP_ENDPOINT           10

#define GENERICAPP_PROFID             0x0F04
#define GENERICAPP_DEVICEID           0x0001
#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0


#define GENERICAPP_MAX_CLUSTERS   1
#define GENERICAPP_CLUSTERS_ID 1


#define HAL_USER_KEY_2 HAL_KEY_SW_1
#define HAL_USER_KEY_1 HAL_KEY_SW_6



  

#define GENERICAPP_DETECT_SENSOR_MSG_TIMEOUT 2000  
#define GENERICAPP_DETECT_SENSOR_MSG_EVT        0x0002


#define GENERICAPP_sync_MSG_TIMEOUT 100  
#define GENERICAPP_sync_MSG_EVT        0x0004


#define GENERICAPP_1s_TIMEOUT 1000  
#define GENERICAPP_1s_EVT        0x0008


#define Uart_Recv_Send_baudRate  HAL_UART_BR_38400






/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Generic Application
 */
extern void GenericApp_Init( byte task_id );

/*
 * Task Event Processor for the Generic Application
 */
extern UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GENERICAPP_H */
