#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"

__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;

uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;
static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
int main(void)
{

	HAL_Init();
	
    SystemClock_Config();
    Led_Key_GPIO_Init();
 
    LCD_Init();
    LCD_Clear(White);
    LCD_SetTextColor(Black);
    LCD_SetBackColor(White);
while (1)
  {
	  
	Led_Proc();
    Key_Proc();
    Lcd_Proc();
  }
  
}



static void Led_Proc(void)
{
	if(uwTick-uwTick_Led_Set_Point<=200)return;
	uwTick_Led_Set_Point=uwTick;
	Led_Disp(ucLed);
}
static void Key_Proc(void)
{
	if(uwTick-uwTick_Key_Set_Point<=50)return;
	uwTick_Key_Set_Point=uwTick;
	Key_Value=Key_Scan();
	Key_Down=Key_Value&(Key_Value^Key_Old);
	Key_Up=~Key_Value&(Key_Value^Key_Old);
	Key_Old=Key_Value;
	
	if(Key_Down==1)ucLed=0xff;
	if(Key_Down==2)ucLed=0xf0;
	if(Key_Down==3)ucLed=0x0f;
	if(Key_Down==4)ucLed=0x00;

}static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;
	sprintf((char *)Lcd_Disp_String,"Hello Blue Cup");
	LCD_DisplayStringLine(Line1,Lcd_Disp_String);
	

}
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
void Error_Handler(void)
{

  __disable_irq();
  while (1)
  {
  }
 
}
