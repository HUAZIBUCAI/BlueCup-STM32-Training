#include "main.h"
#include "rcc\bsp_rcc.h"
#include "key_led\bsp_key_led.h"
#include "string.h"
#include "stdio.h"
#include "lcd\bsp_lcd.h"
#include "adc\bsp_adc.h"
#include "uart\bsp_uart.h"



uint8_t Key_Value,Key_Up,Key_Down,Key_Old;
uint8_t ucLed=0x00;
uint8_t String_Disp[20];
double R37_Voltage;
double Change_R37;
uint8_t rxbuffer;
uint8_t TX_UART_Buffer[20];
uint8_t TX_Times=0;
uint8_t IntereFace=0x00;
_Bool Mode_Flag=1;

double MIN_Clarm=3.7;
double MAX_Clarm=8.4;
double MIN_Disp=6.4;
double MAX_Disp=8.4;

uint8_t LD1=0;
uint8_t LD2=0;
uint8_t LD8=0;
_Bool LD2_Flag=0;
_Bool LD8_Flag=0;
  


static __IO uint32_t uwTick_Key_Set_Point=0;
static __IO uint32_t uwTick_Lcd_Set_Point=0;
static __IO uint32_t uwTick_Led_Set_Point=0;
static __IO uint32_t uwTick_Uart_Set_Point=0;
static void Key_Proc(void);
static void Led_Proc(void);
static void Lcd_Proc(void);
static void Uart_Proc(void);
void Check_Uart_Cmd(void);




int main(void)
{
  HAL_Init();
	
  SystemClock_Config();
  Key_Led_GPIO_Init();

  LCD_Init();
  LCD_SetTextColor(White);
  LCD_SetBackColor(Black);
  LCD_Clear(Black);
  
  R37_ADC2_Init();

  RX_TX_USART1_UART_Init();
  HAL_UART_Receive_IT(&huart1,&rxbuffer,1);
  while (1)
  {
	Key_Proc();
	Lcd_Proc();
	Led_Proc();
	Uart_Proc();
  
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
    if(IntereFace>>4==0x00)
    {
      IntereFace=0x10;
    }

    else if(IntereFace>>4==0x01)
    {
      MAX_Clarm=MAX_Disp;
      MIN_Clarm=MIN_Disp;
      IntereFace=0x00;
    }

  }

   if(Key_Down==2)
  {
    LCD_Clear(Black);
    if(IntereFace>>4==0x01)
    {
      IntereFace++;
      if(IntereFace==0x12)
      {
        IntereFace=0x10;
      }
    }
    
  }

  if(Key_Down==3)
  {
    LCD_Clear(Black);

    if(IntereFace==0x10)
    {
      MIN_Disp+=0.1;
      if(MIN_Disp+0.1>=MAX_Disp)MIN_Disp=MAX_Disp-0.1;
    }

    else if(IntereFace==0x11)
    {
      MAX_Disp+=0.1;
      if(MAX_Disp>=8.4)MAX_Disp=8.4;
    }
    
  }


  if(Key_Down==4)
  {
    LCD_Clear(Black);  
    if(IntereFace>>4==0x00)
    {
      Mode_Flag^=1;
    }

    if(IntereFace==0x10)
    {
      MIN_Disp-=0.1;
      if(MIN_Disp<=3.7)MIN_Disp=3.7;
    }
    else if(IntereFace==0x11)
    {
      MAX_Disp-=0.1;
      if(MAX_Disp-0.1<=MIN_Disp)MAX_Disp=MIN_Disp+0.1;
    }


  }
  
}


uint8_t Check_Cmd(void)
{
  if(MIN_Clarm<Change_R37&&Change_R37<MAX_Clarm)
  {
      return 1;
  }
return 0;
}

static void Led_Proc(void)
{
	if(uwTick-uwTick_Led_Set_Point<=100)return;
	uwTick_Led_Set_Point=uwTick;

  LD2_Flag^=1;
  LD8_Flag^=1;


  if(Mode_Flag==0)
  {
    LD1=1;
  }
  else
  {
    LD1=0;
  }
  if( Check_Cmd()==0)
  {
    LD2=1;
  }
  else
  {
    LD2=0;
  }
  if(Change_R37>MAX_Clarm)
  {
    LD8=1;
  }
  else
  {
    LD8=0;
  }
  
  if(LD1==1&&LD2==1&&LD8==1)
  {
    if(LD2_Flag)
    {
      if(LD8_Flag)
      {
        ucLed=0x83;
      }
      else
      {
        ucLed=0x03;
      }

    }
    else
    {
      if(LD8_Flag)
      {
        ucLed=0x81;
      }
      else
      {
        ucLed=0x01;
      }

    }

  }
  else if(LD1==1&&LD2==1&&LD8==0)
  {
    if(LD2_Flag)
    {
      ucLed=0x03;
    }
    else
    {
      ucLed=0x01;
    }

  }
    else if(LD1==1&&LD2==0&&LD8==1)
  {

    if(LD8_Flag)
    {
      ucLed=0x81;
    }
    else
    {
      ucLed=0x01;
    }
  }
  
    else if(LD1==0&&LD2==1&&LD8==1)
  {
    if(LD2_Flag)
    {
      if(LD8_Flag)
      {
        ucLed=0x82;
      }
      else
      {
        ucLed=0x02;
      }

    }
    else
    {
      if(LD8_Flag)
      {
        ucLed=0x80;
      }
      else
      {
       ucLed=0x00;
      }

    }

  }
  
    else if(LD1==0&&LD2==0&&LD8==1)
  {
    if(LD8_Flag)
    {
      ucLed=0x80;
    }
    else
    {
      ucLed=0x00;
    }
  }
  
    else if(LD1==0&&LD2==1&&LD8==0)
  {
    if(LD2_Flag)
    {
      ucLed=0x02;
    }
    else
    {
      ucLed=0x00;
    }

  }
    else if(LD1==1&&LD2==0&&LD8==0)
  {
    ucLed=0x01;

  }
  
    else if(LD1==0&&LD2==0&&LD8==0)
  {
    ucLed=0x00;
  }
  

  Led_Disp(ucLed);
}

static void Lcd_Proc(void)
{
	if(uwTick-uwTick_Lcd_Set_Point<=200)return;
	uwTick_Lcd_Set_Point=uwTick;

  R37_Voltage=Get_ADC2_R37()*3.3/4095;
  Change_R37=R37_Voltage*3;


  if(IntereFace>>4==0x00)
  {
      sprintf((char *)String_Disp,"      BATTERY  ");
      LCD_DisplayStringLine(Line1,String_Disp);

      sprintf((char *)String_Disp,"     VOLT:%.1fV  ",Change_R37);
      LCD_DisplayStringLine(Line3,String_Disp);
      if(Mode_Flag)
      {
        sprintf((char *)String_Disp,"    STATE:IDLE    ");
        LCD_DisplayStringLine(Line4,String_Disp);
      }
      else
      {
        sprintf((char *)String_Disp,"    STATE:CHARGE  ");
        LCD_DisplayStringLine(Line4,String_Disp);
      }
        
  }

  else if(IntereFace>>4==0x01)
  {
      sprintf((char *)String_Disp,"        SET    ");
      LCD_DisplayStringLine(Line1,String_Disp);

      sprintf((char *)String_Disp,"      MIN:%.1fV   ",MIN_Disp);
      LCD_DisplayStringLine(Line3,String_Disp);

      sprintf((char *)String_Disp,"      MAX:%.1fV   ",MAX_Disp);
      LCD_DisplayStringLine(Line4,String_Disp);

  }
  // sprintf((char *)String_Disp,"Hello World");
  // LCD_DisplayStringLine(Line0,String_Disp);
  
  // sprintf((char *)String_Disp,"%.1f",R37_Voltage);
  // LCD_DisplayStringLine(Line1,String_Disp);

}
	
static void Uart_Proc(void)
{
	if(uwTick-uwTick_Uart_Set_Point<=100)return;
	uwTick_Uart_Set_Point=uwTick;

 Check_Uart_Cmd();
 


}

void Check_Uart_Cmd(void)
{
  if(TX_Times==10)
  {
    if(TX_UART_Buffer[0]=='C'&&TX_UART_Buffer[1]=='h'&&TX_UART_Buffer[2]=='e'&&TX_UART_Buffer[3]=='c'&&TX_UART_Buffer[4]=='k'&&
    TX_UART_Buffer[5]=='_'&&TX_UART_Buffer[6]=='V'&&TX_UART_Buffer[7]=='O'&&TX_UART_Buffer[8]=='L'&&TX_UART_Buffer[9]=='T')
    {

      sprintf((char *)TX_UART_Buffer,"BATTERY:%.1f\r\n",Change_R37);
      HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
      TX_Times=0;
      memset(TX_UART_Buffer,0,10);
    }
    else
    {
      sprintf((char *)TX_UART_Buffer,"error\r\n");
      HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
      TX_Times=0;
      memset(TX_UART_Buffer,0,10);
    }

  }
  else if(TX_Times==11)
  {
    
    if(TX_UART_Buffer[0]=='C'&&TX_UART_Buffer[1]=='h'&&TX_UART_Buffer[2]=='e'&&TX_UART_Buffer[3]=='c'&&TX_UART_Buffer[4]=='k'&&
    TX_UART_Buffer[5]=='_'&&TX_UART_Buffer[6]=='S'&&TX_UART_Buffer[7]=='T'&&TX_UART_Buffer[8]=='A'&&TX_UART_Buffer[9]=='R'&&TX_UART_Buffer[10]=='T')
    {
       if(Mode_Flag)
      {
        sprintf((char *)TX_UART_Buffer,"START:IDLE\r\n");
        HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
        TX_Times=0;
        memset(TX_UART_Buffer,0,11);
      }
      else
      {
        sprintf((char *)TX_UART_Buffer,"START:CHARGE\r\n");
        HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
        TX_Times=0;
        memset(TX_UART_Buffer,0,11);
      }
      
    }
    else
    {
      sprintf((char *)TX_UART_Buffer,"error\r\n");
      HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
      TX_Times=0;
      memset(TX_UART_Buffer,0,11);
    }


  }
  else if(TX_Times!=0&&TX_Times!=10&&TX_Times!=11)
  {
    sprintf((char *)TX_UART_Buffer,"error\r\n");
    HAL_UART_Transmit(&huart1,TX_UART_Buffer,strlen((const char *)TX_UART_Buffer),50);
    TX_Times=0;
    memset(TX_UART_Buffer,0,strlen((const char *)TX_UART_Buffer));
    }

  }





  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
  {
    // Led_Disp(0xff);
    // HAL_Delay(300);
    // Led_Disp(0x00);
    TX_UART_Buffer[TX_Times++]=rxbuffer;
    HAL_UART_Receive_IT(&huart1,&rxbuffer,1);


  }

/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
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
