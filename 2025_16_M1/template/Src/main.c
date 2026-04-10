#include "main.h"
#include "rcc\bsp_rcc.h"
#include "key_led\bsp_key_led.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "adc\bsp_adc.h"
#include "iic\bsp_iic.h"



__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_3seconds_Set_Point=0;

uint8_t Key_Value,Key_Down,Key_Up,Key_Old;
uint8_t ucLed=0x00;
uint8_t String_Disp[20];

uint8_t EEPROM_String1[5]={22,33,44};
uint8_t EEPROM_String2[5]={0,0,0};
uint8_t Res;

uint8_t IntereFace=0x00;

uint8_t Lock_Num1=0;
uint8_t Lock_Num2=0;
uint8_t Lock_Num3=0;

uint8_t Set_Num1=0;
uint8_t Set_Num2=1;
uint8_t Set_Num3=2;
uint8_t Set_Time_Old;

uint8_t Storge_Num1=0;
uint8_t Storge_Num2=1;
uint8_t Storge_Num3=2;



double R37_Voltage;
uint8_t second3=0;
_Bool Out_time_Flag=0;


static void Led_Proc(void);
static void Key_Proc(void);
static void Lcd_Proc(void);
static void Check_3seconds_Proc(void);

uint8_t Return_Voltage(void);
void Input_R37(void);
_Bool Check_Pass_Word(void);
void Storge_Pass_Word(void);
void Check_First_Storge(void);

int main(void)
{

  
  HAL_Init();

 
  SystemClock_Config();
  
  Key_Led_GPIO_Init();
  Led_Disp(0x01);

  LCD_Init();
  LCD_SetTextColor(White);
  LCD_SetBackColor(Black);
  LCD_Clear(Black);

  R37_ADC2_Init();

  I2CInit();

  Check_First_Storge();
  // AT24C02_Write(EEPROM_String1,0,5);
  // HAL_Delay(5);
  // AT24C02_Read(EEPROM_String2,0,5);

  // HAL_Delay(10);
  // MCP1407_Write(0x03);
  // HAL_Delay(10);
  // Res=MCP1407_Read();
  // HAL_Delay(10);
  while (1)
  {
    Led_Proc();
    Key_Proc();
    Lcd_Proc();
    Check_3seconds_Proc();
  }
  
  
}


static void Led_Proc(void)
{
  if(uwTick-uwTick_Led_Set_Point<=100)return;
  uwTick_Led_Set_Point=uwTick;
  if(IntereFace>>4==0x00)
  {
    ucLed=0x01;
  }
  else if(IntereFace>>4==0x01)
  {
    ucLed=0x02;
  }
  else
  {
    ucLed=0x00;
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
    if(IntereFace>>4==0x00)
    {
      IntereFace++;
      if(IntereFace==0x03)
      {
        if(Check_Pass_Word()==1)
        {
          IntereFace=0x10;
          Input_R37();
          Set_Time_Old=Set_Num1;
        }
        else
        {
          IntereFace=0x00;
        }
      }
    }
    
   else if(IntereFace>>4==0x01)
    {
      IntereFace++;
      if(IntereFace==0x13)
      {
        Storge_Pass_Word();
        IntereFace=0x00;
      }
    }
  }

}

void Storge_Pass_Word(void)
{
    AT24C02_Write(&Set_Num1,0x00,1);
    HAL_Delay(2);
    AT24C02_Write(&Set_Num2,0x01,1);
    HAL_Delay(2);
    AT24C02_Write(&Set_Num3,0x02,1);
    HAL_Delay(2);
}

_Bool Check_Pass_Word(void)
{
  
  AT24C02_Read(&Storge_Num1,0x00,1);
  HAL_Delay(2);
  AT24C02_Read(&Storge_Num2,0x01,1);
  HAL_Delay(2);
  AT24C02_Read(&Storge_Num3,0x02,1);
  HAL_Delay(2);
  if( (Lock_Num1==Storge_Num1)&&(Lock_Num2==Storge_Num2)&&(Lock_Num3==Storge_Num3))
  {
    Set_Num1=Storge_Num1;
    Set_Num2=Storge_Num2;
    Set_Num3=Storge_Num3;
    return 1;
  }
  return 0;
}

uint8_t Return_Voltage(void)
{
  R37_Voltage=Get_ADC2_R37()*3.3/4096;
  if(R37_Voltage<1.50)
  {
    return 0;
  }
  else if(R37_Voltage>=1.50&&R37_Voltage<=2.50)
  {
    return 1;
  }
  return 2;

}

void Input_R37(void)
{
    if(IntereFace==0x00)
    {

      Lock_Num1=Return_Voltage();
      
    }
    else if(IntereFace==0x01)
    {
      Lock_Num2=Return_Voltage();
    }
    else if(IntereFace==0x02)
    {
      Lock_Num3=Return_Voltage();
    }
    else if(IntereFace==0x10)
    {
      Set_Num1=Return_Voltage();
    }
    else if(IntereFace==0x11)
    {
      Set_Num2=Return_Voltage();
    }
    else if(IntereFace==0x12)
    {
      Set_Num3=Return_Voltage();
    }




}
static void Lcd_Proc(void)
{
  if(uwTick-uwTick_Lcd_Set_Point<=200)return;
  uwTick_Lcd_Set_Point=uwTick;

  Input_R37();
  if(IntereFace==0x00)
  {
    sprintf( (char *)String_Disp,"        Lock");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"      Pass Word");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d * *    ",Lock_Num1);
    LCD_DisplayStringLine(Line4,String_Disp);
  }
  
  else if(IntereFace==0x01)
  {
    sprintf( (char *)String_Disp,"        Lock");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"      Pass Word");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d %d *   ",Lock_Num1,Lock_Num2);
    LCD_DisplayStringLine(Line4,String_Disp);
  }

    else if(IntereFace==0x02)
  {
    sprintf( (char *)String_Disp,"        Lock");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"      Pass Word");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d %d %d    ",Lock_Num1,Lock_Num2,Lock_Num3);
    LCD_DisplayStringLine(Line4,String_Disp);
  }


  else if(IntereFace==0x10)
  {
    sprintf( (char *)String_Disp,"        Set   ");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"       Change    ");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d * *    ",Set_Num1);
    LCD_DisplayStringLine(Line4,String_Disp);
  }

  else if(IntereFace==0x11)
  {
    sprintf( (char *)String_Disp,"        Set   ");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"       Change    ");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d %d *    ",Set_Num1,Set_Num2);
    LCD_DisplayStringLine(Line4,String_Disp);
  }

    else if(IntereFace==0x12)
  {
    sprintf( (char *)String_Disp,"        Set   ");
    LCD_DisplayStringLine(Line1,String_Disp);

    sprintf( (char *)String_Disp,"       Change    ");
    LCD_DisplayStringLine(Line3,String_Disp);

    sprintf( (char *)String_Disp,"        %d %d %d    ",Set_Num1,Set_Num2,Set_Num3);
    LCD_DisplayStringLine(Line4,String_Disp);
  }


  // sprintf( (char *)String_Disp,"Hello,World");
  // LCD_DisplayStringLine(Line0,String_Disp);
  
  
  // sprintf( (char *)String_Disp,"%.1fV",Get_ADC2_R37()*3.3/4096);
  // LCD_DisplayStringLine(Line1,String_Disp);

  // sprintf( (char *)String_Disp,"%d %d %d  %x",EEPROM_String2[0],EEPROM_String2[1],EEPROM_String2[2],Res );
  // LCD_DisplayStringLine(Line2,String_Disp);
}




void Check_First_Storge(void)
{
  uint8_t value=0xf5;
  uint8_t First_Storge=0x00;
  AT24C02_Read(&First_Storge,0x03,1);
  HAL_Delay(2);
  if(First_Storge!=0xf5)//判断是否是第一次存取 如果是 那么0x03的值坑定不是0xf5 所以将默认的0 1 2 存入 标准位存入
  {
      AT24C02_Write(&Set_Num1,0x00,1);
      HAL_Delay(2);
      AT24C02_Write(&Set_Num2,0x01,1);
      HAL_Delay(2);
      AT24C02_Write(&Set_Num3,0x02,1);
      HAL_Delay(2);
      AT24C02_Write(&value,0x03,1);
      HAL_Delay(2);
  }

}

static void Check_3seconds_Proc(void)
{
  if(IntereFace==0x10)
  {
    if(uwTick-uwTick_3seconds_Set_Point<=500)return;
    uwTick_3seconds_Set_Point=uwTick;
    if(Set_Num1==Set_Time_Old)
    {
      second3++;
      if(second3>=6)
     {
      Out_time_Flag=1;
      second3=0;
      }
    }
    else
    {
      second3=0;
    }
      
    if(Out_time_Flag==1)
    {
      Out_time_Flag=0;
      IntereFace=0x00;
    }
    

  }
  


}

/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/
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