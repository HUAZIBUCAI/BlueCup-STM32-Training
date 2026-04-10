#include "main.h"

extern TIM_HandleTypeDef htim3;
void PA7_TIM3_Init(void);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle);
