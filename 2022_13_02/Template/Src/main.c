#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"
#include "uart\bsp_uart.h"
#include "iic\bsp_iic.h"

__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Uart_Set_Point=0;
__IO uint32_t uwTick_LD1_Set_Point=0;


#define EEPROM_X_INV_ADDR  0x00  // X库存地址
#define EEPROM_Y_INV_ADDR  0x01  // Y库存地址
#define EEPROM_X_PRC_ADDR  0x02  // X价格地址
#define EEPROM_Y_PRC_ADDR  0x03  // Y价格地址
#define EEPROM_FLAG_ADDR   0x05  // 魔法标志位地址（极度关键）

// 定义全局变量
uint8_t X_Inventory = 0;
uint8_t X_Price_x10 = 0; // 存 10 代表 1.0
uint8_t Y_Inventory = 0;
uint8_t Y_Price_x10 = 0;

uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0x00;

uint8_t EEPROM_String1[5]={1,2,3,4,5};
uint8_t EEPROM_String2[5]={0};
uint8_t rxbuffer=0;

uint8_t IntereFace=0x00;

uint8_t Purchase_X=0;
uint8_t Purchase_Y=0;

double Price_X=1.0;
double Price_Y=1.0;
double Total=0.0;

uint8_t Price_X_10=10;
uint8_t Price_Y_10=10;


uint8_t Rep_X=10;
uint8_t Rep_Y=10;

_Bool Buling=0;

static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void Uart_Proc(void);
static void Uart_Proc(void);
void EEPROM_Init_Check(void);

int main(void)
{

	HAL_Init();
	
    SystemClock_Config();
    Led_Key_GPIO_Init();
 
    LCD_Init();
    LCD_Clear(Black);
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);

	I2CInit();
	EEPROM_Init_Check();
	MY_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1,&rxbuffer,1);

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
	if(Rep_X==0&&Rep_Y==0)
	{
		Buling^=1;
		if(Buling)
		{		
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
		}
	}


	if(uwTick-uwTick_LD1_Set_Point<=5000)
	{
		return;
	}
	else
	{
		ucLed=0x00;
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,25);
		__HAL_TIM_SET_COUNTER(&htim2,0);

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
		if( (IntereFace>>4)==0x00 )
		{
			IntereFace=0x10;
		}
		
		else if( (IntereFace>>4)==0x01 )
		{
			IntereFace=0x20;
		}
		else if((IntereFace>>4)==0x02)
		{
			IntereFace=0x00;
		}
	}

	if(Key_Down==2)
	{
		LCD_Clear(Black);
		if( (IntereFace>>4)==0x00 )
		{
			Purchase_X+=1;
			if(Purchase_X>=Rep_X)Purchase_X=Rep_X;
		}
		
		else if( (IntereFace>>4)==0x01 )
		{
			Price_X+=0.1;
			if(Price_X>=2.1)Price_X=1.0;
			 Price_X_10=Price_X*10;
			AT24C02_Write(&Price_X_10, EEPROM_X_PRC_ADDR,1);
		}
		else if((IntereFace>>4)==0x02)
		{
			Rep_X+=1;
			AT24C02_Write(&Rep_X, EEPROM_X_INV_ADDR,1);
		}


	}

	
	if(Key_Down==3)
	{
		LCD_Clear(Black);
		if( (IntereFace>>4)==0x00 )
		{
			Purchase_Y+=1;
			if(Purchase_Y>=Rep_Y)Purchase_Y=Rep_Y;
		}
		
		else if( (IntereFace>>4)==0x01 )
		{
			Price_Y+=0.1;
			if(Price_Y>=2.1)Price_Y=1.0;
			 Price_Y_10=Price_Y*10;
	        AT24C02_Write(&Price_Y_10, EEPROM_Y_PRC_ADDR,1);
		}
		else if((IntereFace>>4)==0x02)
		{
			Rep_Y+=1;
			  AT24C02_Write(&Rep_Y, EEPROM_Y_INV_ADDR,1);
		}
	}

	if(Key_Down==4)
	{
		LCD_Clear(Black);
		Rep_X-=Purchase_X;
		Rep_Y-=Purchase_Y;
		Total=Purchase_X*Price_X+Purchase_Y*Price_Y;
		sprintf((char *)Lcd_Disp_String,"X:%d,Y:%d,Z:%.1f",Purchase_X,Purchase_Y,Total);
		HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);
		Purchase_X=0;
		Purchase_Y=0;
		AT24C02_Write(&Rep_X, EEPROM_X_INV_ADDR,1);
		HAL_Delay(5);
        AT24C02_Write(&Rep_Y, EEPROM_Y_INV_ADDR,1);
        uwTick_LD1_Set_Point=uwTick;
		ucLed=0x01;
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,150);
		__HAL_TIM_SET_COUNTER(&htim2,0);
		
	}

}
static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;

	if(IntereFace>>4==0x00)
	{
		sprintf((char *)Lcd_Disp_String,"        SHOP");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     X:%d",Purchase_X);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     Y:%d",Purchase_Y);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
		
	}



	else if(IntereFace>>4==0x01)
	{
		sprintf((char *)Lcd_Disp_String,"        PRISE");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     X:%.1f",Price_X);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     Y:%.1f",Price_Y);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);

	}

	
	else if(IntereFace>>4==0x02)
	{
		sprintf((char *)Lcd_Disp_String,"        REP");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     X:%d",Rep_X);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     Y:%d",Rep_Y);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);


	}
	
	// sprintf((char *)Lcd_Disp_String,"Hello Blue Cup");
	// LCD_DisplayStringLine(Line1,Lcd_Disp_String);
	
	// sprintf((char *)Lcd_Disp_String,"%d   %02x",EEPROM_String2[3],IntereFace);
	// LCD_DisplayStringLine(Line2,Lcd_Disp_String);


}

static void Uart_Proc(void)
{
	if(uwTick-uwTick_Uart_Set_Point<=200)return;
	uwTick_Uart_Set_Point=uwTick;
	

}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(rxbuffer=='?')
	{
		sprintf((char *)Lcd_Disp_String,"X:%.1f,Y:%.1f",Price_X,Price_Y);
		HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);
	}

	HAL_UART_Receive_IT(&huart1,&rxbuffer,1);
}

void EEPROM_Init_Check(void)
{
    uint8_t flag = 0;
    uint8_t biaozi=0x03;
    // 1. 读取标志位
    AT24C02_Read(&flag,EEPROM_FLAG_ADDR,1); 
    
    if(flag == 0x03) 
    {
        // 【情况A】不是第一次开机，以前存过数据，直接读取！
        AT24C02_Read(&Rep_X, EEPROM_X_INV_ADDR,1);
		HAL_Delay(5);
        AT24C02_Read(&Rep_Y, EEPROM_Y_INV_ADDR,1);
		
        AT24C02_Read(&Price_X_10, EEPROM_X_PRC_ADDR,1);
		Price_X=Price_X_10*0.1;
        AT24C02_Read(&Price_Y_10, EEPROM_Y_PRC_ADDR,1);
		Price_Y=Price_Y_10*0.1;
    }
    else 
    {
        // 【情况B】第一次开机（乱码），强行写入题目要求的默认值！
        // 把默认值存入 EEPROM，防止下次开机还是乱码
        AT24C02_Write(&Rep_X, EEPROM_X_INV_ADDR,1);
        HAL_Delay(5); // 🚨 EEPROM 每次写完必须延时 5ms！绝对不能忘！
        
        AT24C02_Write(&Rep_Y, EEPROM_Y_INV_ADDR,1);
        HAL_Delay(5);

        Price_X_10=Price_X*10;
        AT24C02_Write(&Price_X_10, EEPROM_X_PRC_ADDR,1);
        HAL_Delay(5);

         Price_Y_10=Price_Y*10;
        AT24C02_Write(&Price_Y_10, EEPROM_Y_PRC_ADDR,1);
        HAL_Delay(5);
        
        // 核心：刻下魔法标志位 0x55，下次开机就认识了！
        AT24C02_Write(&biaozi,EEPROM_FLAG_ADDR,1);
        HAL_Delay(5);
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
