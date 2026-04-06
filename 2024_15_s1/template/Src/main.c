#include "main.h"
#include "rcc\bsp_rcc.h"
#include "key_led\bsp_key_led.h"
#include "tim\bsp_tim.h"
#include "stdio.h"
#include "string.h"
#include "lcd\bsp_lcd.h"
static __IO uint32_t uwTick_Led_Set_Point=0; 
static __IO uint32_t uwTick_Lcd_Set_Point=0;
static __IO uint32_t uwTick_Key_Set_Point=0;
static __IO uint32_t uwTick_Long_Set_Point=0;

uint8_t Key_Value,Key_Down,Key_Up,Key_Old;

uint8_t ucLed=0x00;
uint8_t IntereFace=0x00;
uint8_t String_Disp[20];

uint16_t PB4_T_Counter=0;
uint16_t PA15_T_Counter=0;

uint16_t PB4_D_Counter=0;
uint16_t PA15_D_Counter=0;

uint16_t PB4_Freq=0;
uint16_t PA15_Freq=0;
double PB4_Freq_Disp=0;
double PA15_Freq_Disp=0;
double PB4_T_Disp=0;
double PA15_T_Disp=0;


double PB4_Duty;
double PA15_Duty;

_Bool Data_Flag=0;

uint16_t PD_Disp=1000;
uint16_t PH_Disp=5000;
int16_t PX_Disp=0;

uint16_t NDA_Disp=0;
uint16_t NDB_Disp=0;
uint16_t NHA_Disp=0;
uint16_t NHB_Disp=0;

int16_t PA15_Caibrate=0;
int16_t PB4_Caibrate=0;

uint16_t PA15_NH_Old=0;
uint16_t PA15_NH_Current=0;
uint16_t PB4_NH_Old=0;
uint16_t PB4_NH_Current=0;
uint16_t PA15_Freq_Bag[30];
uint8_t PA15_Freq_times=0;
uint16_t PB4_Freq_Bag[30];
uint8_t PB4_Freq_times=0;
uint16_t PA15_Freq_Max=0;
uint16_t PA15_Freq_Min=0;
uint16_t PB4_Freq_Max=0;
uint16_t PB4_Freq_Min=0;

_Bool LED_8=0;
_Bool LED_3=0;
_Bool LED_2=0;
_Bool LED_1=0;

static void Key_Proc(void);
static void Led_Proc(void);
static void Lcd_Proc(void);
void Freq_Caibrate(void);
void Frequency_limit_exceeded(void);
void Frequency_jump(void);

int main(void)
{
  HAL_Init();

  SystemClock_Config();
	
  Key_Led_GPIO_Init();
 
  LCD_Init();
  LCD_SetBackColor(Black);
  LCD_SetTextColor(White);
  LCD_Clear(Black);

  
  PA15_TIM2_Init();
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);

  PB4_TIM3_Init();
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);

  while (1)
  {
    Key_Proc();
    Led_Proc();
    Lcd_Proc(); 
  }
  
}


static void Key_Proc(void)
{
  
  if(uwTick-uwTick_Key_Set_Point<=50)return;
  uwTick_Key_Set_Point=uwTick;

  Key_Value=Key_Scan();
  Key_Down=Key_Value&(Key_Old^Key_Value);
  Key_Up=~Key_Value&(Key_Old^Key_Value);
  Key_Old=Key_Value;
  

  if(Key_Down==1)
  {
    if(IntereFace==0x10)
    {
      PD_Disp+=100;
      if(PD_Disp>=1000)PD_Disp=1000;
    }
    else if(IntereFace==0x11)
    {
      PH_Disp+=100;
      if(PH_Disp>=10000)PH_Disp=10000;
    }
    else if(IntereFace==0x12)
    {
      PX_Disp+=100;
      if(PX_Disp>=1000)PX_Disp=1000;
    }

  }
  
  if(Key_Down==2)
  {
    if(IntereFace==0x10)
    {
      PD_Disp-=100;
      if(PD_Disp<=100)PD_Disp=100;
    }
    else if(IntereFace==0x11)
    {
      PH_Disp-=100;
      if(PH_Disp<=1000)PH_Disp=1000;
    }
    else if(IntereFace==0x12)
    {
      PX_Disp-=100;
      if(PX_Disp<=-1000)PX_Disp=-1000;
    }

  }



  if(Key_Down==3)
  {
    LCD_Clear(Black);
    if(IntereFace>>4==0x00)
    {
       Data_Flag^=1;
    }
    else if(IntereFace>>4==0x01)
    {
      IntereFace++;
      if(IntereFace>=0x13)IntereFace=0x00;

     
    }
    else if(IntereFace>>4==0x02)
    {
     uwTick_Long_Set_Point=uwTick;
    }

  }

  if(Key_Value==3&&(uwTick-uwTick_Long_Set_Point>1000))
  {
     NDA_Disp=0;
     NDB_Disp=0;
     NHA_Disp=0;
     NHB_Disp=0;
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
      IntereFace=0x20;
    }
    else if(IntereFace>>4==0x02)
    {
      IntereFace=0x00;
      Data_Flag=0;
    }
  }

}


static void Led_Proc(void)
{
  if(uwTick-uwTick_Led_Set_Point<=200)return;
  uwTick_Led_Set_Point=uwTick;
  
  Led_disp(ucLed);

  if(IntereFace>>4==0x00)
  {
      LED_1=1;
  }
  else
  {
    LED_1=0;
  }
  if(PA15_Caibrate>PH_Disp)
  {
    LED_2=1;
  }
  
  else
  {
    LED_2=0;
  }
  if(PB4_Caibrate>PH_Disp)
  {
    LED_3=1;
  }
    else
  {
    LED_3=0;
  }
  if((NDA_Disp>=3)||(NDB_Disp>=3 ))
  {
    LED_8=1;
  }
  else
  {
    LED_8=0;
  }

  if( (LED_8==1)&&(LED_3==1)&&(LED_2==1)&&(LED_1==1) )ucLed=0x87;
  else if( (LED_8==0)&&(LED_3==1)&&(LED_2==1)&&(LED_1==1) )ucLed=0x07;
  else if( (LED_8==1)&&(LED_3==0)&&(LED_2==1)&&(LED_1==1) )ucLed=0x83;
  else if( (LED_8==1)&&(LED_3==1)&&(LED_2==0)&&(LED_1==1) )ucLed=0x85;
  else if( (LED_8==1)&&(LED_3==1)&&(LED_2==1)&&(LED_1==0) )ucLed=0x86;
  else if( (LED_8==0)&&(LED_3==0)&&(LED_2==1)&&(LED_1==1) )ucLed=0x03;
  else if( (LED_8==0)&&(LED_3==1)&&(LED_2==0)&&(LED_1==1) )ucLed=0x05;
  else if( (LED_8==0)&&(LED_3==1)&&(LED_2==1)&&(LED_1==0) )ucLed=0x06;
  else if( (LED_8==1)&&(LED_3==0)&&(LED_2==0)&&(LED_1==1) )ucLed=0x81;
  else if( (LED_8==1)&&(LED_3==0)&&(LED_2==1)&&(LED_1==0) )ucLed=0x82;
  else if( (LED_8==1)&&(LED_3==1)&&(LED_2==0)&&(LED_1==0) )ucLed=0x84;
  else if( (LED_8==0)&&(LED_3==0)&&(LED_2==0)&&(LED_1==1) )ucLed=0x01;
  else if( (LED_8==0)&&(LED_3==0)&&(LED_2==1)&&(LED_1==0) )ucLed=0x02;
  else if( (LED_8==0)&&(LED_3==1)&&(LED_2==0)&&(LED_1==0) )ucLed=0x04;
  else if( (LED_8==1)&&(LED_3==0)&&(LED_2==0)&&(LED_1==0) )ucLed=0x80;
  else if( (LED_8==0)&&(LED_3==0)&&(LED_2==0)&&(LED_1==0) )ucLed=0x00;

}




static void Lcd_Proc(void)
{
  
  if(uwTick-uwTick_Lcd_Set_Point<=100)return;
  uwTick_Lcd_Set_Point=uwTick;
  Frequency_limit_exceeded();
  Frequency_jump();
  // sprintf((char *)String_Disp,"Hello World  %02x",IntereFace);
  // LCD_DisplayStringLine(Line0,String_Disp);
  // sprintf((char *)String_Disp,"%d  %.1f",PA15_Freq,PA15_Duty);
  // LCD_DisplayStringLine(Line1,String_Disp);
  // sprintf((char *)String_Disp,"%d  %.1f",PB4_Freq,PB4_Duty);
  // LCD_DisplayStringLine(Line2,String_Disp);

   if(IntereFace>>4==0x00)
    {
      Freq_Caibrate();
      if(Data_Flag==0)
      {
         sprintf((char *)String_Disp,"        DATA");
         LCD_DisplayStringLine(Line1,String_Disp);
         if(PA15_Caibrate>=0)
         {
          
           if(PA15_Freq>1000)
           {
             PA15_Freq_Disp=(double)PA15_Freq/1000;
             sprintf((char *)String_Disp,"     A=%.2fKHz      ",PA15_Freq_Disp);
             LCD_DisplayStringLine(Line3,String_Disp);
           }
           else
           {
             sprintf((char *)String_Disp,"     A=%dHz     ",PA15_Freq);
             LCD_DisplayStringLine(Line3,String_Disp);
           }
        
         }
         else
         {
             sprintf((char *)String_Disp,"     A=NULL     ");
             LCD_DisplayStringLine(Line3,String_Disp);
         }

          if(PB4_Caibrate>=0)
         {
            if(PB4_Freq>1000)
          {
             PB4_Freq_Disp=(double)PB4_Freq/1000;
             sprintf((char *)String_Disp,"     B=%.2fKHz    ",PB4_Freq_Disp);
             LCD_DisplayStringLine(Line4,String_Disp);
           }
           else
           {
             sprintf((char *)String_Disp,"     B=%dHz     ",PB4_Freq);
             LCD_DisplayStringLine(Line4,String_Disp);
           }
         }
         else
         {
             sprintf((char *)String_Disp,"     B=NULL     ");
             LCD_DisplayStringLine(Line4,String_Disp);
         }
         
      }
      else
      {

         sprintf((char *)String_Disp,"        DATA");
         LCD_DisplayStringLine(Line1,String_Disp);

         if(PA15_Caibrate>=0)
         {
             if(PA15_T_Counter>1000)
             {
               PA15_T_Disp=(double)PA15_T_Counter/1000;
               sprintf((char *)String_Disp,"     A=%.2fmS     ",PA15_T_Disp);
               LCD_DisplayStringLine(Line3,String_Disp);
              }
              else
             {
               sprintf((char *)String_Disp,"     A=%duS     ",PA15_T_Counter);
               LCD_DisplayStringLine(Line3,String_Disp);
             }
         }

         else
         {
            sprintf((char *)String_Disp,"     A=NULL      ");
            LCD_DisplayStringLine(Line3,String_Disp);
         }


          if(PB4_Caibrate>=0)
          {
             if(PB4_T_Counter>1000)
            {
                PB4_Freq_Disp=(double)PB4_T_Counter/1000;
                sprintf((char *)String_Disp,"     B=%.2fmS      ",PB4_Freq_Disp);
                LCD_DisplayStringLine(Line4,String_Disp);
            }
            else
            {
                sprintf((char *)String_Disp,"     B=%duS      ",PB4_T_Counter);
                LCD_DisplayStringLine(Line4,String_Disp);
            }
          }
          else
          {
            sprintf((char *)String_Disp,"     B=NULL      ");
            LCD_DisplayStringLine(Line4,String_Disp);
          }
      }
      
    }
    else if(IntereFace>>4==0x01)
    {
       sprintf((char *)String_Disp,"        PARA");
       LCD_DisplayStringLine(Line1,String_Disp);
       
       sprintf((char *)String_Disp,"     PD=%dHz",PD_Disp);
       LCD_DisplayStringLine(Line3,String_Disp);

       sprintf((char *)String_Disp,"     PH=%dHz",PH_Disp);
       LCD_DisplayStringLine(Line4,String_Disp);

       sprintf((char *)String_Disp,"     PX=%dHz",PX_Disp);
       LCD_DisplayStringLine(Line5,String_Disp);
     
    }
    else if(IntereFace>>4==0x02)
    {
       sprintf((char *)String_Disp,"        RECD");
       LCD_DisplayStringLine(Line1,String_Disp);

       sprintf((char *)String_Disp,"     NDA=%d",NDA_Disp);
       LCD_DisplayStringLine(Line3,String_Disp);

       sprintf((char *)String_Disp,"     NDB=%d",NDB_Disp);
       LCD_DisplayStringLine(Line4,String_Disp);

       sprintf((char *)String_Disp,"     NHA=%d",NHA_Disp);
       LCD_DisplayStringLine(Line5,String_Disp);

       sprintf((char *)String_Disp,"     NHB=%d",NHB_Disp);
       LCD_DisplayStringLine(Line6,String_Disp);

    }
    

  }
  
  
  void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
  {
    if(htim->Instance==TIM2)
    {
      if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
      {
        PA15_T_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
        PA15_Duty=(double)PA15_D_Counter/PA15_T_Counter*100;
        PA15_Freq=(uint16_t)(1000000/PA15_T_Counter);
      }
      else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
      {
        PA15_D_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2); 
      }
    }
    
    if(htim->Instance==TIM3)
    {
      if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
      {
        PB4_T_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1)+1;
        PB4_Duty=(double)PB4_D_Counter/PB4_T_Counter*100;
        PB4_Freq=(uint16_t)(1000000/PB4_T_Counter);
      }
      else if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
      {
        PB4_D_Counter=HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2); 
      }
  }

}


void Freq_Caibrate(void)
{
  PA15_Caibrate=PX_Disp+PA15_Freq;
  PB4_Caibrate=PX_Disp+PB4_Freq;
}

void Frequency_limit_exceeded(void)//有一个小问题如果频率一开始就超限就会计算超限的次数不对
{
  PA15_NH_Current=PA15_Freq;
  if( (PA15_NH_Current>PH_Disp)&&(PH_Disp>PA15_NH_Old) )NHA_Disp+=1;
  PA15_NH_Old=PA15_NH_Current;

  PB4_NH_Current=PB4_Freq;
  if( (PB4_NH_Current>PH_Disp)&&(PH_Disp>PB4_NH_Old) )NHB_Disp+=1;
  PB4_NH_Old=PB4_NH_Current;


}



void Frequency_jump(void)
{
if(uwTick < 1000) return;
  static _Bool first_run = 1;
  if(first_run)
  {
      for(int j=0; j<30; j++)
      {
          PA15_Freq_Bag[j] = PA15_Freq;
          PB4_Freq_Bag[j]  = PB4_Freq;
      }
      first_run = 0;
  }

  PA15_Freq_Bag[PA15_Freq_times++]=PA15_Freq;
  if(PA15_Freq_times>29)PA15_Freq_times=0;
  
  PB4_Freq_Bag[PB4_Freq_times++]=PB4_Freq;
  if(PB4_Freq_times>29)PB4_Freq_times=0;

   PA15_Freq_Max=PA15_Freq_Bag[0];
   PA15_Freq_Min=PA15_Freq_Bag[0];
   PB4_Freq_Max=PB4_Freq_Bag[0];
   PB4_Freq_Min=PB4_Freq_Bag[0];
 
  uint8_t i;
  for(i=0;i<30;i++)
  {
    if(PA15_Freq_Max<=PA15_Freq_Bag[i])
    PA15_Freq_Max=PA15_Freq_Bag[i];
      
    if(PA15_Freq_Min>=PA15_Freq_Bag[i])
    PA15_Freq_Min=PA15_Freq_Bag[i];

    if(PB4_Freq_Max<=PB4_Freq_Bag[i])
    PB4_Freq_Max=PB4_Freq_Bag[i];
      
    if(PB4_Freq_Min>=PB4_Freq_Bag[i])
    PB4_Freq_Min=PB4_Freq_Bag[i];

  }

  if((PA15_Freq_Max-PA15_Freq_Min)>PD_Disp)
  {
    NDA_Disp+=1;for(i=0; i<30; i++) 
    PA15_Freq_Bag[i] = PA15_Freq;

  } 
   if((PB4_Freq_Max-PB4_Freq_Min)>PD_Disp)
   {
    NDB_Disp+=1;
    for(i=0; i<30; i++) PB4_Freq_Bag[i] = PB4_Freq;
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
/*************************************************************************************************************************/
/*************************************************************************************************************************/
void Error_Handler(void)
{
  
  __disable_irq();
  while (1)
  {
  }
  
}
