#include "main.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim17;

void PA6_TIM3_Init(void);
void PA7_TIM17_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
