#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"
#include "uart\bsp_uart.h"


__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Uart_Set_Point=0;
__IO uint32_t uwTick_PSWD_Flag_Set_Point=0;


uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;
uint8_t Counter=0;
uint8_t rxbuffer=0;

uint8_t B1_Num=0;
uint8_t B2_Num=0;
uint8_t B3_Num=0;
uint8_t PSWD_Interface[3]={1,2,3};
_Bool Interface_Flag=0;
uint16_t Out_PWM=2000;
uint8_t Out_Duty=10;

static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void Uart_Proc(void);


int main(void)
{

	HAL_Init();
	
    SystemClock_Config();
    Led_Key_GPIO_Init();
 
    LCD_Init();
    LCD_Clear(Black);
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);

	PA1_TIM2_Init();
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);

	MY_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1,&rxbuffer,1);

while (1)
  {
	  
	Led_Proc();
    Key_Proc();
    Lcd_Proc();
	Uart_Proc();
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
	if(Interface_Flag==0)
	{
		if(Key_Down==1)
		{
			++B1_Num;
			if(B1_Num>9)
			{
				B1_Num=0;
			}
		}

	
		if(Key_Down==2)
		{
			++B2_Num;
			if(B2_Num>9)
			{
				B2_Num=0;
			}
		}

	
		if(Key_Down==3)
		{
			++B3_Num;
			if(B3_Num>9)
			{
				B3_Num=0;
			}
		}
	}


	if(Key_Down==4)
	{

		if( (B1_Num==PSWD_Interface[0])&&(B2_Num==PSWD_Interface[1])&&(B3_Num==PSWD_Interface[2]) )
		{
			//HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_2);
			__HAL_TIM_SET_AUTORELOAD(&htim2,499);
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,50);
			//HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
			LCD_Clear(Black);
			Interface_Flag=1;
			uwTick_PSWD_Flag_Set_Point=uwTick;
			
		}
		else
		{
			B1_Num=0;
			B2_Num=0;
		 	B3_Num=0;
		}
	}
}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;

 	if(Interface_Flag==0)
	{
		
		sprintf((char *)Lcd_Disp_String,"       PSD");
		LCD_DisplayStringLine(Line2,Lcd_Disp_String);


		sprintf((char *)Lcd_Disp_String,"    B1:%d",B1_Num);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
	
		sprintf((char *)Lcd_Disp_String,"    B2:%d",B2_Num);
		LCD_DisplayStringLine(Line5,Lcd_Disp_String);
	
		sprintf((char *)Lcd_Disp_String,"    B3:%d",B3_Num);
		LCD_DisplayStringLine(Line6,Lcd_Disp_String);
	}	
	else if(Interface_Flag==1)
	{
		sprintf((char *)Lcd_Disp_String,"       STA");
		LCD_DisplayStringLine(Line2,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"    F:%d",Out_PWM);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
	
		sprintf((char *)Lcd_Disp_String,"    D:%d%%",Out_Duty);
		LCD_DisplayStringLine(Line5,Lcd_Disp_String);
		
	

		if(uwTick-uwTick_PSWD_Flag_Set_Point<=5000)
		{
			return;
		}
		else
		{
	   		LCD_Clear(Black);
			Interface_Flag=0;
			//HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_2);
			__HAL_TIM_SET_AUTORELOAD(&htim2,999);
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,500);
			//HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
		}
	


	}

}


static void Uart_Proc(void)
{
	if(uwTick-uwTick_Uart_Set_Point<=1000)return;
	uwTick_Uart_Set_Point=uwTick;
	// Counter++;
	// sprintf( (char *)Lcd_Disp_String,(const char *)"%dHello,World\r\n",Counter);
	// HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);

}


 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 {
	// ucLed=0xff;
	// Led_Disp(ucLed);
	// HAL_Delay(300);
	// ucLed=0x00;
	

	HAL_UART_Receive_IT(&huart1,&rxbuffer,1);
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
