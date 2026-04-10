#include "main.h"
#include "key_led\bsp_key_led.h"
#include "rcc\bsp_rcc.h"
#include "adc\bsp_adc.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "tim\bsp_tim.h"
#include "uart\bsp_uart.h"

static __IO uint32_t uwTick_Led_Set_Point=0; 
static __IO uint32_t uwTick_Key_Set_Point=0; 
static __IO uint32_t uwTick_Lcd_Set_Point=0; 
static __IO uint32_t uwTick_Uart_Set_Point=0; 

uint8_t Key_Value,Key_Old,Key_Up,Key_Down;
uint8_t ucLed=0x00;

uint8_t String_Disp[20]={0};
uint8_t rxbuffer;
uint8_t IntereFace=0x00;
_Bool Mode_Flag=0;
double Tempature;
double R37_Voltage;
int16_t Speed=200;
uint8_t Duty=20;
uint8_t TH_Ref=30;
uint8_t RX_Buffer[20];
uint8_t Uart_Times=0;
uint8_t IntereFace_LD1_LD2=0;
uint8_t LD3=0;
uint8_t LD8=0;
uint8_t Buling_LD8=0;

static void Key_Proc(void);
static void Lcd_Proc(void);
static void Led_Proc(void);
static void Uart_Proc(void);
void Check_CMD(void);


int main(void)
{

  
  HAL_Init();

  
  SystemClock_Config();

  Key_Led_GPIO_Init();

  LCD_SetTextColor(White);
  LCD_Init();
  LCD_SetBackColor(Black);
  LCD_Clear(Black);

  R37_ADC2_Init();

  PA7_TIM3_Init();
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);

  TX_RX_USART1_UART_Init();
  HAL_UART_Receive_IT(&huart1,&rxbuffer,1);

  while (1)
  {
    Key_Proc();
    Lcd_Proc();
    Led_Proc();
    Uart_Proc();

  }
  
}


static void Led_Proc(void)
{
  if(uwTick-uwTick_Led_Set_Point<=100)return;
  uwTick_Led_Set_Point=uwTick;
  
  if(IntereFace>>4==0x01)
  {
    IntereFace_LD1_LD2=1;//2亮 
  }
  else
  {
    IntereFace_LD1_LD2=0;//1亮
  }

  if(Mode_Flag==1)
  {
    LD3=1;//3亮
  }
  else
  {
    LD3=0;
  }
  if(Speed>800)
  {
    LD8=1;
  }
  else
  {
    LD8=0;
  }
  
  Buling_LD8^=1;

    if( (LD8==0)&&(LD3==0)&&(IntereFace_LD1_LD2==0) )ucLed=0x01;
    else if( (LD8==0)&&(LD3==0)&&(IntereFace_LD1_LD2==1) )ucLed=0x02;
    else if( (LD8==0)&&(LD3==1)&&(IntereFace_LD1_LD2==0) )ucLed=0x05;
    else if( (LD8==0)&&(LD3==1)&&(IntereFace_LD1_LD2==1) )ucLed=0x06;
    else if(LD8==1)
    {
      if((Buling_LD8==0)&&(LD3==0)&&(IntereFace_LD1_LD2==0) )ucLed=0x01;
      else if((Buling_LD8==1)&&(LD3==0)&&(IntereFace_LD1_LD2==0) )ucLed=0x81;
      else if((Buling_LD8==0)&&(LD3==0)&&(IntereFace_LD1_LD2==1) )ucLed=0x02;
      else if((Buling_LD8==1)&&(LD3==0)&&(IntereFace_LD1_LD2==1) )ucLed=0x82;
      else if((Buling_LD8==0)&&(LD3==1)&&(IntereFace_LD1_LD2==0) )ucLed=0x05;
      else if((Buling_LD8==1)&&(LD3==1)&&(IntereFace_LD1_LD2==0) )ucLed=0x85;      
      else if((Buling_LD8==0)&&(LD3==1)&&(IntereFace_LD1_LD2==1) )ucLed=0x06;
      else if((Buling_LD8==1)&&(LD3==1)&&(IntereFace_LD1_LD2==1) )ucLed=0x86;
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
    if(IntereFace>>4==0x00)
    {
      Mode_Flag^=1;
    }
  }

  
  if(Key_Down==2)
  {
    LCD_Clear(Black);

    
      if(IntereFace>>4==0x00&&Mode_Flag==0)
      {
        Speed+=100;
        if(Speed>=1000)
        Speed=1000;
     }
     else if(IntereFace>>4==0x01)
     {
       TH_Ref+=5;
       if(TH_Ref>=60)
       TH_Ref=60;
     }
    
  }


  if(Key_Down==3)
  {
    LCD_Clear(Black);

      if(IntereFace>>4==0x00&&Mode_Flag==0)
    {
      Speed-=100;
      if(Speed<=100)
      Speed=100;
    }
    else if(IntereFace>>4==0x01)
    {
      TH_Ref-=5;
      if(TH_Ref<=30)
      TH_Ref=30;
    }

    
  }


  if(Key_Down==4)
  {
    LCD_Clear(Black);
    if(IntereFace>>4==0x00)
    {
      IntereFace=0x10;
    }
    else if(IntereFace>>4==0x01)
    {
      IntereFace=0x00;
    }
  }
}

static void Lcd_Proc(void)
{
  if(uwTick-uwTick_Lcd_Set_Point<=100)return;
  uwTick_Lcd_Set_Point=uwTick;

  R37_Voltage=R37_Get_ADC2()*3.3/4096;
  if(0.3<=R37_Voltage&&R37_Voltage<=3.0)
  {
    Tempature=R37_Voltage*800/27-80.0/9;
  }

  else if(R37_Voltage<0.3)
  {
    Tempature=0.0;
  }

  else if(R37_Voltage>3)
  {
    Tempature=80.0;
  }
  
  if(Mode_Flag==1)
  {
    Speed=( (uint16_t)Tempature-TH_Ref)*20;
    if(Speed<=100)Speed=100;
     
  }
   
    Duty=Speed/10;
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,Duty*10);

     if(IntereFace>>4==0x00)
    {
       sprintf((char *)String_Disp,"        DATA");
       LCD_DisplayStringLine(Line1,String_Disp);

       if(Mode_Flag==0)
       {
          sprintf((char *)String_Disp,"      MODE:MANU");
          LCD_DisplayStringLine(Line3,String_Disp);
       }
       else
       {
          sprintf((char *)String_Disp,"      MODE:AUTO");
          LCD_DisplayStringLine(Line3,String_Disp);
       }

         sprintf((char *)String_Disp,"      TEMP:%.1f   ",Tempature);
         LCD_DisplayStringLine(Line4,String_Disp);
         sprintf((char *)String_Disp,"      SPEED:%d    ",Speed);
         LCD_DisplayStringLine(Line5,String_Disp);
    }
    else if(IntereFace>>4==0x01)
    {
        sprintf((char *)String_Disp,"        SET");
        LCD_DisplayStringLine(Line1,String_Disp);

        sprintf((char *)String_Disp,"      TH:%d    ",TH_Ref);
        LCD_DisplayStringLine(Line3,String_Disp);
    }
  // sprintf((char *)String_Disp,"Hello World");
  // LCD_DisplayStringLine(Line0,String_Disp);
  
  // sprintf((char *)String_Disp,"%.1f",R37_Get_ADC2()*3.3/4096);
  // LCD_DisplayStringLine(Line1,String_Disp);

}


static void Uart_Proc(void)
{
  if(uwTick-uwTick_Uart_Set_Point<=100)return;
  uwTick_Uart_Set_Point=uwTick;
  
  Check_CMD();
  // sprintf((char *)String_Disp,"Hello World\r\n");
  // HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50);

}

void Check_CMD(void)
{
  if(Uart_Times==1)
  { 
    if(rxbuffer=='M')
    {
      if(Mode_Flag==0)
      {
         sprintf((char *)String_Disp,"Mode:MANU\r\n");
         HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50);
      }
      else
      {
        sprintf((char *)String_Disp,"Mode:AUTO\r\n");
        HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50);
      }
      memset(RX_Buffer,0,strlen((const char *)RX_Buffer));
      Uart_Times=0;

   }
    else if(rxbuffer=='T')
    {
      sprintf((char *)String_Disp,"Temp:%.1f\r\n",Tempature);
      HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50); 

      memset(RX_Buffer,0,strlen((const char *)RX_Buffer));
      Uart_Times=0;
    }
    else if(rxbuffer=='S')
    {
      sprintf((char *)String_Disp,"Speed:%drpm\r\n",Speed);
      HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50); 

      memset(RX_Buffer,0,strlen((const char *)RX_Buffer));
      Uart_Times=0;
    }
    else
    {
      sprintf((char *)String_Disp,"ERROR\r\n");
      HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50); 
      
      memset(RX_Buffer,0,strlen((const char *)RX_Buffer));
      Uart_Times=0;
    }
  }
  else if(Uart_Times>1)
  {
      sprintf((char *)String_Disp,"ERROR\r\n");
      HAL_UART_Transmit(&huart1,(const uint8_t *)String_Disp,strlen((const char*)String_Disp),50); 
      
      memset(RX_Buffer,0,strlen((const char *)RX_Buffer));
      Uart_Times=0;

  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  // Led_Disp(0xff);
  // HAL_Delay(300);
  // Led_Disp(0x00);

  RX_Buffer[Uart_Times++]=rxbuffer;
  HAL_UART_Receive_IT(&huart1,&rxbuffer,1);

}
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/

 void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

