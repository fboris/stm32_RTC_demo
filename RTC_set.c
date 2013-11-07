#include "RTC_set.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_conf.h"
extern xTaskHandle *pvLEDTask;
void RTC_Alarm_IRQHandler()
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    vTaskSuspend( pvLEDTask );

    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }
}

static void initialize_RTC(void)
{
  RTC_InitTypeDef RTC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);   /* Enable the PWR clock */
  PWR_BackupAccessCmd(ENABLE);                          /* Allow access to RTC */

  RCC_LSICmd(ENABLE);                                   /* Enable the LSI OSC */    
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);   /* Wait till LSI is ready */  
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);               /* Select the RTC Clock Source */

  RCC_RTCCLKCmd(ENABLE);                                /* Enable the RTC Clock */
  RTC_WaitForSynchro();                                 /* Wait for RTC APB registers synchronisation */

  /* Configure the RTC data register and RTC prescaler */
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv = 0xF9;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);
}

static void setting_time(void)
{
  /* set 8:29:55 */
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_TimeStruct.RTC_Hours = 8;
  RTC_TimeStruct.RTC_Minutes = 29;
  RTC_TimeStruct.RTC_Seconds = 55;

  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

}
static void initialize_RTC_alarm(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* EXTI configuration */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable the RTC Alarm Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
static void set_alarm_time(void)
{

  RTC_AlarmTypeDef RTC_AlarmStructure;

  RTC_AlarmCmd(RTC_Alarm_A, DISABLE);   /* disable before setting or cann't write */

  /* set alarm time 8:30:0 everyday */
  RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = 0x00;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 8;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 30;
  RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0;
  RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31; // Nonspecific
  RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
  RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // Everyday 
  RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

  /* Enable Alarm */
  RTC_ITConfig(RTC_IT_ALRA, ENABLE);
  RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
  RTC_ClearFlag(RTC_FLAG_ALRAF);
}

void RTC_setting()
{
    initialize_RTC();
    setting_time();
    initialize_RTC_alarm();
    set_alarm_time();

}
