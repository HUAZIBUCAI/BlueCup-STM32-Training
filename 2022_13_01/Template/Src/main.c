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
__IO uint32_t uwTick_Alarm_Flag_Set_Point=0;


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
uint8_t PSWD_Interface[5]={1,2,3,'-'};
_Bool Interface_Flag=0;
uint16_t Out_PWM=2000;
uint8_t Out_Duty=10;

uint8_t Rx_Buffer[30];
uint8_t RX_Counter=0;
uint8_t Failed_Times=0;
_Bool Alarm=0;
_Bool Buling=0;
_Bool MoRenB1=0;
_Bool MoRenB2=0;
_Bool MoRenB3=0;



static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void Uart_Proc(void);
_Bool CheckCmd(uint8_t *uart_str);


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
	if(uwTick-uwTick_Led_Set_Point<=100)return;
	uwTick_Led_Set_Point=uwTick;
	Led_Disp(ucLed);
	if(Alarm)
	{
		if(uwTick-uwTick_Alarm_Flag_Set_Point<=5000)
		{
			Buling^=1;
			if(Buling)
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
			Alarm=0;
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
	
	// if(Key_Down==1)ucLed=0xff;
	// if(Key_Down==2)ucLed=0xf0;
	// if(Key_Down==3)ucLed=0x0f;
	// if(Key_Down==4)ucLed=0x00;
	if(Interface_Flag==0)
	{
		
		if(Key_Down==1)
		{
			if(MoRenB1)
			{
				++B1_Num;
				if(B1_Num>9)
				{
					B1_Num=0;
				}
			}
			MoRenB1=1;

			
		}

	
		if(Key_Down==2)
		{
			if(MoRenB2)
			{
				++B2_Num;
				if(B2_Num>9)
				{
					B2_Num=0;
				}
			}
			MoRenB2=1;
		}

	
		if(Key_Down==3)
		{
			if(MoRenB3)
			{	
				++B3_Num;
				if(B3_Num>9)
				{
					B3_Num=0;
				}
			}
			MoRenB3=1;
		}
	}


	if(Key_Down==4)
	{
		LCD_Clear(Black);
		if( (B1_Num==PSWD_Interface[0])&&(B2_Num==PSWD_Interface[1])&&(B3_Num==PSWD_Interface[2]) )
		{
			
			__HAL_TIM_SET_AUTORELOAD(&htim2,499);
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,50);
			__HAL_TIM_SET_COUNTER(&htim2, 0);			
			Interface_Flag=1;
			ucLed=0x01;
			Led_Disp(ucLed);
			Failed_Times=0;
			uwTick_PSWD_Flag_Set_Point=uwTick;
			
		}
		else
		{
			Failed_Times++;
			if(Failed_Times>=3)
			{
				Alarm=1;
				uwTick_Alarm_Flag_Set_Point=uwTick;
			}
			B1_Num=0;
			B2_Num=0;
		 	B3_Num=0;
			MoRenB1=0;
			MoRenB2=0;
			MoRenB3=0;
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
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);

		if(MoRenB1)
		{
			sprintf((char *)Lcd_Disp_String,"    B1:%d",B1_Num);
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"    B1:@");
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		}

		if(MoRenB2)
		{
			sprintf((char *)Lcd_Disp_String,"    B2:%d",B2_Num);
			LCD_DisplayStringLine(Line4,Lcd_Disp_String);
		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"    B2:@");
			LCD_DisplayStringLine(Line4,Lcd_Disp_String);

		}

		
		if(MoRenB3)
		{
			sprintf((char *)Lcd_Disp_String,"    B3:%d",B3_Num);
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);
		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"    B3:@");
			LCD_DisplayStringLine(Line5,Lcd_Disp_String);

		}
	
	}

	else if(Interface_Flag==1)
	{
		sprintf((char *)Lcd_Disp_String,"       STA");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"    F:%dHz",Out_PWM);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
	
		sprintf((char *)Lcd_Disp_String,"    D:%d%%",Out_Duty);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
		
	

		if(uwTick-uwTick_PSWD_Flag_Set_Point<=5000)
		{
			return;
		}
		else
		{
			
			__HAL_TIM_SET_AUTORELOAD(&htim2,999);
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,500);
			__HAL_TIM_SET_COUNTER(&htim2, 0);
	   		LCD_Clear(Black);
			Interface_Flag=0;
			ucLed=0x00;
			Led_Disp(ucLed);
			MoRenB1=0;
			MoRenB2=0;
			MoRenB3=0;
			//HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_2);
			
			//HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
		}
	


	}

}


static void Uart_Proc(void)
{
	if(uwTick-uwTick_Uart_Set_Point<=200)return;
	uwTick_Uart_Set_Point=uwTick;
	
	// Counter++;
	// sprintf( (char *)Lcd_Disp_String,(const char *)"%dHello,World\r\n",Counter);
	// HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);

	if(RX_Counter!=0)
	{
		if(CheckCmd(Rx_Buffer))
		{
			sprintf( (char *)Lcd_Disp_String,"PSWD is Right ,Password change success\r\n");
			HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);
		}
		else
		{
			sprintf( (char *)Lcd_Disp_String,"PSWD is Error ,Password change failed\r\n");
			HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);
			memset(Rx_Buffer,0,sizeof(Rx_Buffer) );
		}	

	}

}


_Bool CheckCmd(uint8_t *uart_str)
{
	if(RX_Counter!=7)
	{
		RX_Counter=0;
		return 0;
	}

	if( (Rx_Buffer[0]== (PSWD_Interface[0]+'0') )&&(Rx_Buffer[1]==(PSWD_Interface[1]+'0') )&&
	(Rx_Buffer[2]==(PSWD_Interface[2]+'0') )&&(Rx_Buffer[3]==PSWD_Interface[3])  )
	{
		if( ('0'<=Rx_Buffer[4]&&Rx_Buffer[4]<='9')&&('0'<=Rx_Buffer[5]&&Rx_Buffer[5]<='9')&&('0'<=Rx_Buffer[6]&&Rx_Buffer[6]<='9')  )
		{
			PSWD_Interface[0]=Rx_Buffer[4]-'0';
			PSWD_Interface[1]=Rx_Buffer[5]-'0';
			PSWD_Interface[2]=Rx_Buffer[6]-'0';
			RX_Counter=0;
			return 1; 
		}
		else
		{
			RX_Counter=0;
			return 0;
		}

	}
	else
	{
		RX_Counter=0;
		return 0;
	}
	
	
}

 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 {
	// ucLed=0xff;
	// Led_Disp(ucLed);
	// HAL_Delay(300);
	// ucLed=0x00;

	Rx_Buffer[RX_Counter++]=rxbuffer;

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
