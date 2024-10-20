#include "../../inc/MarlinConfig.h"

#if ENABLED(BOARD_CONFIGURE)

#include "board_configure.h"

Board_Configure board_configure;

u16 Board_Configure::Get_Adc(u8 ch) {
	u16 time = 5000;

	ADC1->SQR3 &= 0xFFFFFFE0;
	ADC1->SQR3 |= ch;
	SBI(ADC1->CR2, 30);
	while (!(ADC1->SR & (1 << 1)) && time--) { /* nada */ }
	return ADC1->DR;
}

u16 Board_Configure::Get_Adc_Average(u8 ch, u8 times) {
 	u32 temp_val = 0;
	u8 t;
	for (t = 0; t < times; t++) {
		temp_val += Get_Adc(ch);
		_delay_ms(5);
	}
	return temp_val / times;
}

void Board_Configure::init() {
  SBI(RCC->APB2ENR, 8);
  SBI(RCC->AHB1ENR, 0);
  GPIO_Set(GPIOA, PIN0, GPIO_MODE_AIN, 0, 0, GPIO_PUPD_PU);
	GPIO_Set(GPIOA, PIN1, GPIO_MODE_AIN, 0, 0, GPIO_PUPD_PU);

	SBI(RCC->APB2RSTR, 8);
	CBI(RCC->APB2RSTR, 8);
	ADC->CCR = 3 << 16;

	ADC1->CR1 = 0;
	ADC1->CR2 = 0;
	ADC1->CR1 |= 0 << 24;
	ADC1->CR1 |= 0 << 8;

	CBI(ADC1->CR2, 1);
 	CBI(ADC1->CR2, 11);
	ADC1->CR2 |= 0 << 28;

	ADC1->SQR1 &= ~(0xF << 20);
	ADC1->SQR1 |= 0 << 20;

	ADC1->SMPR2 &= ~(7 << (3*0));
 	ADC1->SMPR2 |=   7 << (3*0);
	ADC1->SMPR2 &= ~(7 << (3*1));
 	ADC1->SMPR2 |=   7 << (3*1);
 	SBI(ADC1->CR2, 0);

	board_configure.adc1 = board_configure.Get_Adc_Average(0, 10);
  board_configure.adc2 = board_configure.Get_Adc_Average(1, 10);

  if (WITHIN(board_configure.adc1, 497, 868)) // 100K/20K 0.55V => 0.4~0.7V
		strcpy(board_configure.board_version, MAIN_BOARD_V8111_V0_5);
	else if (WITHIN(board_configure.adc1, 0, 100))
		strcpy(board_configure.board_version, MAIN_BOARD_V8111_V0_2);
	else if (WITHIN(board_configure.adc1, 1901, 2199))
		strcpy(board_configure.board_version, MAIN_BOARD_V8111_V0_3);

  if (WITHIN(board_configure.adc2, 0, 100))
		strcpy(board_configure.board_chip, GD32F407VGT6_CHIP);
	else if (WITHIN(board_configure.adc2, 1900, 2200))
		strcpy(board_configure.board_chip, STM32F407VGT6_CHIP);
}

#endif // BOARD_CONFIGURE
