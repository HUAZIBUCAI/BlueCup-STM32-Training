#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
# include "adc\bsp_adc.h"

__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_TH_Set_Point=0;
__IO uint32_t uwTick_TL_Set_Point=0;
__IO uint32_t uwTick_Time_Old_Set_Point=0;
__IO uint32_t uwTick_Time_New_Set_Point=0;

uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;

uint8_t InterFace=0x00;
float R37_Vlotage;
float R37_V1;
float R37_V2;
uint8_t i=0;
uint32_t Seconds=0;
uint8_t Vmax=30;
uint8_t Vmin=10;
uint8_t Vmax_Disp=30;
uint8_t Vmin_Disp=10;

_Bool ADC_Flag=0;
_Bool Time_Flag=0;
_Bool Para_Flag=0;


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

	R37_ADC2_Init();
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
	
	// if(Key_Down==1)ucLed=0xff;
	// if(Key_Down==2)ucLed=0xf0;
	// if(Key_Down==3)ucLed=0x0f;
	// if(Key_Down==4)ucLed=0x00;

	if(Key_Down==1)
	{
		 LCD_Clear(White);
		if((InterFace>>4)==0x01)
		{
			InterFace=0x00;
		}
		else
		{
			InterFace=0x10;
		}

		if(Vmax_Disp>=(Vmin_Disp+10))
		{
			Vmin=Vmin_Disp;
			Vmax=Vmax_Disp;		
		}
		else
		{
			Vmin_Disp=Vmin;
			Vmax_Disp=Vmax;
		}
		Para_Flag=0;
	}

	if(Key_Down==2)
	{
		LCD_Clear(White);
		Vmax_Disp+=1;
		if(Vmax_Disp>33)
		Vmax_Disp=0;
		Para_Flag=1;
	}

	if(Key_Down==3)
	{
		LCD_Clear(White);
		Vmin_Disp+=1;
		if(Vmin_Disp>33)
		Vmin_Disp=0;
		Para_Flag=1;
	}


}


static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=100)return;
	uwTick_Lcd_Set_Point=uwTick;
	R37_Vlotage=Get_R37_ADC()*3.3/4096;

	Time_Flag^=1;
	if(Time_Flag==0)
	{
		R37_V1=R37_Vlotage;
		uwTick_Time_Old_Set_Point=uwTick;
	}
	
	else if(Time_Flag==1)
	{
		R37_V2=R37_Vlotage;
		uwTick_Time_New_Set_Point=uwTick;
	}


	if(R37_V1<(Vmin*0.1)&&R37_V2>(Vmin*0.1)&&(uwTick_Time_Old_Set_Point<uwTick_Time_New_Set_Point))//ADC转换标识位 电压从低到高才开始计时
	{
		uwTick_TL_Set_Point=uwTick;
		ADC_Flag=1;
	}

	if((R37_V1<(Vmax*0.1))&&(R37_V2>(Vmax*0.1))&&(ADC_Flag)&&(uwTick_Time_Old_Set_Point<uwTick_Time_New_Set_Point))//ADC转换标识位 电压从低到高才停止计时
	{
		uwTick_TH_Set_Point=uwTick;
		ADC_Flag=0;
	}

	if(ADC_Flag)
	{
		Seconds=uwTick-uwTick_TL_Set_Point;
	}
	else
	{
		Seconds=uwTick_TH_Set_Point-uwTick_TL_Set_Point;
	}


	if(InterFace==0x00)
	{
		sprintf((char *)Lcd_Disp_String,"      Data");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String," V:%1.2fV",R37_Vlotage);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String," T:%.1fs",(float)Seconds/1000);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);

	}
	else
	{
		switch(Para_Flag=1)
		{
			case 0:
			sprintf((char *)Lcd_Disp_String,"      Para");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String," Vmax:%1.1fV",Vmax*0.1);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String," Vmin:%1.1fV",Vmin*0.1);
			LCD_DisplayStringLine(Line4,Lcd_Disp_String);
			break;
			
			case 1:
			sprintf((char *)Lcd_Disp_String,"      Para");
			LCD_DisplayStringLine(Line1,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String," Vmax:%1.1fV",Vmax_Disp*0.1);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);

			sprintf((char *)Lcd_Disp_String," Vmin:%1.1fV",Vmin_Disp*0.1);
			LCD_DisplayStringLine(Line4,Lcd_Disp_String);
			break;			
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
