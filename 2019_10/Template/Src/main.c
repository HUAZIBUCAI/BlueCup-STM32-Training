#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "iic\bsp_iic.h"
#include "tim\bsp_tim.h"
#include "adc\bsp_adc.h"


__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Long_Set_Point=0;


uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;

uint8_t EEPROM_String1[20]={1,2,3,4,5};
uint8_t EEPROM_String2[20];
uint8_t Res=0;

uint8_t InterFace=0x00;//0x00是第一个界面 0x10 0x11 0x12 0x13第二个界面分别表示MAX MIX UPPER LOWER
int8_t R37_Max=24;
int8_t R37_Min=12;
uint8_t Upper=0x80;
uint8_t Lower=0x01;
float Vlotage_R37;
int8_t LED_High=1;
int8_t LED_Low=0;
uint8_t Led_Crtl=0;
_Bool Led_Buling=0;
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

	I2CInit();
    AT24C02_Write(0,EEPROM_String1,5);
	HAL_Delay(1);
	AT24C02_Read(0,EEPROM_String2,5);
	MCP1407_Write(0x45);

//	PA6_TIM3_Init();
//	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);

	R38_ADC1_Init();
	R37_ADC2_Init();
while (1)
  {
	  
	Led_Proc();
    Key_Proc();
    Lcd_Proc();
  }
  
}


void Add_LED_R37(void)
{
	if(InterFace==0x10)
	{
		R37_Max+=3;
		if(R37_Max>33)R37_Max=33;
	}
	else if(InterFace==0x11)
	{
		R37_Min+=3;
		if(R37_Min>R37_Max)R37_Min=R37_Max;

	}

	else if(InterFace==0x12)
	{
		++LED_High;
		if(LED_High>7)LED_High=0;
	}
		else if(InterFace==0x13)
	{
		++LED_Low;
		if(LED_Low>7)LED_Low=0;	
	}

}


void Sub_LED_R37(void)
{
	if(InterFace==0x10)
	{
		R37_Max-=3;
		if(R37_Max<R37_Min)R37_Max=R37_Min;

	}

	else if(InterFace==0x11)
	{
		R37_Min-=3;
		if(R37_Min<0)R37_Min=0;
	}
	else if(InterFace==0x12)
	{
		--LED_High;
		if(LED_High<0)LED_High=7;
	}
		else if(InterFace==0x13)
	{
		--LED_Low;
		if(LED_Low<0)LED_Low=7;	
	}
	
}



static void Led_Proc(void)
{
	if(uwTick-uwTick_Led_Set_Point<=200)return;
	uwTick_Led_Set_Point=uwTick;

	if(Vlotage_R37>(0.1*R37_Max))
	{
		ucLed=(0x01<<LED_High);
	}
	else if(((0.1*R37_Min)<=Vlotage_R37)&&(Vlotage_R37<=(0.1*R37_Max)))
	{
		ucLed=0x00;
	}
	else
	{
		ucLed=(0x01<<LED_Low);
	}

		Led_Buling^=1;
	if(Led_Buling)
	{
		Led_Disp(ucLed);
	}
	else
	{
		Led_Disp(0x00);
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
		 LCD_Clear(White);
		if(	(InterFace>>4)==0x01)
		{
			InterFace=0x00;
		}
		else
		{
			InterFace=0x10;
		}
	}
	if(Key_Down==2)
	{
		 LCD_Clear(White);
		if(	(InterFace>>4)==0x01)
		{
			++InterFace;
			if(InterFace>0x13)
			{
				InterFace=0x10;
			}
			
		}

	}


	
		//判断长按 短按
		if(Key_Down)//一旦有按键按下开始计时
		{
			uwTick_Long_Set_Point=uwTick;
		}

	if((uwTick-uwTick_Long_Set_Point)<=800)//短按
	{
		switch(Key_Up)
		{
			case 3:
			Add_LED_R37();
			break;
			case 4:
			Sub_LED_R37();
			break;
		}


	}
	else
	{
	
		switch(Key_Value)//长按各个按键执行
		{

			case 3:
			Add_LED_R37();
			break;
			case 4:
			Sub_LED_R37();
			break;

		}

	}

}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;
	Vlotage_R37=Get_ADC2()*3.3/4096;

	if(InterFace==0x00)
	{
		sprintf((char *)Lcd_Disp_String,"        Main  ");
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);

		
		sprintf((char *)Lcd_Disp_String,"  Volt:%.2fV",Vlotage_R37);
		LCD_DisplayStringLine(Line6,Lcd_Disp_String);
		
		if(Vlotage_R37>(0.1*R37_Max))
		{
			sprintf((char *)Lcd_Disp_String,"  Status:Upper  ");
			LCD_DisplayStringLine(Line8,Lcd_Disp_String);
		}
		else if(((0.1*R37_Min)<=Vlotage_R37)&&(Vlotage_R37<=(0.1*R37_Max)))
		{
			sprintf((char *)Lcd_Disp_String,"  Status:Normal ");
			LCD_DisplayStringLine(Line8,Lcd_Disp_String);
		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"  Status:Lower  ");
			LCD_DisplayStringLine(Line8,Lcd_Disp_String);
		}
		
	}

	else
	{
		switch(InterFace)
		{
			case 0x10:
				sprintf((char *)Lcd_Disp_String,"        Setting");
				LCD_DisplayStringLine(Line3,Lcd_Disp_String);
				LCD_SetTextColor(Yellow);
				sprintf((char *)Lcd_Disp_String,"  Max Volt:%1.1fV",R37_Max*0.1);
				LCD_DisplayStringLine(Line4,Lcd_Disp_String);
				LCD_SetTextColor(Black);
				sprintf((char *)Lcd_Disp_String,"  Min Volt:%1.1fV",R37_Min*0.1);
				LCD_DisplayStringLine(Line5,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Upper :LD%d",LED_High+1);
				LCD_DisplayStringLine(Line6,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Lower :LD%d",LED_Low+1);
				LCD_DisplayStringLine(Line7,Lcd_Disp_String);
			break;
		
			case 0x11:
				sprintf((char *)Lcd_Disp_String,"        Setting");
				LCD_DisplayStringLine(Line3,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Max Volt:%1.1fV",R37_Max*0.1);
				LCD_DisplayStringLine(Line4,Lcd_Disp_String);
				LCD_SetTextColor(Yellow);
				sprintf((char *)Lcd_Disp_String,"  Min Volt:%1.1fV",R37_Min*0.1);
				LCD_DisplayStringLine(Line5,Lcd_Disp_String);
				LCD_SetTextColor(Black);
				sprintf((char *)Lcd_Disp_String,"  Upper :LD%d",LED_High+1);
				LCD_DisplayStringLine(Line6,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Lower :LD%d",LED_Low+1);
				LCD_DisplayStringLine(Line7,Lcd_Disp_String);

			break;

			case 0x12:
				sprintf((char *)Lcd_Disp_String,"        Setting");
				LCD_DisplayStringLine(Line3,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Max Volt:%1.1fV",R37_Max*0.1);
				LCD_DisplayStringLine(Line4,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Min Volt:%1.1fV",R37_Min*0.1);
				LCD_DisplayStringLine(Line5,Lcd_Disp_String);
				LCD_SetTextColor(Yellow);
				sprintf((char *)Lcd_Disp_String,"  Upper :LD%d",LED_High+1);
				LCD_DisplayStringLine(Line6,Lcd_Disp_String);
				LCD_SetTextColor(Black);
				sprintf((char *)Lcd_Disp_String,"  Lower :LD%d",LED_Low+1);
				LCD_DisplayStringLine(Line7,Lcd_Disp_String);

			break;

			case 0x13:
				sprintf((char *)Lcd_Disp_String,"        Setting");
				LCD_DisplayStringLine(Line3,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Max Volt:%1.1fV",R37_Max*0.1);
				LCD_DisplayStringLine(Line4,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Min Volt:%1.1fV",R37_Min*0.1);
				LCD_DisplayStringLine(Line5,Lcd_Disp_String);
				sprintf((char *)Lcd_Disp_String,"  Upper :LD%d",LED_High+1);
				LCD_DisplayStringLine(Line6,Lcd_Disp_String);
				LCD_SetTextColor(Yellow);
				sprintf((char *)Lcd_Disp_String,"  Lower :LD%d",LED_Low+1);
				LCD_DisplayStringLine(Line7,Lcd_Disp_String);
				LCD_SetTextColor(Black);

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
