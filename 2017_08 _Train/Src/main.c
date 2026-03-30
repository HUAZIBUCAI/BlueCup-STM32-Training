#include "main.h"
#include "rcc\bsp_rcc.h"
#include "rtc\bsp_rtc.h"
#include "key_led\bsp_key_led.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"

__IO uint32_t uwTick_KEY_SET_Point=0;
__IO uint32_t uwTick_LED_SET_Point=0;
__IO uint32_t uwTick_LCD_SET_Point=0;

uint8_t Key_Value=0;
uint8_t Key_Old=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;

uint8_t ucLed=0x00;

uint8_t Lcd_Disp_str[20];

RTC_TimeTypeDef	H_M_S;
RTC_DateTypeDef Y_M_D;
static void Key_Proc(void);
static void Led_Proc(void);
static void Lcd_Proc(void);

int main(void)
{
	
	HAL_Init();

	SystemClock_Config();

	Clock_RTC_Init();
	Key_Lcd_GPIO_Init();
	LCD_Init();
	LCD_Clear(White);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(White);
	
	PA6_TIM3_Init();
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	PA7_TIM17_Init();
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);	
  while (1)
  {
	  Key_Proc();
	  Led_Proc();
	  Lcd_Proc();
	 
  }
 
}


static void Key_Proc(void)
{
	if(uwTick-uwTick_KEY_SET_Point<=50)return;
	uwTick_KEY_SET_Point=uwTick;
	
	Key_Value=Key_Scan();
	Key_Down=Key_Value&(Key_Value^Key_Up);
	Key_Up=~Key_Value&(Key_Value^Key_Up);
	Key_Old=Key_Value;

	if(Key_Value==1)ucLed=0xf0;
	if(Key_Value==2)ucLed=0x0f;
}
static void Led_Proc(void)
{
	if(uwTick-uwTick_LED_SET_Point<=200)return;
	uwTick_LED_SET_Point=uwTick;
	Led_Disp(ucLed);

}
static void Lcd_Proc(void)
{
	if(uwTick-uwTick_LCD_SET_Point<=200)return;
	uwTick_LCD_SET_Point=uwTick;
	sprintf((char*)Lcd_Disp_str,"Hello Blue Cup");
	LCD_DisplayStringLine(Line1,Lcd_Disp_str);
	
	
	HAL_RTC_GetDate(&hrtc,&Y_M_D,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc,&H_M_S,RTC_FORMAT_BIN);
	sprintf((char*)Lcd_Disp_str,"%02d-%02d-%02d",H_M_S.Hours,H_M_S.Minutes,H_M_S.Seconds);
	LCD_DisplayStringLine(Line3,Lcd_Disp_str);
	
	
}

/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
void Error_Handler(void)
{
  
  __disable_irq();
  while (1)
  {
  }
  
}
