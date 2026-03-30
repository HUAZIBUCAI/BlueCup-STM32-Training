#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "iic\bsp_iic.h"
#include "tim\bsp_tim.h"


__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Long_Short_Set_Point=0;
__IO uint32_t uwTick_Time_Set_Point=0;
__IO uint32_t uwTick_Led_Ctrl_Set_Point=0;


uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
__IO uint8_t ucLed=0;

// uint8_t EEPROM_String1[20]={1,2,3,4,5};
// uint8_t EEPROM_String2[20];
// uint8_t Res=0;

uint8_t ucState=0;
uint8_t Storage_Num=0;
uint8_t ucHour_Min_Sec[3]={1,4,55};
uint32_t Second_Buf=0;
_Bool Second_Flag=0;

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
    // AT24C02_Write(0,EEPROM_String1,5);
	// HAL_Delay(1);
	// AT24C02_Read(0,EEPROM_String2,5);
	// MCP1407_Write(0x45);

	PA6_TIM3_Init();
	
while (1)
  {
	  
	Led_Proc();
    Key_Proc();
    Lcd_Proc();
  }
  
}


void B3_Ctrl(void)
{
	switch(ucState)
	{
		case 1:
		if(++ucHour_Min_Sec[0]==24)ucHour_Min_Sec[0]=0;
		break;
		case 2:
		if(++ucHour_Min_Sec[1]==60)ucHour_Min_Sec[1]=0;
		break;
		case 3:
		if(++ucHour_Min_Sec[2]==60)ucHour_Min_Sec[2]=0;
		break;

	}


}

static void Led_Proc(void)
{
	// 精准的 500ms 定时器
    if(uwTick - uwTick_Led_Ctrl_Set_Point >= 500)
    {
        uwTick_Led_Ctrl_Set_Point = uwTick;
        
        // 1. 判断当前应该亮什么灯
        if(ucState == 4) 
        {
            // Running 模式：让 LED1 (0x01) 闪烁
            ucLed ^= 0x01;  
        }
        else 
        {
            // 其他所有模式（包括暂停、设置、待机）：全部熄灭
            ucLed = 0x00;   
        }
        
        // 2. 将算好的变量交给底层执行（【关键修复】绝对不能写死 0xff！）
        Led_Disp(ucLed); 
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
	
	// if(Key_Down==1)ucLed=0xff;
	// if(Key_Down==2)ucLed=0xf0;
	// if(Key_Down==3)ucLed=0x0f;
	// if(Key_Down==4)ucLed=0x00;

	if(Key_Down)
	{
		uwTick_Long_Short_Set_Point=uwTick;
	}
	if(uwTick-uwTick_Long_Short_Set_Point<=800)//小于八百毫秒
	{
		switch(Key_Up)
		{
			case 0:
			break;
			case 1:
			if(++Storage_Num==5)Storage_Num=0;
			AT24C02_Read(Storage_Num*3,ucHour_Min_Sec,3);

			break;
			case 2:
			if(++ucState>=4)
			ucState=1;
			break;
			case 3:
			B3_Ctrl();
			break;
			case 4:
			if(ucState!=4)//倒计时开始
			{
				ucState=4;
				HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
				Second_Buf=ucHour_Min_Sec[0]*3600+ucHour_Min_Sec[1]*60+ucHour_Min_Sec[2];
				uwTick_Time_Set_Point=uwTick;
				Second_Flag=1;
				
			}
			else//状态五暂停
			{
				ucState=5;
				HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
				Second_Flag=0;
			}
			
			break;
		}


	}
	else//长按
	{
		switch(Key_Value)
		{
		case 0:
		//uwTick_Long_Short_Set_Point=uwTick;
		break;
		case 1:
		
		break;
		case 2:
		if(ucState==1||ucState==2||ucState==3)
		{
			AT24C02_Write(Storage_Num*3,ucHour_Min_Sec,3);
			ucState=0;
		}
		break;
		case 3:
		B3_Ctrl();
		break;
		case 4:
		if(ucState==4)
		{
			ucState=0;
		}
		
		break;

		}
	}


}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;
	sprintf((char *)Lcd_Disp_String,"Hello Blue Cup");
	LCD_DisplayStringLine(Line1,Lcd_Disp_String);

	sprintf((char *)Lcd_Disp_String,"%d Storage:%d",ucState,Storage_Num);
	LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	switch(ucState)
	{
		case 0:
		LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
		LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));

		LCD_DisplayChar(Line5,192,':');

		LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
		LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));

		LCD_DisplayChar(Line5,144,':');

		LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
		LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));

		sprintf((char *)Lcd_Disp_String,"    Standby");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		break;

		case 1:
		LCD_SetTextColor(Yellow);
		LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
		LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));

		LCD_SetTextColor(Black);
		LCD_DisplayChar(Line5,192,':');

		LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
		LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));

		LCD_DisplayChar(Line5,144,':');

		LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
		LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));
		sprintf((char *)Lcd_Disp_String,"    Setting");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		
		break;

		case 2:

		LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
		LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));
		LCD_DisplayChar(Line5,192,':');

		LCD_SetTextColor(Yellow);
		LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
		LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));
		
		LCD_SetTextColor(Black);
		LCD_DisplayChar(Line5,144,':');

		LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
		LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));
		sprintf((char *)Lcd_Disp_String,"    Setting");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		break;

		case 3:
		LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
		LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));
		
		LCD_DisplayChar(Line5,192,':');
		
		LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
		LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));
		
		LCD_DisplayChar(Line5,144,':');
		
		LCD_SetTextColor(Yellow);
		LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
		LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));
		LCD_SetTextColor(Black);
		sprintf((char *)Lcd_Disp_String,"    Setting");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		break;

		case 4:
		if(uwTick-uwTick_Time_Set_Point>=1000)
		{
			uwTick_Time_Set_Point=uwTick;
			if(Second_Flag)--Second_Buf;
			ucHour_Min_Sec[0]=Second_Buf/3600;
			ucHour_Min_Sec[1]=(Second_Buf-ucHour_Min_Sec[0]*3600)/60;
			ucHour_Min_Sec[2]=Second_Buf-ucHour_Min_Sec[0]*3600-ucHour_Min_Sec[1]*60;
			LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
			LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));

			LCD_DisplayChar(Line5,192,':');

			LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
			LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));

			LCD_DisplayChar(Line5,144,':');

			LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
			LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));

			
		}

		sprintf((char *)Lcd_Disp_String,"    Runn123");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		break;
		case 5:
		LCD_DisplayChar(Line5,224,(ucHour_Min_Sec[0]/10+0x30));
		LCD_DisplayChar(Line5,208,(ucHour_Min_Sec[0]%10+0x30));

		LCD_DisplayChar(Line5,192,':');

		LCD_DisplayChar(Line5,176,(ucHour_Min_Sec[1]/10+0x30));
		LCD_DisplayChar(Line5,160,(ucHour_Min_Sec[1]%10+0x30));

		LCD_DisplayChar(Line5,144,':');

		LCD_DisplayChar(Line5,128,(ucHour_Min_Sec[2]/10+0x30));
		LCD_DisplayChar(Line5,112,(ucHour_Min_Sec[2]%10+0x30));
		sprintf((char *)Lcd_Disp_String,"    Pausing");
		LCD_DisplayStringLine(Line7,Lcd_Disp_String);
		break;
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
