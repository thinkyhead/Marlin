#include "../../inc/MarlinConfig.h"
#include "sys.h"

void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset) {
	SCB->VTOR = NVIC_VectTab | (Offset & (u32)0xFFFFFE00);
}

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group) {
	u32 temp, temp1;
	temp1 = (~NVIC_Group) & 0x07;
	temp1 <<= 8;
	temp = SCB->AIRCR;
	temp &= 0x0000F8FF;
	temp |= 0x05FA0000;
	temp |= temp1;
	SCB->AIRCR = temp;
}

void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group) {
	u32 temp;
	MY_NVIC_PriorityGroupConfig(NVIC_Group);
	temp = NVIC_PreemptionPriority << (4 - NVIC_Group);
	temp |= NVIC_SubPriority & (0x0F >> NVIC_Group);
	temp &= 0xF;
	SBI(NVIC->ISER[NVIC_Channel / 32], NVIC_Channel % 32);
	NVIC->IP[NVIC_Channel] |= temp << 4;
}

void Ex_NVIC_Config(u8 GPIOx, u8 BITx, u8 TRIM) {
	u8 EXTOFFSET = (BITx % 4) * 4;
	SBI(RCC->APB2ENR, 14);
	SYSCFG->EXTICR[BITx / 4] &= ~(0x000F << EXTOFFSET);
	SYSCFG->EXTICR[BITx / 4] |= GPIOx << EXTOFFSET;

	SBI(EXTI->IMR, BITx);
	if (TRIM & 0x01) SBI(EXTI->FTSR, BITx);
	if (TRIM & 0x02) SBI(EXTI->RTSR, BITx);
}

void GPIO_AF_Set(GPIO_TypeDef* GPIOx, u8 BITx, u8 AFx) {
	GPIOx->AFR[BITx >> 3] &= ~(0x0F << ((BITx & 0x07) * 4));
	GPIOx->AFR[BITx >> 3] |= (u32)AFx << ((BITx & 0x07) * 4);
}

void GPIO_Set(GPIO_TypeDef* GPIOx, u32 BITx, u32 MODE, u32 OTYPE, u32 OSPEED, u32 PUPD) {
	u32 pinpos = 0, pos = 0, curpin = 0;
	for (pinpos = 0; pinpos < 16; pinpos++) {
		pos = _BV(pinpos);
		curpin = BITx & pos;
		if (curpin == pos) {
			GPIOx->MODER &= ~(3 << (pinpos * 2));
			GPIOx->MODER |= MODE << (pinpos * 2);
			if (MODE == 0x01 || MODE == 0x02) {
				GPIOx->OSPEEDR &= ~(3 << (pinpos * 2));
				GPIOx->OSPEEDR |= (OSPEED << (pinpos * 2));
				CBI(GPIOx->OTYPER, pinpos);
				GPIOx->OTYPER |= OTYPE << pinpos;
			}
			GPIOx->PUPDR &= ~(3 << (pinpos * 2));
			GPIOx->PUPDR |= PUPD << (pinpos * 2);
		}
	}
}

void WFI_SET() {
	__ASM volatile("wfi");
}

void INTX_DISABLE() {
	__ASM volatile("cpsid i");
}

void INTX_ENABLE() {
	__ASM volatile("cpsie i");
}

void Sys_Standby() {
	SBI(SCB->SCR, 2);
	SBI(RCC->APB1ENR, 28);
	SBI(PWR->CSR, 8);
	SBI(PWR->CR, 2);
	SBI(PWR->CR, 1);
	WFI_SET();
}

void Sys_Soft_Reset() {
	SCB->AIRCR = 0x05FA0000 | (u32)0x04;
}

u8 Sys_Clock_Set(u32 plln, u32 pllm, u32 pllp, u32 pllq) {
	u16 retry = 0;
	u8 status = 0;
	SBI(RCC->CR, 16);
	while (!TEST(RCC->CR, 17) && retry < 0x1FFF) retry++;
	if (retry == 0x1FFF) status = 1;
	else {
		SBI(RCC->APB1ENR, 28);
		PWR->CR |= 3 << 14;
		RCC->CFGR |= (0 << 4) | (5 << 10) | (4 << 13);
		CBI(RCC->CR, 24);
		RCC->PLLCFGR = pllm | (plln << 6) | (((pllp >> 1) - 1) << 16) | (pllq << 24) | (1 << 22);
		SBI(RCC->CR, 24);
		while ((RCC->CR & (1 << 25)) == 0) { /* nada */ }
		SBI(FLASH->ACR, 8);
		SBI(FLASH->ACR, 9);
		SBI(FLASH->ACR, 10);
		FLASH->ACR |= 5 << 0;
		RCC->CFGR &= ~(3 << 0);
		RCC->CFGR |= 2 << 0;
		while ((RCC->CFGR & (3 << 2)) != (2 << 2));
	}
	return status;
}

void Stm32_Clock_Init(u32 plln, u32 pllm, u32 pllp, u32 pllq) {
	SBI(RCC->CR, 0);
	RCC->CFGR = 0x00000000;
	RCC->CR &= 0xFEF6FFFF;
	RCC->PLLCFGR = 0x24003010;
	CBI(RCC->CR, 18);
	RCC->CIR = 0x00000000;
	Sys_Clock_Set(plln, pllm, pllp, pllq);
  #ifdef VECT_TAB_RAM
  	MY_NVIC_SetVectorTable(_BV32(29), 0x0);
  #else
  	MY_NVIC_SetVectorTable(0, 0x0);
  #endif
}
