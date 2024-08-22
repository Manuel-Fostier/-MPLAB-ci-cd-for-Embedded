/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sdcard.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_sdcard.h"
#include "app_i2c_temp_sensor.h"
#include "peripheral/rtc/plib_rtc.h"
#include "system/console/sys_console.h"
#include "bsp/bsp.h"
#include "definitions.h"                // SYS function prototypes

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define SDCARD_MOUNT_NAME    SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0
#define SDCARD_DEV_NAME      SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0
#define SDCARD_FILE_NAME     "temp_log.txt"

#define BUILD_TIME_HOUR     ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define BUILD_TIME_MIN      ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define BUILD_TIME_SEC      ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define LOG_TIME_LEN        10
#define LOG_TEMP_LEN        21
#define LOG_LEN             (LOG_TIME_LEN + LOG_TEMP_LEN)

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_SDCARD_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_SDCARD_DATA appSDCARDData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void APP_SDCARD_Notify(uint8_t temperature)
{
    /* Temperature Sensor value ready. Here -----> Step #6 */
    appSDCARDData.temperature = temperature;
    appSDCARDData.isTemperatureReady = true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void APP_SysFSEventHandler(SYS_FS_EVENT event,void* eventData,uintptr_t context)
{
    switch(event)
    {
        /* If the event is mount then check which media has been mounted */
        case SYS_FS_EVENT_MOUNT:
            if(strcmp((const char *)eventData, SDCARD_MOUNT_NAME) == 0)
            {
                /* Set SDCARD Mount flag. Here -----> Step #3 */
                appSDCARDData.sdCardMountFlag = true;
            }
            break;

        /* If the event is unmount then check which media has been unmount */
        case SYS_FS_EVENT_UNMOUNT:
            if(strcmp((const char *)eventData, SDCARD_MOUNT_NAME) == 0)
            {
                appSDCARDData.sdCardMountFlag = false;
            }

            if (appSDCARDData.state != APP_SDCARD_STATE_IDLE)
            {
                SYS_CONSOLE_PRINT("!!! WARNING SDCARD Ejected Abruptly !!!\r\n\r\n");

                LED_Clear();

                appSDCARDData.state = APP_SDCARD_STATE_ERROR;
            }
            break;

        case SYS_FS_EVENT_ERROR:
        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_SDCARD_Initialize ( void )

  Remarks:
    See prototype in app_sdcard.h.
 */

void APP_SDCARD_Initialize ( void )
{
    struct tm sys_time;

    /* Intialize the app state to wait for media attach. */
    appSDCARDData.state                     = APP_SDCARD_STATE_MOUNT_WAIT;

    appSDCARDData.isTemperatureReady        = false;
   
    appSDCARDData.sdCardMountFlag           = false;

    sys_time.tm_hour                        = BUILD_TIME_HOUR;
    sys_time.tm_min                         = BUILD_TIME_MIN;
    sys_time.tm_sec                         = BUILD_TIME_SEC;

    /* Set RTC Time to current system time. Here -----> Step #1 */
    RTC_RTCCTimeSet(&sys_time);

    /* Register the File System Event handler. Here -----> Step #2 */
    SYS_FS_EventHandlerSet(APP_SysFSEventHandler,(uintptr_t)NULL);
}

/******************************************************************************
  Function:
    void APP_SDCARD_Tasks ( void )

  Remarks:
    See prototype in app_sdcard.h.
 */
void APP_SDCARD_Tasks ( void )
{
    struct tm sys_time = { 0 };
    char log_data[LOG_LEN] = { 0 };

    switch (appSDCARDData.state)
    {
        case APP_SDCARD_STATE_MOUNT_WAIT:
        {
            /* Wait for SDCARD to be Auto Mounted. Here -----> Step #4 */
            if(appSDCARDData.sdCardMountFlag == true)
            {
                appSDCARDData.state = APP_SDCARD_STATE_OPEN_FILE;
                appSDCARDData.sdCardMountFlag = false;
            }
            break;
        }

        case APP_SDCARD_STATE_OPEN_FILE:
        {
            /* Open Temperature Log file. Here -----> Step #5 */
            appSDCARDData.fileHandle = SYS_FS_FileOpen(SDCARD_MOUNT_NAME"/"SDCARD_FILE_NAME,
                                                       (SYS_FS_FILE_OPEN_WRITE));

            if(appSDCARDData.fileHandle == SYS_FS_HANDLE_INVALID)
            {
                /* Could not open the file. Error out*/
                appSDCARDData.state = APP_SDCARD_STATE_ERROR;
                break;
            }

            appSDCARDData.state = APP_SDCARD_STATE_WRITE;

            break;
        }

        case APP_SDCARD_STATE_WRITE:
        {
            /* Check if temperature data is ready to be written to SDCARD. */
            if (appSDCARDData.isTemperatureReady == true)
            {
                SYS_CONSOLE_PRINT("Logging temperature to SDCARD...");

                appSDCARDData.isTemperatureReady = false;

                /* Get System Time from RTC. Here -----> Step #7 */
                RTC_RTCCTimeGet(&sys_time);
                
                sprintf(&log_data[0], "[%02d:%02d:%02d]", sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec);
                sprintf(&log_data[LOG_TIME_LEN], " Temperature : %d F", appSDCARDData.temperature);

                /* Log System time and temperature value to log file. Here -----> Step #8 */
                if(SYS_FS_FilePrintf(appSDCARDData.fileHandle, "%s\r\n", log_data)
                                    == SYS_FS_RES_FAILURE)
                {
                    /* There was an error while reading the file error out. */
                    appSDCARDData.state = APP_SDCARD_STATE_ERROR;
                }
                else
                {
                    /* The test was successful. Lets idle. */
                    SYS_CONSOLE_PRINT("Done!!!\r\n\r\n");

                    LED_Toggle();

                    appSDCARDData.state = APP_SDCARD_STATE_SWITCH_CHECK;
                }
            }
            break;
        }

        case APP_SDCARD_STATE_SWITCH_CHECK:
        {
            /* Check if Switch is pressed to stop logging of data. Here -----> Step #9 */
            if ( SWITCH_Get() == SWITCH_STATE_PRESSED)
            {
                appSDCARDData.state = APP_SDCARD_STATE_CLOSE_FILE;
            }
            else
            {
                appSDCARDData.state = APP_SDCARD_STATE_WRITE;
            }
                 
            break;
        }

        case APP_SDCARD_STATE_CLOSE_FILE:
        {
            SYS_FS_FileClose(appSDCARDData.fileHandle);

            SYS_CONSOLE_PRINT("Logging temperature to SDCARD Stopped \r\n");
            SYS_CONSOLE_PRINT("Safe to Eject SDCARD \r\n\r\n");

            LED_Clear();

            appSDCARDData.state = APP_SDCARD_STATE_IDLE;

            break;
        }

        case APP_SDCARD_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT("SDCARD Task Error \r\n\r\n");
            appSDCARDData.state = APP_SDCARD_STATE_IDLE;
            break;
        }

        case APP_SDCARD_STATE_IDLE:
        default:
        {
            break;
        }
    }
}
