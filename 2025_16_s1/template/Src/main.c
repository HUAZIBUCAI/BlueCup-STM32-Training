#include "main.h"
#include "rcc\bsp_rcc.h"
#include "key_led\bsp_key_led.h"
#include "lcd\bsp_lcd.h"
#include "stdio.h"
#include "string.h"
#include "adc\bsp_adc.h"
#include "tim\bsp_tim.h"

// ====== 系统时间基准 ======
__IO uint32_t uwTick_Key_Set_Point=0;
__IO uint32_t uwTick_Led_Set_Point=0;
__IO uint32_t uwTick_Lcd_Set_Point=0;
__IO uint32_t uwTick_Sec_Set_Point=0; // 用于计算秒数

// ====== 题目要求的全局系统变量 ======
uint32_t Run_Time_Sec = 0;         // 系统运行总时长(秒)
uint8_t InterFace = 0x00;          // 0x00:监控  0x10:统计  0x20:参数
uint8_t Para_Select = 0;           // 参数选中项: 0:DS, 1:DR, 2:FS, 3:FR
_Bool Lock_Flag = 0;               // 0:解锁, 1:锁定
_Bool Error_Flag = 0;              // 0:正常, 1:异常 (频差>1000)
_Bool Last_Error_Flag = 0;         // 记录上一次异常状态，用于抓拍快照

// ====== 四大核心参数 (附带初始值) ======
uint8_t PARA_DS = 1;     // 占空比步长 1%
uint8_t PARA_DR = 80;    // 占空比最大范围 80%
uint16_t PARA_FS = 100;  // 频率步长 100Hz
uint16_t PARA_FR = 2000; // 频率最大范围 2000Hz

// ====== 数据采集与输出控制 ======
uint8_t ucLed = 0x00;
uint8_t Key_Value, Key_Down, Key_Up, Key_Old;
uint8_t String_Disp[20];

double R37_Voltage;
double R38_Voltage;

uint16_t Target_Freq = 1000; // PA7 实际输出频率 CF
uint8_t  Target_Duty = 10;   // PA7 实际输出占空比 CD
uint16_t PA15_Freq = 0;      // PA15 测到的频率 DF
uint16_t XF_Diff = 0;        // 频率差值 XF

uint16_t PA15_T_Counter = 0;
uint16_t PA15_D_Counter = 0;

// ====== 统计界面(RECD) 快照记忆库 ======
uint16_t RECD_CF = 0;
uint8_t  RECD_CD = 0;
uint16_t RECD_DF = 0;
uint16_t RECD_XF = 0;
uint32_t RECD_Time_Sec = 0;


// ====== 函数声明 ======
static void Key_Proc(void);
static void Led_Proc(void);
static void Lcd_Proc(void);
static void Data_Process_Proc(void); // 数据与时间处理后台
static void PWM_Control_Proc(void);  // 阶梯式PWM控制后台


int main(void)
{
    HAL_Init();
    SystemClock_Config();

    Key_Led_GPIO_Init();
    Led_Disp(0x00); // 初始全灭，交给 Led_Proc 去管

    LCD_Init();
    LCD_SetTextColor(White);
    LCD_SetBackColor(Black);
    LCD_Clear(Black);

    R38_ADC1_Init();
    R37_ADC2_Init();

    PA15_TIM2_Init();
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);

    PA7_TIM3_Init();
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    while (1)
    {
        Key_Proc();          // 按键扫描
        Data_Process_Proc(); // 计时与异常判断后台
        PWM_Control_Proc();  // 阶梯PWM输出后台
        Lcd_Proc();          // 屏幕刷新
        Led_Proc();          // LED控制
    }
}


// =========================================================================
// 1. 数据与时间处理后台 (计算秒数、判断异常、抓拍快照)
// =========================================================================
static void Data_Process_Proc(void)
{
    // --- 1秒精准计时 ---
    if(uwTick - uwTick_Sec_Set_Point >= 1000)
    {
        uwTick_Sec_Set_Point += 1000;
        Run_Time_Sec++;
    }

    // --- 计算频差 XF ---
    if(Target_Freq > PA15_Freq) XF_Diff = Target_Freq - PA15_Freq;
    else                        XF_Diff = PA15_Freq - Target_Freq;

    // --- 异常状态判定 ---
    if(XF_Diff > 1000) Error_Flag = 1;
    else               Error_Flag = 0;

    // --- 🚨 核心考点：只在触发异常的一瞬间抓拍！ ---
    if(Error_Flag == 1 && Last_Error_Flag == 0)
    {
        RECD_CF = Target_Freq;
        RECD_CD = Target_Duty;
        RECD_DF = PA15_Freq;
        RECD_XF = XF_Diff;
        RECD_Time_Sec = Run_Time_Sec;
    }
    Last_Error_Flag = Error_Flag;
}

// =========================================================================
// 2. 阶梯式 PWM 控制 (核心难点)
// =========================================================================
static void PWM_Control_Proc(void)
{
    if(Lock_Flag == 1) return; // 💡 如果锁定，无视电位器，直接返回

    // 假设你的底层读取函数名是这两个，如果名字不一样请自行修改
    R37_Voltage = Get_ADC2_R37() * 3.3 / 4095.0; 
    R38_Voltage = Get_ADC1_R38() * 3.3 / 4095.0;

    // --- 阶梯占空比计算 (R37控制) ---
    uint8_t n_duty_steps = (PARA_DR - 10) / PARA_DS; 
    double v_step_duty = 3.3 / n_duty_steps;         
    uint8_t k_duty = (uint8_t)(R37_Voltage / v_step_duty);
    if(k_duty > n_duty_steps) k_duty = n_duty_steps; // 封顶保护
    Target_Duty = 10 + k_duty * PARA_DS;

    // --- 阶梯频率计算 (R38控制) ---
    uint16_t n_freq_steps = (PARA_FR - 1000) / PARA_FS;
    double v_step_freq = 3.3 / n_freq_steps;
    uint16_t k_freq = (uint16_t)(R38_Voltage / v_step_freq);
    if(k_freq > n_freq_steps) k_freq = n_freq_steps;
    Target_Freq = 1000 + k_freq * PARA_FS;

    // --- 更新 TIM3 ---
    uint32_t arr_val = 1000000 / Target_Freq;
    uint32_t ccr_val = arr_val * Target_Duty / 100;

    __HAL_TIM_SET_AUTORELOAD(&htim3, arr_val - 1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr_val);
}

// =========================================================================
// 3. 完美按键逻辑 (复合按键防冲突)
// =========================================================================
static void Key_Proc(void)
{
    if(uwTick - uwTick_Key_Set_Point <= 50) return;
    uwTick_Key_Set_Point = uwTick;

    Key_Value = Key_Scan();
    Key_Down = Key_Value & (Key_Value ^ Key_Old);
    Key_Up = ~Key_Value & (Key_Value ^ Key_Old);
    Key_Old = Key_Value;

    // ====== B1: 切换界面 ======
    if(Key_Down == 1)
    {
        LCD_Clear(Black);
        InterFace += 0x10;
        if(InterFace >= 0x30) InterFace = 0x00;
        
        if(InterFace == 0x20) Para_Select = 0; // 每次进参数界面，默认选DS
    }

    // ====== B2: 复合按键处理 ======
    if(InterFace == 0x20) // 在参数界面，短按切换参数
    {
        if(Key_Down == 2)
        {
            Para_Select++;
            if(Para_Select > 3) Para_Select = 0;
        }
    }
    else if(InterFace == 0x00) // 在监控界面，短按锁定，长按清零
    {
        static uint16_t b2_long_cnt = 0;
        if(Key_Value == 2) 
        {
            b2_long_cnt++;
            if(b2_long_cnt == 40) // 50ms * 40 = 2秒
            {
                Run_Time_Sec = 0; // 🚀 长按触发：时间清零
            }
        }
        else
        {
            if(Key_Up == 2)
            {
                if(b2_long_cnt > 0 && b2_long_cnt < 40) 
                {
                    Lock_Flag ^= 1; // 🔓 短按松手：切换锁定状态
                }
            }
            b2_long_cnt = 0;
        }
    }

    // ====== B3: 加 ======
    if(Key_Down == 3 && InterFace == 0x20)
    {
        if(Para_Select == 0)      { PARA_DS += 1;   if(PARA_DS > 10) PARA_DS = 10; }
        else if(Para_Select == 1) { PARA_DR += 10;  if(PARA_DR > 100) PARA_DR = 100; }
        else if(Para_Select == 2) { PARA_FS += 100; if(PARA_FS > 1000) PARA_FS = 1000; }
        else if(Para_Select == 3) { PARA_FR += 1000; if(PARA_FR > 10000) PARA_FR = 10000; }
    }

    // ====== B4: 减 ======
    if(Key_Down == 4 && InterFace == 0x20)
    {
        if(Para_Select == 0)      { PARA_DS -= 1;   if(PARA_DS < 1) PARA_DS = 1; }
        else if(Para_Select == 1) { PARA_DR -= 10;  if(PARA_DR < 20) PARA_DR = 20; }
        else if(Para_Select == 2) { PARA_FS -= 100; if(PARA_FS < 100) PARA_FS = 100; }
        else if(Para_Select == 3) { PARA_FR -= 1000; if(PARA_FR < 2000) PARA_FR = 2000; }
    }
}

// =========================================================================
// 4. 屏幕刷新逻辑 (要求100%复刻题目排版)
// =========================================================================
static void Lcd_Proc(void)
{
    if(uwTick - uwTick_Lcd_Set_Point <= 100) return;
    uwTick_Lcd_Set_Point = uwTick;

    // 格式化当前时间
    uint8_t HH = Run_Time_Sec / 3600;
    uint8_t MM = (Run_Time_Sec % 3600) / 60;
    uint8_t SS = Run_Time_Sec % 60;

    if(InterFace == 0x00) // DATA
    {
        sprintf((char *)String_Disp, "       PWM       ");
        LCD_DisplayStringLine(Line1, String_Disp);
        sprintf((char *)String_Disp, "   CF=%dHz       ", Target_Freq);
        LCD_DisplayStringLine(Line3, String_Disp);
        sprintf((char *)String_Disp, "   CD=%d%%        ", Target_Duty);
        LCD_DisplayStringLine(Line4, String_Disp);
        sprintf((char *)String_Disp, "   DF=%dHz       ", PA15_Freq);
        LCD_DisplayStringLine(Line5, String_Disp);
        
        if(Lock_Flag) sprintf((char *)String_Disp, "   ST=LOCK      ");
        else          sprintf((char *)String_Disp, "   ST=UNLOCK    ");
        LCD_DisplayStringLine(Line6, String_Disp);

        sprintf((char *)String_Disp, "   %02dH%02dM%02dS    ", HH, MM, SS);
        LCD_DisplayStringLine(Line7, String_Disp);
    }
    else if(InterFace == 0x10) // RECD
    {
        // 格式化快照时间
        uint8_t R_HH = RECD_Time_Sec / 3600;
        uint8_t R_MM = (RECD_Time_Sec % 3600) / 60;
        uint8_t R_SS = RECD_Time_Sec % 60;

        sprintf((char *)String_Disp, "       RECD     ");
        LCD_DisplayStringLine(Line1, String_Disp);
        sprintf((char *)String_Disp, "   CF=%dHz       ", RECD_CF);
        LCD_DisplayStringLine(Line3, String_Disp);
        sprintf((char *)String_Disp, "   CD=%d%%        ", RECD_CD);
        LCD_DisplayStringLine(Line4, String_Disp);
        sprintf((char *)String_Disp, "   DF=%dHz       ", RECD_DF);
        LCD_DisplayStringLine(Line5, String_Disp);
        sprintf((char *)String_Disp, "   XF=%dHz       ", RECD_XF);
        LCD_DisplayStringLine(Line6, String_Disp);
        sprintf((char *)String_Disp, "   %02dH%02dM%02dS    ", R_HH, R_MM, R_SS);
        LCD_DisplayStringLine(Line7, String_Disp);
    }
    else if(InterFace == 0x20) // PARA
    {
        sprintf((char *)String_Disp, "       PARA     ");
        LCD_DisplayStringLine(Line1, String_Disp);
        
        sprintf((char *)String_Disp, " %c DS=%d%%       ", (Para_Select==0)?'>':' ', PARA_DS);
        LCD_DisplayStringLine(Line3, String_Disp);
        sprintf((char *)String_Disp, " %c DR=%d%%       ", (Para_Select==1)?'>':' ', PARA_DR);
        LCD_DisplayStringLine(Line4, String_Disp);
        sprintf((char *)String_Disp, " %c FS=%dHz       ", (Para_Select==2)?'>':' ', PARA_FS);
        LCD_DisplayStringLine(Line5, String_Disp);
        sprintf((char *)String_Disp, " %c FR=%dHz       ", (Para_Select==3)?'>':' ', PARA_FR);
        LCD_DisplayStringLine(Line6, String_Disp);
        
        sprintf((char *)String_Disp, "                    "); // 清除下方残影
        LCD_DisplayStringLine(Line7, String_Disp);
    }
}

// =========================================================================
// 5. LED 控制 (优雅处理所有亮灭逻辑)
// =========================================================================
static void Led_Proc(void)
{
    if(uwTick - uwTick_Led_Set_Point <= 100) return;
    uwTick_Led_Set_Point = uwTick;

    ucLed = 0x00; // 先全部熄灭

    if(InterFace == 0x00) ucLed |= 0x01; // LD1：监控界面点亮
    if(Lock_Flag)         ucLed |= 0x02; // LD2：锁定点亮
    if(Error_Flag)        ucLed |= 0x04; // LD3：频率异常(>1000)点亮

    Led_Disp(ucLed);
}

// =========================================================================
// 6. 输入捕获中断回调 (附带防死机保护)
// =========================================================================
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            PA15_T_Counter = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
            if(PA15_T_Counter != 0) // 🚨 极度关键：防止开机除以0导致死机！
            {
                PA15_Freq = 1000000 / PA15_T_Counter;
                // 注意：如果需要用到PA15_Duty，在这下面继续算 D/T
            }
        }
        else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
            PA15_D_Counter = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        }
    }
}

// =========================================================================
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}