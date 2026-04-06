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
__IO uint32_t uwTick_PWM_Effective_Set_Point=0;
__IO uint32_t uwTick_KEY_Long_Set_Point=0;


uint8_t Key_Value=0;
uint8_t Key_Down=0;
uint8_t Key_Up=0;
uint8_t Key_Old=0;

uint8_t Lcd_Disp_String[20]={0};
uint8_t ucLed=0;
uint16_t IC_PA7_Counter=0;
uint16_t PA7_D_Counter;
uint16_t IC_PA7_Freq=0;
double IC_PA7_Duty;
double R37_Voltage;
uint8_t Output_Duty_Hundred=0;

uint8_t IntereFace=0x00;
_Bool PWM_Mode=0;//设置高低频模式
_Bool Effective_Flag=1;
uint16_t Set_Currrent_Freq=4000;
uint16_t PA1_AUTORELOAD=249;
uint16_t PA1_COMAPRE=25;

float Speed;
uint8_t R=1;
uint8_t K=1;

_Bool PARA_UP_DOWN=1;
uint8_t R_Disp=1;
uint8_t K_Disp=1;
_Bool Lock_Duty=0;

uint8_t Mode_Times=0;
_Bool Buling_Flag=0;

float Max_Speed_H = 0.0; // 高频模式下的最大速度
float Max_Speed_L = 0.0; // 低频模式下的最大速度
float Last_Speed = 0.0;  // 记录上一次的速度，用来比对
uint8_t Speed_Stable_Cnt = 0; // 稳定时间计时器


static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void PWM_Effective_Proc(void);
void Set_PA1_PWM(void);
void Set_LOCK_PA1_PWM(void);
void GET_SPEED_MAX(void);

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
	
	PA1_TIM2_Init();
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	
	PA7_TIM3_Init();
	HAL_TIM_Base_Init(&htim3);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
	
while (1)
  {
	  
	Led_Proc();
    Key_Proc();
    Lcd_Proc();
	PWM_Effective_Proc();
	  
  }
  
}



static void Led_Proc(void)
{
	if(uwTick-uwTick_Led_Set_Point<=100)return;
	uwTick_Led_Set_Point=uwTick;
	Buling_Flag^=1;
    if(Lock_Duty)
    {
		if(IntereFace==0x00)
		{
			if(Buling_Flag&&(Effective_Flag==0)	)    //1	1	1
			{
				ucLed=0x07;
			}
			else			//1		0			1
			{
				ucLed=0x05;
			}
		}
		else
		{
			if(Buling_Flag&&(Effective_Flag==0)	)    //1	1	0
			{
				ucLed=0x06;
			}
			else			//1			0			0
			{
				ucLed=0x04;
			}

		}
		
		
    }
    else
    {
		
		if(IntereFace==0x00)
		{
			if(Buling_Flag&&(Effective_Flag==0)	)    //0	1	1
			{
				ucLed=0x03;
			}
			else			//0-		0			1
			{
				ucLed=0x01;
			}
		}
		else
		{
			if(Buling_Flag&&(Effective_Flag==0)	)    //0	1	0
			{
				ucLed=0x02;
			}
			else			//0			0			0
			{
				ucLed=0x00;
			}

		}
		
    }
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
	
	if(Key_Down==1)
	{
		 LCD_Clear(Black);
		if((IntereFace>>4)==0x00)
		{
			IntereFace=0x10;
			PARA_UP_DOWN=1;
		}
		else if((IntereFace>>4)==0x01)
		{
			IntereFace=0x20;
			K=K_Disp;
			R=R_Disp;	
		}
		else if((IntereFace>>4)==0x02)
		{
			IntereFace=0x00;
		}
	}

	
	if(Key_Down==2)
	{
		if((IntereFace>>4)==0x00)
		{
			if(Effective_Flag)
			{
				PWM_Mode^=1;
				++Mode_Times;
				uwTick_PWM_Effective_Set_Point=uwTick;
				Effective_Flag=0;
			}
		}

		if((IntereFace>>4)==0x01)
		{
			PARA_UP_DOWN^=1;
		}

	}
	if(Key_Down==3)
	{
		if(PARA_UP_DOWN)
		{
			if((IntereFace>>4)==0x01)
			{		
				++R_Disp;
				if(R_Disp>10)R_Disp=1;
			}
		}
		else
		{	
			if((IntereFace>>4)==0x01)
			{		
				++K_Disp;
				if(K_Disp>10)K_Disp=1;
			}
		}
	}
	if(Key_Down==4)
	{
		if((IntereFace>>4)==0x01)
		{
			
			if(PARA_UP_DOWN)
			{			
				if((IntereFace>>4)==0x01)
				{		
					--R_Disp;
					if(R_Disp<=0)R_Disp=10;
				}
			}
			else
			{	
				if((IntereFace>>4)==0x01)
				{		
					--K_Disp;
					if(K_Disp<=0)K_Disp=10;
				}

			}
		}

	}	

	// if(Key_Down)
	// {
	// 	uwTick_KEY_Long_Set_Point=uwTick;
	// }

	// if((uwTick-uwTick_KEY_Long_Set_Point)<=2000)
	// {
	// 	if(Key_Up==4)
	// 	{
	// 		if((IntereFace>>4)==0x00)
	// 		{
	// 			Lock_Duty=0;
	// 		}
	// 	}
	// }
	// else
	// {
	// 	if(Key_Value==4)
	// 	{
	// 		if((IntereFace>>4)==0x00)
	// 		{
	// 			Lock_Duty=1;
	// 			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10, GPIO_PIN_RESET);
	// 			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	// 			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	// 		}
	// 	}
	// }






//====== 替换到 Key_Proc 的最尾部 ======
    static uint16_t b4_long_cnt = 0; // 专属的抗抖动长按计时器

    if (Key_Value == 4) // 只要按键还处于被按下的状态 (屏蔽了物理抖动)
    {
        b4_long_cnt++;
        if (b4_long_cnt == 40) // 50ms * 40次 = 刚好 2 秒！
        {
            if((IntereFace>>4)==0x00) // 必须在 DATA 界面
            {
                Lock_Duty = 1; // 🚀 锁定！
               
            }
        }
    } 
    else 
    {
        if (Key_Up == 4) 
        {
            // 如果松手时，没到 2秒（计数器大于0且小于40），说明是短按！
            if (b4_long_cnt > 0 && b4_long_cnt < 40) 
            {
                if((IntereFace>>4)==0x00)
                {
                    Lock_Duty = 0; // 🔓 解锁！
                }
            }
        }
        b4_long_cnt = 0; // 只要手一松开或者按了别的键，计时器立刻清零
    }
    // ======================================



}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;

	if(Lock_Duty==0)
	{
		Set_PA1_PWM();//改变PWM的频率
	}
	else 
	{
		Set_LOCK_PA1_PWM();
	}

	IC_PA7_Freq=1000000/IC_PA7_Counter;//计算PA7检测输入频率
	Speed=(IC_PA7_Freq*6.28*R)/(100*K);//计算速度
	GET_SPEED_MAX();
//	sprintf((char *)Lcd_Disp_String,"%.2f %.2f ",R37_Voltage,IC_PA7_Duty*100);

	if((IntereFace>>4)==0x00)
	{
		sprintf((char *)Lcd_Disp_String,"        DATA");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);

		if(PWM_Mode)
		{
			sprintf((char *)Lcd_Disp_String,"     M=H");
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		}
		else
		{
			sprintf((char *)Lcd_Disp_String,"     M=L");
			LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		}
		sprintf((char *)Lcd_Disp_String,"     P=%d%%",Output_Duty_Hundred);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     V=%.1f",Speed);
		LCD_DisplayStringLine(Line5,Lcd_Disp_String);


	}	
	
	else if((IntereFace>>4)==0x01)
	{	
		sprintf((char *)Lcd_Disp_String,"        PARA");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     R=%d",R_Disp);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     K=%d",K_Disp);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);

	}

	else if((IntereFace>>4)==0x02)
	{
		sprintf((char *)Lcd_Disp_String,"        RECD");
		LCD_DisplayStringLine(Line1,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     N=%d",Mode_Times);
		LCD_DisplayStringLine(Line3,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     MH=%.1f",Max_Speed_H);
		LCD_DisplayStringLine(Line4,Lcd_Disp_String);
		sprintf((char *)Lcd_Disp_String,"     ML=%.1f",Max_Speed_L);
		LCD_DisplayStringLine(Line5,Lcd_Disp_String);

	}

	

}



static void PWM_Effective_Proc(void)
{
	if(uwTick-uwTick_PWM_Effective_Set_Point<=5000)return;
	Effective_Flag=1;

}

void GET_SPEED_MAX(void)
{
	double speed_diff = Speed - Last_Speed;
    if(speed_diff < 0) speed_diff = -speed_diff; // 手动求绝对值

    // 2. 如果速度波动小于 0.5，说明电位器没动，速度是稳定的
    if(speed_diff <= 0.5) 
    {
        Speed_Stable_Cnt++; // 计时器加加
        if(Speed_Stable_Cnt >= 10) // 200ms * 10次 = 刚好 2 秒！
        {
            Speed_Stable_Cnt = 10; // 封顶，防止变量溢出
            
            // 💡 只有稳定了2秒，才配被纳入统计！
            if(PWM_Mode == 1) // M=H (高频模式)
            {
                if(Speed > Max_Speed_H) Max_Speed_H = Speed;
            }
            else // M=L (低频模式)
            {
                if(Speed > Max_Speed_L) Max_Speed_L = Speed;
            }
        }
    }
    else
    {
        // 3. 如果波动大于 0.5，说明你在拧电位器，速度变了！重新开始 2 秒倒计时！
        Last_Speed = Speed;
        Speed_Stable_Cnt = 0; 
    }
}

void Set_PA1_PWM(void)
{
	R37_Voltage=Get_ADCR37()*3.3/4096;
	if(R37_Voltage<=1)
	{
	Output_Duty_Hundred=10;
	}
	else if(R37_Voltage>=3)
	{
	Output_Duty_Hundred=85;
	}
	else
	{
		Output_Duty_Hundred=(uint8_t)(37.5*R37_Voltage-27.5);
	}

	if(PWM_Mode==1)
	{
		Set_Currrent_Freq+=160;
		if(Set_Currrent_Freq>=8000)
		{
			Set_Currrent_Freq=8000;
		}
		PA1_AUTORELOAD=1000000/Set_Currrent_Freq;
		PA1_COMAPRE=(uint16_t)(PA1_AUTORELOAD*Output_Duty_Hundred*0.01);
		__HAL_TIM_SET_AUTORELOAD(&htim2,PA1_AUTORELOAD);
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,PA1_COMAPRE);
		__HAL_TIM_SET_COUNTER(&htim2,0);

	}
	else
	{
		Set_Currrent_Freq-=160;
		if(Set_Currrent_Freq<=4000)
		{
			Set_Currrent_Freq=4000;
		}
		PA1_AUTORELOAD=1000000/Set_Currrent_Freq;
		PA1_COMAPRE=(uint16_t)(PA1_AUTORELOAD*Output_Duty_Hundred*0.01);
		__HAL_TIM_SET_AUTORELOAD(&htim2,PA1_AUTORELOAD);
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,PA1_COMAPRE);
		__HAL_TIM_SET_COUNTER(&htim2,0);
	}

}





void Set_LOCK_PA1_PWM(void)
{
	if(PWM_Mode==1)
	{
		Set_Currrent_Freq+=160;
		if(Set_Currrent_Freq>=8000)
		{
			Set_Currrent_Freq=8000;
		}
		PA1_AUTORELOAD=1000000/Set_Currrent_Freq;
		PA1_COMAPRE=(uint16_t)(PA1_AUTORELOAD*Output_Duty_Hundred*0.01);
		__HAL_TIM_SET_AUTORELOAD(&htim2,PA1_AUTORELOAD);
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,PA1_COMAPRE);
		__HAL_TIM_SET_COUNTER(&htim2,0);

	}
	else
	{
		Set_Currrent_Freq-=160;
		if(Set_Currrent_Freq<=4000)
		{
			Set_Currrent_Freq=4000;
		}
		PA1_AUTORELOAD=1000000/Set_Currrent_Freq;
		PA1_COMAPRE=(uint16_t)(PA1_AUTORELOAD*Output_Duty_Hundred*0.01);
		__HAL_TIM_SET_AUTORELOAD(&htim2,PA1_AUTORELOAD);
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,PA1_COMAPRE);
		__HAL_TIM_SET_COUNTER(&htim2,0);
	}

}






void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM3)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		{
			IC_PA7_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)+1;
			IC_PA7_Duty=(float)PA7_D_Counter/IC_PA7_Counter;
			
		}
		else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PA7_D_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
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
