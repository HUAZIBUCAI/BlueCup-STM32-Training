#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"
#include "adc\bsp_adc.h"


__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;



uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;



uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;

uint8_t IntereFace=0x00;
_Bool Mode_Flag=0;//0为自动模式 1为手动模式
float Voltage;
uint8_t PA6_Compare=10;
uint8_t PA7_Compare=10;
uint8_t Auto_Duty=10;
_Bool Freq_Flag=0;//0 PA6 100Hz PA7 200Hz 	1 PA6 200Hz PA7 100Hz
static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
int main(void)
{

	HAL_Init();
	
    SystemClock_Config();
    Led_Key_GPIO_Init();
 
    LCD_Init();
    LCD_Clear(Black);
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);

	R37_ADC2_Init();

	PA6_TIM3_Init();
	PA7_TIM17_Init();
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);

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
	if(Mode_Flag)
	{
		if(IntereFace==0x00)
		{
			ucLed=0x02;
		}
		else
		{
			ucLed=0x00;
		}

	}
	else
	{
		if(IntereFace==0x00)
		{
			ucLed=0x03;
		}
		else
		{
			ucLed=0x01;	
		}
	}

}

static void Key_Proc(void)
{
	if(uwTick-uwTick_Key_Set_Point<=50)return;
	uwTick_Key_Set_Point=uwTick;
	Key_Value=Key_Scan();
	Key_Down=Key_Value&(Key_Value^Key_Old);
	Key_Up=~Key_Value&(Key_Value^Key_Old);
	Key_Old=Key_Value;
	

	if(Key_Down==1)
	{
		LCD_Clear(Black);
		if((IntereFace>>4)==0x01)
		{
			IntereFace=0x00;
		}
		else
		{
			IntereFace=0x10;
		}

	}

	if(Key_Down==2)
	{
		LCD_Clear(Black);
		if(IntereFace==0x10)
		{
			PA6_Compare+=10;
			if(PA6_Compare>90)
			{
				PA6_Compare=10;
			}
		}


	}
	if(Key_Down==3)
	{
		LCD_Clear(Black);
		if(IntereFace==0x10)
		{
			PA7_Compare+=10;
			if(PA7_Compare>90)
			{
				PA7_Compare=10;
			}
		}

		if(IntereFace==0x00)
		{
			Freq_Flag^=1;
			if(Freq_Flag)
			{
				__HAL_TIM_SET_AUTORELOAD(&htim3,4999);
				__HAL_TIM_SET_AUTORELOAD(&htim17,9999);
			}
			else
			{
				__HAL_TIM_SET_AUTORELOAD(&htim3,9999);
				__HAL_TIM_SET_AUTORELOAD(&htim17,4999);
			}
			

		}


	}

	if(Key_Down==4)
	{
		LCD_Clear(Black);
		if(IntereFace==0x00)
		{
			 Mode_Flag^=1;
		}
	}
	
}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;
	Voltage=Get_ADC_R37()*3.3/4096;

	Auto_Duty=(uint8_t)((double)Voltage*100/3.3);
	if(Mode_Flag)
	{
		if(Freq_Flag)
		{	
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,PA6_Compare*50);
			__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,PA7_Compare*100);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,PA6_Compare*100);
			__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,PA7_Compare*50);
		}

	
	}
	else
	{
		if(Freq_Flag)
		{
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,Auto_Duty*50);
			__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,Auto_Duty*100);
		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,Auto_Duty*100);
			__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,Auto_Duty*50);
		}
		
	}	
	if(IntereFace==0x00)
	{
		if(Mode_Flag)
		{
			sprintf((char *)Lcd_Disp_String,"      Data");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String,"    V:%1.2f",Voltage);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	
			sprintf((char *)Lcd_Disp_String,"      Mode:MANU");
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);

		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"      Data");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String,"    V:%1.2f",Voltage);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	
			sprintf((char *)Lcd_Disp_String,"      Mode:AUTO");
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);
		}



	}
	else
	{
		
		if(Mode_Flag)//参数手动模式显示
		{
			sprintf((char *)Lcd_Disp_String,"      Para");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String,"    PA6:%2d",PA6_Compare);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	
			sprintf((char *)Lcd_Disp_String,"    PA7:%2d",PA7_Compare);
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);

		}
		else//参数自动模式显示
		{
			sprintf((char *)Lcd_Disp_String,"      Data");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String,"    PA6:%2d",Auto_Duty);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	
			sprintf((char *)Lcd_Disp_String,"    PA7:%2d",Auto_Duty);
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);
		}



	}


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
