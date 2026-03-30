#include "main.h"

extern ADC_HandleTypeDef hadc2;

void R37_ADC2_Init(void);
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle);
uint16_t Get_ADC_R37(void);
