#include "main.h"


extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

void PA15_TIM2_Init(void);
void PB4_TIM3_Init(void);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle);

