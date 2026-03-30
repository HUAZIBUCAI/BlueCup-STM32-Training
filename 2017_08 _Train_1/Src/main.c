#include "main.h"
#include "rcc\bsp_rcc.h"
#include "rtc\bsp_rtc.h"
#include "key_led\bsp_key_led.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"

__IO uint32_t uwTick_KEY_SET_Point=0;
__IO uint32_t uwTick_LED_SET_Point=0;
__IO uint32_t uwTick_LCD_SET_Point=0;
__IO uint32_t uwTick_Clock_SET_Point=0;
_Bool PA4_Vlotage=0;
_Bool PA5_Vlotage=0;

uint8_t Key_Value=0;
uint8_t Key_Old=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;

uint8_t ucLed=0x00;

uint8_t Lcd_Disp_str[20];

RTC_TimeTypeDef	H_M_S;
RTC_DateTypeDef Y_M_D;


uint8_t ucSet;//LED低四位
uint8_t ucPlat=1;//当前平台
uint8_t ucState=0;//当前状态
uint8_t dir=0;//方向 0电梯没有动 1上行 2下行
uint8_t Flow=0x80;

static void Key_Proc(void);
static void Led_Proc(void);
static void Lcd_Proc(void);
static void Elev_Proc(void);

int main(void)
{
	
	HAL_Init();

	SystemClock_Config();

	Clock_RTC_Init();
	Key_Lcd_GPIO_Init();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	LCD_Init();
	LCD_Clear(White);
	LCD_SetTextColor(Black);
	LCD_SetBackColor(White);
	
  while (1)
  {
	  Key_Proc();
	  Led_Proc();
	  Lcd_Proc();
	  Elev_Proc();
	 
  }
 
}


static void Key_Proc(void)
{
	if(uwTick-uwTick_KEY_SET_Point<=50)return;
	uwTick_KEY_SET_Point=uwTick;
	
	Key_Value=Key_Scan();
	Key_Down=Key_Value&(Key_Value^Key_Up);
	Key_Up=~Key_Value&(Key_Value^Key_Up);
	Key_Old=Key_Value;

	if(ucState==0)//状态0 有按键按下 点亮对应的LED 如果有LED被点亮说明有目标层
	{
		if(Key_Value==1){if(ucPlat!=1)ucSet|=0x01;}
		else if(Key_Value==2){if(ucPlat!=2)ucSet|=0x02;}
		else if(Key_Value==3){if(ucPlat!=3)ucSet|=0x04;}	
		else if(Key_Value==4){if(ucPlat!=4)ucSet|=0x08;}
		ucLed&=0xf0;
		ucLed|=ucSet;
	}
	if(Key_Down!=0)
	{
		uwTick_Clock_SET_Point=uwTick;


	}


}
static void Led_Proc(void)
{
	if(uwTick-uwTick_LED_SET_Point<=200)return;
	uwTick_LED_SET_Point=uwTick;
	Led_Disp(ucLed);

}
static void Lcd_Proc(void)
{
	if(uwTick-uwTick_LCD_SET_Point<=200)return;
	uwTick_LCD_SET_Point=uwTick;
	

	sprintf((char*)Lcd_Disp_str," Current Platform");
	LCD_DisplayStringLine(Line1,Lcd_Disp_str);	
	
	sprintf((char*)Lcd_Disp_str,"    %1d",ucPlat);
	LCD_DisplayStringLine(Line3,Lcd_Disp_str);	

	HAL_RTC_GetDate(&hrtc,&Y_M_D,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc,&H_M_S,RTC_FORMAT_BIN);
	sprintf((char*)Lcd_Disp_str,"    %02d-%02d-%02d",H_M_S.Hours,H_M_S.Minutes,H_M_S.Seconds);
	LCD_DisplayStringLine(Line5,Lcd_Disp_str);

	

//	sprintf((char*)Lcd_Disp_str,"PA4:%1d PA5:%1d",PA4_Vlotage,PA5_Vlotage);
//	LCD_DisplayStringLine(Line9,Lcd_Disp_str);	



}

static void Elev_Proc(void)
{
	if(ucSet!=0)//有目标层 则执行程序
	{
		switch(ucState)
		{
			case 0://有按键按下 且最后一次按键按下 超过1S 进入状态1 否则 原地等待
				if(uwTick-uwTick_Clock_SET_Point>=1000)
				ucState=1;
			else
				break;
	
			case 1://进入状态1 电梯门关闭 PA7 PWM 2KH D=50%		P5低电平  PA5= 0
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
				PA5_Vlotage=0;
				PA7_TIM17_Init();
				__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,250);
				HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
				sprintf((char*)Lcd_Disp_str,"Door Closing   ");
				LCD_DisplayStringLine(Line9,Lcd_Disp_str);	
				uwTick_Clock_SET_Point=uwTick;
				ucState=2;
			break;
			case 2:
				if(uwTick-uwTick_Clock_SET_Point>=4000)
				{
					HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
					ucState=3;
				}
				else
			break;
			case 3:
				if(ucSet>(1<<ucPlat-1))//上行 PA6 1KHz 80% PA4高电平
				{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
					PA4_Vlotage=1;
					PA6_TIM3_Init();
					__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,800);
					HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
					sprintf((char*)Lcd_Disp_str,"Elve Uping      ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);
					dir=1;
				}
				else if(ucSet<(1<<ucPlat-1))//下行
				{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
					PA4_Vlotage=0;
					PA6_TIM3_Init();
					__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,600);
					HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
					sprintf((char*)Lcd_Disp_str,"Elve Downing     ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);
					dir=2;	
				}
				uwTick_Clock_SET_Point=uwTick;
				ucState=4;

			break;
			case 4:
				if(uwTick-uwTick_Clock_SET_Point>=6000)//如果大于6秒则当前平台加1或者减1
				{
					ucLed&=0x0F;
					if(dir==1)
					{
						ucPlat++;
						ucState=5;
					}
					else if(dir==2)
					{
						ucPlat--;
						ucState=5;
					}
						
					sprintf((char*)Lcd_Disp_str,"    %1d",ucPlat);
					LCD_DisplayStringLine(Line3,Lcd_Disp_str);
					sprintf((char*)Lcd_Disp_str,"Elve Running      ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);
				}
				else
				{
					if(dir==1)//上行流水从左到右
					{
						ucLed&=0x0F;
						if(Flow==0x08)Flow=0x80;
						ucLed|=Flow;
						Flow=Flow>>1;

					}
					else if(dir==2)//下行流水从右到左
					{
						ucLed&=0x0F;
						Flow=Flow<<1;
						if(Flow==0x00)Flow=0x10;
						ucLed|=Flow;
					}
					HAL_Delay(200);
				}
			break;
				case 5:
				if((1<<(ucPlat-1))&ucSet)//如果为1说明已经到达目标层
				{
					HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);//电机停止运转
					HAL_Delay(300);
					sprintf((char*)Lcd_Disp_str,"      ");
					LCD_DisplayStringLine(Line3,Lcd_Disp_str);
					HAL_Delay(300);
					sprintf((char*)Lcd_Disp_str,"    %1d",ucPlat);
					LCD_DisplayStringLine(Line3,Lcd_Disp_str);//闪烁两次当前楼层数字
					
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);//电梯开门 PA5低电平
					PA5_Vlotage=1;
					PA7_TIM17_Init();//控制门电机开门 60%占空比
					__HAL_TIM_SET_COMPARE(&htim17,TIM_CHANNEL_1,300);
					HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
					sprintf((char*)Lcd_Disp_str,"Door Opening   ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);	
					uwTick_Clock_SET_Point=uwTick;
					ucState=6;

				}
				else//没有到达目标层
				{
					uwTick_Clock_SET_Point=uwTick;
					ucState=4;
				}
			break;
				case 6://到达目标层 开门大于四秒后 开门电机停止运转 
				if(uwTick-uwTick_Clock_SET_Point>=4000)
				{
					HAL_TIM_PWM_Stop(&htim17,TIM_CHANNEL_1);
					ucSet&=(~(1<<(ucPlat-1)));
					ucLed&=0xF0;
					ucLed&=ucSet;
					Led_Disp(ucLed);
					ucState=7;
					sprintf((char*)Lcd_Disp_str,"Door Openned  ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);
				}
				else
			break;
			case 7:
				if(ucSet)//检测是否还有别的层
				{
					uwTick_Clock_SET_Point=uwTick;
					sprintf((char*)Lcd_Disp_str,"Waitting 2s   ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);
					ucState=8;
				}
				else
				{
					ucState=0;
					sprintf((char*)Lcd_Disp_str,"             ");
					LCD_DisplayStringLine(Line9,Lcd_Disp_str);	
			break;
				}

			case 8:
			if(uwTick-uwTick_Clock_SET_Point>=2000)//开门后的两秒到达
			{
				sprintf((char*)Lcd_Disp_str,"           ");
				LCD_DisplayStringLine(Line9,Lcd_Disp_str);
				ucState=1;
			}
			break;
		}
	}



}
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
void Error_Handler(void)
{
  
  __disable_irq();
  while (1)
  {
  }
  
}
