#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
void R38_ADC1_Init(void);
void R37_ADC2_Init(void);
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle);
uint16_t Get_ADC1_R38(void);
uint16_t Get_ADC2_R37(void);
