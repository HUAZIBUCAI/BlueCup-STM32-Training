#include "main.h"
#include "rcc\bsp_rcc.h"
#include "led_key\bsp_led_key.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "uart\bsp_uart.h"
#include "tim\bsp_tim.h"


__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Uart_Set_Point=0;

uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;
uint8_t Count=0;
uint8_t rxbuffer;
uint8_t Interface=0x00;

uint8_t Cnbr=0;
uint8_t Vnbr=0;
uint8_t Idel=8;
double CNBR_Price=3.5;
double VNBR_Price=2.0;

_Bool PA7_Flag=0;
uint8_t RX_BUF[50];
uint8_t RX_Counter;
uint8_t str_str[20];


typedef struct 
{
	uint8_t type[5];
	uint8_t id[5];
	uint8_t year_in;
	uint8_t month_in;
	uint8_t day_in;
	uint8_t hour_in;
	uint8_t min_in;
	uint8_t sec_in;
	_Bool NotEmpty;
}Car_Data_Storage_TypeDef;

Car_Data_Storage_TypeDef Car_Data_Storage[8];//存储8个进来的车的信息

static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void Uart_Proc(void);
_Bool Check_Cmd(uint8_t * str);
void substr(uint8_t * d_str,uint8_t * s_str,uint8_t loacte,uint8_t len);
uint8_t idExist(uint8_t* str);//检查id是否存在
uint8_t FindLocate(void);


int main(void)
{

	HAL_Init();
    SystemClock_Config();
	// Car_Data_Storage[5].id[0]='M';
	// Car_Data_Storage[5].id[1]='Y';
	// Car_Data_Storage[5].id[2]='G';
	// Car_Data_Storage[5].id[3]='C';
	// Car_Data_Storage[5].id[4]='\0';

    Led_Key_GPIO_Init();
 
    LCD_Init();
    LCD_Clear(Black);
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);
	
	Park_USART1_UART_Init();

	HAL_UART_Receive_IT(&huart1,&rxbuffer,1);

	PA7_TIM17_Init();
	//HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
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

	if(PA7_Flag)
	{
		if(Idel>0)
		{
			ucLed=0x03;
		}
		else
		{
			ucLed=0x02;
		}
		
	}
	else
	{
		if(Idel>0)
		{
			ucLed=0x01;
		}
		else
		{
			ucLed=0x00;
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
		if((Interface>>4)==0x01)
		{
			Interface=0x00;
		}
		else
		{
			Interface=0x10;
		}

	}

	if(Key_Down==2)
	{
		CNBR_Price+=0.5;
		VNBR_Price+=0.5;
	}
	
	if(Key_Down==3)
	{
		CNBR_Price-=0.5;
		VNBR_Price-=0.5;
	}

	if(Key_Down==4)
	{
		PA7_Flag^=1;
		if(PA7_Flag)
		{
			HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
		}
		else
		{
			HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
		}	
	}

}


static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;


	if(Check_Cmd(RX_BUF))
	{
		uint8_t year_temp,month_temp,day_temp,hour_temp,min_temp,sec_temp;
		uint8_t car_id[5];
		uint8_t car_type[5];
		uint32_t second_data=0;

		year_temp=(RX_BUF[10]-'0')*10+(RX_BUF[11]-'0');
		month_temp=(RX_BUF[12]-'0')*10+(RX_BUF[13]-'0');
		day_temp=(RX_BUF[14]-'0')*10+(RX_BUF[15]-'0');
		hour_temp=(RX_BUF[16]-'0')*10+(RX_BUF[17]-'0');
		min_temp=(RX_BUF[18]-'0')*10+(RX_BUF[19]-'0');
		sec_temp=(RX_BUF[20]-'0')*10+(RX_BUF[21]-'0');

		if( (year_temp>99)||(month_temp>12)||(day_temp>31)||(hour_temp>23)||(min_temp>59)||(sec_temp>59) )
		{
			goto SEND_ERROR;
		}
		
		substr(car_id,RX_BUF,5,4);//提取车的ID
  		substr(car_type,RX_BUF,0,4);//提取车的类型
		
		// sprintf((char *)Lcd_Disp_String,"    Test:%d",idExist(car_id));
		// LCD_DisplayStringLine(Line9,Lcd_Disp_String);

		if(idExist(car_id)==0xff)//数据库不存在 存入当前数据车辆
		{
			uint8_t locate=FindLocate();
			if(locate==0xff)//看看数据库是否还有空位置 即 看看停车场还有没有空位
			{
				goto SEND_ERROR;//没有空位返回0xff
			}

			//存入当前的车辆信息
			substr(Car_Data_Storage[locate].type,car_type,0,4);
			substr(Car_Data_Storage[locate].id,car_id,0,4);
			Car_Data_Storage[locate].year_in=year_temp;
			Car_Data_Storage[locate].month_in=month_temp;
			Car_Data_Storage[locate].day_in=day_temp;
			Car_Data_Storage[locate].hour_in=hour_temp;
			Car_Data_Storage[locate].min_in=min_temp;
			Car_Data_Storage[locate].sec_in=sec_temp;
			Car_Data_Storage[locate].NotEmpty=1;

			if(Car_Data_Storage[locate].type[0]=='C')
			{
				Cnbr++;
			}
			else if(Car_Data_Storage[locate].type[0]=='V')
			{
				Vnbr++;
			}
			Idel--;
			
		}
		else if(idExist(car_id)!=0xff)//数据库存在 表面当前车辆要出去 
		{
			uint8_t in_locate=idExist(car_id);
			if(strcmp((const char*)Car_Data_Storage[in_locate].type,(const char*)car_type)!=0)
			{
				goto SEND_ERROR;
			}
			second_data=(year_temp-Car_Data_Storage[in_locate].year_in)*365*24*3600+
			(month_temp-Car_Data_Storage[in_locate].month_in)*30*24*3600+
			(day_temp-Car_Data_Storage[in_locate].day_in)*24*3600+
			(hour_temp-Car_Data_Storage[in_locate].hour_in)*3600+
			(min_temp-Car_Data_Storage[in_locate].min_in)*60+
			(sec_temp-Car_Data_Storage[in_locate].sec_in);

			if(second_data<=0)
			{
				goto SEND_ERROR;
			}
			
			second_data=(second_data+3599)/3600;
			sprintf((char *)str_str,"%s:%s:%.2f\r\n",Car_Data_Storage[in_locate].type,Car_Data_Storage[in_locate].id
			,Car_Data_Storage[in_locate].type[0]=='C'?CNBR_Price*second_data:VNBR_Price*second_data);
			HAL_UART_Transmit(&huart1,str_str,strlen((const char*)str_str),50);
			if(Car_Data_Storage[in_locate].type[0]=='C')
			{
				Cnbr--;
			}
			else if(Car_Data_Storage[in_locate].type[0]=='V')
			{
				Vnbr--;
			}
			Idel++;
			memset(&Car_Data_Storage[in_locate],0,sizeof(Car_Data_Storage[in_locate]));
		}



		goto CMD_YES;
		SEND_ERROR:
		sprintf((char*)str_str,"Error\r\n");
		HAL_UART_Transmit(&huart1,str_str,strlen((const char*)str_str),50);
		
		CMD_YES:
		RX_Counter=0;
	}

	if(Interface==0x00)
	{
		sprintf((char *)Lcd_Disp_String,"       Data");
		LCD_DisplayStringLine(Line2,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"    CNBR:%d",Cnbr);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"    VNBR:%d",Vnbr);
		LCD_DisplayStringLine(Line6,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"    IDLE:%d",Idel);
		LCD_DisplayStringLine(Line8,Lcd_Disp_String);
		
		
	}
	else
	{
		sprintf((char *)Lcd_Disp_String,"       Para");
		LCD_DisplayStringLine(Line2,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"   CNBR:%1.2f",CNBR_Price);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);

		sprintf((char *)Lcd_Disp_String,"   VNBR:%1.2f",VNBR_Price);
		LCD_DisplayStringLine(Line6,Lcd_Disp_String);

	}
	

}

uint8_t idExist(uint8_t* str)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(strcmp((const char *)str,(const char *)Car_Data_Storage[i].id)==0)
		{
			return i;
		}
		
	}

return 0xff;
}

static void Uart_Proc(void)
{
	if(uwTick-uwTick_Uart_Set_Point<=1000)return;
	uwTick_Uart_Set_Point=uwTick;
	Count++;
	// sprintf((char *)Lcd_Disp_String,"%dHello Blue Cup\r\n",Count);
	// HAL_UART_Transmit(&huart1,Lcd_Disp_String,strlen((const char*)Lcd_Disp_String),50);

	
}

_Bool Check_Cmd(uint8_t * str)
{
	if(RX_Counter!=22)
	return 0;
	if( (str[0]=='C'||str[0]=='V')&&(str[1]=='N')&&str[2]=='B'&&(str[3]=='R')&&(str[4]==':')&&(str[9]==':') )
	{
		uint8_t i=0;
		for(i=10;i<22;i++)
		{
			if( (str[i]>'9') || (str[i]<'0') )
				return 0;
		}
		return 1;
	}

	return 0;
}

void substr(uint8_t * d_str,uint8_t * s_str,uint8_t loacte,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
	{
		d_str[i]=s_str[loacte+i];
	}
	d_str[len]='\0';
	
}

uint8_t FindLocate(void)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(Car_Data_Storage[i].NotEmpty==0)
		{
			return i;
		}

	}
		return 0xff;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// ucLed=0xf0;
	// HAL_Delay(300);
	//ucLed=0x00;

	RX_BUF[RX_Counter]=rxbuffer;
	RX_Counter++;
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
