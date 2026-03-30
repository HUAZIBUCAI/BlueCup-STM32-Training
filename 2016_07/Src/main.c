#include "main.h"
#include "RCC\bsp_rcc.h"
#include "KEY_LED\bsp_key_led.h"
#include "LCD\bsp_lcd.h"
#include "UART\bsp_uart.h"
#include "I2C\bsp_i2c.h"
#include "ADC\bsp_adc.h"
//#include "TIM\bsp_tim.h"
//#include "RTC\bsp_rtc.h"
#include "stdio.h"
#include "string.h"

//123456
//565656

//***全局变量声明区
//*减速变量
__IO uint32_t uwTick_Key_Set_Point = 0;//控制Key_Proc的执行速度
__IO uint32_t uwTick_Led_Set_Point = 0;//控制Led_Proc的执行速度
__IO uint32_t uwTick_Lcd_Set_Point = 0;//控制Lcd_Proc的执行速度
__IO uint32_t uwTick_Usart_Set_Point = 0;//控制Usart_Proc的执行速度
__IO uint32_t uwTick_Data_Collect_Set_Point = 0;//控制Data_Collect的执行速度
_Bool Level_Change_Flag=0;
uint8_t Buling_times=10;
_Bool UART_Change_Flag=0;
uint8_t UART_Buling_times=10;

//*按键扫描专用变量
uint8_t ucKey_Val, unKey_Down, ucKey_Up, ucKey_Old;

//*LED专用变量
uint8_t ucLed;

//*LCD显示专用变量
uint8_t Lcd_Disp_String[21];//最多显示20个字符

//*串口专用变量
uint16_t counter = 0;
uint8_t str[40];
uint8_t rx_buffer;

//*EEPROM的相关变量
int8_t EEPROM_String_1[5] = {0x11,0x22,0x33,0x44,0x55};
int8_t EEPROM_String_2[5] = {0};


uint8_t InterFace_Num=0x00;
float SUM_AD_R37;
float AVE_AD_R37;
uint8_t AD_Num=0;
uint8_t Height=0;
uint8_t Level_Old=0;
uint8_t Level=0;
int8_t Comapre_Level_Disp[3]={50,50,50};
int8_t Comapre_Level_Ctrl[3]={0};



//*4017相关变量
//uint8_t RES_4017;

//*pwm相关变量
//uint16_t PWM_T_Count;
//uint16_t PWM_D_Count;
//float PWM_Duty;


//*rtc相关变量
//RTC_TimeTypeDef H_M_S_Time;
//RTC_DateTypeDef Y_M_D_Date;


//***子函数声明区
void Key_Proc(void);
void Led_Proc(void);
void Lcd_Proc(void);
void Usart_Proc(void);
void Data_Collect_Proc(void);


//***系统主函数
int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

	/*bsp资源的初始化*/
	KEY_LED_Init();
	
	LCD_Init();
	LCD_Clear(White);
  LCD_SetBackColor(White);
  LCD_SetTextColor(Blue);	
	
	UART1_Init();
	I2CInit();
  ADC1_Init();
  ADC2_Init();
	
//	BASIC_TIM6_Init();
//	PWM_INPUT_TIM2_Init();
//	SQU_OUTPUT_TIM15_Init();
//	PWM_OUTPUT_TIM3_Init();
//	PWM_OUTPUT_TIM17_Init();
//	RTC_Init();



	/*外设使用基本配置*/	
	//*EEPROM测试
//	iic_24c02_write(EEPROM_String_1, 0, 5);
//	HAL_Delay(1);	
//	iic_24c02_read(EEPROM_String_2, 0, 5);	
	
  //*MCP4017测试
//	write_resistor(0x77);
//	RES_4017 = read_resistor();	
	
	//*串口接收中断打开
	HAL_UART_Receive_IT(&huart1, (uint8_t *)(&rx_buffer), 1);
	
	
	iic_24c02_read((uint8_t *)Comapre_Level_Ctrl,0,3);
	Comapre_Level_Disp[0]=Comapre_Level_Ctrl[0];
	Comapre_Level_Disp[1]=Comapre_Level_Ctrl[1];
	Comapre_Level_Disp[2]=Comapre_Level_Ctrl[2];
	//*打开基本定时器
//	HAL_TIM_Base_Start_IT(&htim6);//每100ms触发一次中断
		
	//*输入捕获PWM启动
//	HAL_TIM_Base_Start(&htim2);  /* 启动定时器 */
//  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);		  /* 启动定时器通道输入捕获并开启中断 */
//  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);		
		
	//*输出方波PA2引脚	
//  HAL_TIM_OC_Start_IT(&htim15,TIM_CHANNEL_1);
	
	//*启动定时器3和定时器17通道输出
//  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);	//PA6
//  HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);		//PA7
		

		
		
		
		
  while (1)
  {
		Key_Proc();
		Led_Proc();
		Lcd_Proc();
		Usart_Proc();
		Data_Collect_Proc();		
		
		
  }

}


//***按键扫描子函数
void Key_Proc(void)
{
	if((uwTick -  uwTick_Key_Set_Point)<50)	return;//减速函数
		uwTick_Key_Set_Point = uwTick;

	ucKey_Val = Key_Scan();
	unKey_Down = ucKey_Val & (ucKey_Old ^ ucKey_Val); 
	ucKey_Up = ~ucKey_Val & (ucKey_Old ^ ucKey_Val);	
	ucKey_Old = ucKey_Val;
	
	if(unKey_Down==1)
	{
		if(InterFace_Num==0x00)
		{
			InterFace_Num=0x10;
			LCD_Clear(White);
		}
		else
		{
			InterFace_Num=0x00;
			LCD_Clear(White);
			if((Comapre_Level_Disp[0]<=Comapre_Level_Disp[1])&&(Comapre_Level_Disp[1]<=Comapre_Level_Disp[2])&&(Comapre_Level_Disp[1]<=Comapre_Level_Disp[2]))
			{
				iic_24c02_write((uint8_t *)Comapre_Level_Disp,0,3);
				Comapre_Level_Ctrl[0]=Comapre_Level_Disp[0];
				Comapre_Level_Ctrl[1]=Comapre_Level_Disp[1];
				Comapre_Level_Ctrl[2]=Comapre_Level_Disp[2];
			}
			else
			{
				LCD_SetTextColor(Red);	
				sprintf((char *)Lcd_Disp_String, "  ERRO");
				LCD_DisplayStringLine(Line0, Lcd_Disp_String);
				HAL_Delay(1000);
				LCD_Clear(White);
				LCD_SetTextColor(Black);
			}
			
		}
	}
	if(unKey_Down==2)
	{
		LCD_Clear(White);
		if(InterFace_Num>>4==0x01)
			if(++InterFace_Num==0x13)
				InterFace_Num=0x10;
	}
	
	if(unKey_Down==3)
	{
		if(InterFace_Num==0x10)
		{
			Comapre_Level_Disp[0]+=5;
			if(Comapre_Level_Disp[0]>=100)Comapre_Level_Disp[0]=95;
		}
		
		if(InterFace_Num==0x11)
		{
			Comapre_Level_Disp[1]+=5;
			if(Comapre_Level_Disp[1]>=100)Comapre_Level_Disp[1]=95;
		}
		
		if(InterFace_Num==0x12)
		{
			Comapre_Level_Disp[2]+=5;
			if(Comapre_Level_Disp[2]>=100)Comapre_Level_Disp[2]=95;	
		}
			
	
	}
	if(unKey_Down==4)
	{
		if(InterFace_Num==0x10)
		{
			Comapre_Level_Disp[0]-=5;
			if(Comapre_Level_Disp[0]<=0)Comapre_Level_Disp[0]=5;
		}
		
		if(InterFace_Num==0x11)
		{
			Comapre_Level_Disp[1]-=5;
			if(Comapre_Level_Disp[1]<=0)Comapre_Level_Disp[1]=5;
		}
		
		if(InterFace_Num==0x12)
		{
			Comapre_Level_Disp[2]-=5;
			if(Comapre_Level_Disp[2]<=0)Comapre_Level_Disp[2]=5;	
		}
	}		
	
}


//***LED扫描子函数
void Led_Proc(void)
{
	if((uwTick -  uwTick_Led_Set_Point)<200)	return;//减速函数
		uwTick_Led_Set_Point = uwTick;

	LED_Disp(ucLed);
	if(Level_Change_Flag)
	{
		if(Buling_times--)
		{
		ucLed^=0x02;
			
		if(Buling_times==0)
		{
		Buling_times=10;
		Level_Change_Flag=0;
		}			
		}
	
	}
	

		if(UART_Change_Flag)
	{
		if(UART_Buling_times--)
		{
		ucLed^=0x04;
			
		if(UART_Buling_times==0)
		{
		UART_Buling_times=10;
		UART_Change_Flag=0;
		}			
		}
	
	}


}

void Lcd_Proc(void)
{
	if((uwTick -  uwTick_Lcd_Set_Point)<100)	return;//减速函数
		uwTick_Lcd_Set_Point = uwTick;
	
	//开机屏幕测试代码
//	sprintf((char *)Lcd_Disp_String, "TEST CODE:%02X",InterFace_Num);
//	LCD_SetBackColor(White);
//	LCD_DisplayStringLine(Line9, Lcd_Disp_String);	
	
	
	if(InterFace_Num==0x00)
	{
		sprintf((char *)Lcd_Disp_String, "    Lique Level");
		LCD_DisplayStringLine(Line1, Lcd_Disp_String);	
		sprintf((char *)Lcd_Disp_String, "    Height:%3d",Height);
		LCD_DisplayStringLine(Line3, Lcd_Disp_String);	
		sprintf((char *)Lcd_Disp_String, "    ADC:%.2f",AVE_AD_R37);
		LCD_DisplayStringLine(Line5, Lcd_Disp_String);	
		sprintf((char *)Lcd_Disp_String, "    Level:%1d",Level);
		LCD_DisplayStringLine(Line7, Lcd_Disp_String);	
	
	}
	
	else if((InterFace_Num>>4)==0x01)
	{
		sprintf((char *)Lcd_Disp_String, "    Parameter Setup");
		LCD_SetBackColor(White);
		LCD_DisplayStringLine(Line1, Lcd_Disp_String);	
	
		sprintf((char *)Lcd_Disp_String, "    Threshold 1:%2d",Comapre_Level_Disp[0]);
		if(InterFace_Num==0x10)		LCD_SetBackColor(Yellow);
		else		LCD_SetBackColor(White);		
		LCD_DisplayStringLine(Line3, Lcd_Disp_String);	
		
		sprintf((char *)Lcd_Disp_String, "    Threshold 2:%2d",Comapre_Level_Disp[1]);
		if(InterFace_Num==0x11)		LCD_SetBackColor(Yellow);
		else		LCD_SetBackColor(White);	
		LCD_DisplayStringLine(Line5, Lcd_Disp_String);	
		
		sprintf((char *)Lcd_Disp_String, "    Threshold 3:%2d",Comapre_Level_Disp[2]);
		if(InterFace_Num==0x12)		LCD_SetBackColor(Yellow);
		else		LCD_SetBackColor(White);	
		LCD_DisplayStringLine(Line7, Lcd_Disp_String);	
	
	
	}
	
	
	
	//*EEPROM测试
//	sprintf((char *)Lcd_Disp_String, "EE:%x%x%x%x%x R:%x",EEPROM_String_2[0],EEPROM_String_2[1],EEPROM_String_2[2],EEPROM_String_2[3],EEPROM_String_2[4], RES_4017);
//	LCD_DisplayStringLine(Line0, Lcd_Disp_String);

	//*ADC测试
//	sprintf((char *)Lcd_Disp_String, "R38_Vol:%6.3fV",((((float)getADC1())/4096)*3.3));
//	LCD_DisplayStringLine(Line1, Lcd_Disp_String);	
	
	
	
	//*PWM输入捕获测试，测量占空比和频率
//	sprintf((char *)Lcd_Disp_String, "R40P:%05dHz,%4.1f%%",(unsigned int)(1000000/PWM_T_Count),PWM_Duty*100);
//	LCD_DisplayStringLine(Line3, Lcd_Disp_String);		
	
	//*RTC内容显示
//	HAL_RTC_GetTime(&hrtc, &H_M_S_Time, RTC_FORMAT_BIN);//读取日期和时间必须同时使用
//	HAL_RTC_GetDate(&hrtc, &Y_M_D_Date, RTC_FORMAT_BIN);
//	sprintf((char *)Lcd_Disp_String, "Time:%02d-%02d-%02d",(unsigned int)H_M_S_Time.Hours,(unsigned int)H_M_S_Time.Minutes,(unsigned int)H_M_S_Time.Seconds);
//	LCD_DisplayStringLine(Line4, Lcd_Disp_String);		

}

void Usart_Proc(void)
{
//	if((uwTick -  uwTick_Usart_Set_Point)<1000)	return;//减速函数
//	uwTick_Usart_Set_Point = uwTick;
//	
//	sprintf((char*)str, "%04d:Hello,world.\r\n", counter);
//	HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);
//	
//	if(++counter == 10000)
//		counter = 0;
}


void Data_Collect_Proc(void)
{
	if((uwTick -  uwTick_Data_Collect_Set_Point)<1000)	return;//减速函数
	uwTick_Data_Collect_Set_Point = uwTick;
	

	for(AD_Num=0;AD_Num<=9;AD_Num++)
	{
		SUM_AD_R37+=(float)getADC2();
	}
	SUM_AD_R37/=4096;
	SUM_AD_R37*=3.3;
	AVE_AD_R37=SUM_AD_R37/10;
	Height=(uint8_t)AVE_AD_R37*33.3;
	SUM_AD_R37=0;
	
	Level_Old=Level;
	if(Height<Comapre_Level_Ctrl[0])Level=0;
	else if(Height<Comapre_Level_Ctrl[1])Level=1;
	else if(Height<Comapre_Level_Ctrl[2])Level=2;
	else Level=3;
	
	if(Level_Old<Level)
	{
		sprintf((char*)str, "A:H%3d+L%1d+U\r\n", Height,Level);
		HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);
		Level_Change_Flag=1;
	}
	if(Level_Old>Level)
	{
		sprintf((char*)str, "A:H%3d+L%1d+D\r\n", Height,Level);
		HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);
		Level_Change_Flag=1;
	}
	ucLed^=0x01;
}

//串口接收中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	
	if(rx_buffer=='C')
	{
		sprintf((char*)str, "C:H%2d+L%1d\r\n", Height,Level);
		HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);
		UART_Change_Flag=1;
	
	}
	if(rx_buffer=='S')
	{
		sprintf((char*)str, "S:TL%2d+TM%2d+TH%2d\r\n",Comapre_Level_Disp[0],Comapre_Level_Disp[1],Comapre_Level_Disp[2]);
		HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);
		UART_Change_Flag=1;
	}
	
	HAL_UART_Receive_IT(&huart1, (uint8_t *)(&rx_buffer), 1);
//	counter++;
//	sprintf((char*)str, "%04d:Hello,world.\r\n", counter);
//	HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen((const char*)str), 50);	
//	
//	HAL_UART_Receive_IT(&huart1, (uint8_t *)(&rx_buffer), 1);

}



//基本定时器6更新回调函数
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim->Instance==TIM6)
//  {
////		if(++counter == 10)
////		{
////			counter = 0;
////			sprintf(str, "Hello,world.\r\n");
////			HAL_UART_Transmit(&huart1,(unsigned char *)str, strlen(str), 50);	
////		}
//	}
//}


//输入捕获PWM中断回调
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
//{
//	  if(htim->Instance==TIM2)
//  {
//				if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//			{
//				PWM_T_Count =  HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
//				PWM_Duty = (float)PWM_D_Count/PWM_T_Count;
//			}
//				else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//			{
//				PWM_D_Count =  HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
//			}			
//	}	
//}




//方波输出回调函数
//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
//{
//  if(htim->Instance==TIM15)
//  {
//				if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//			{			
//			  __HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_1,(__HAL_TIM_GetCounter(htim)+500));//1Khz
//			}
//	}
//}







//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************

void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}
