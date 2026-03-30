#include "main.h"

extern TIM_HandleTypeDef htim2;

void PA1_TIM2_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle);
