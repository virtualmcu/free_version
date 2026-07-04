/* startup.c — STM32F103 minimal startup */
#include <stdint.h>

extern uint32_t _etext, _sdata, _edata, _sbss, _ebss, _stack_top;
extern int main(void);

void __attribute__((section(".text.Reset_Handler"),noreturn))
Reset_Handler(void) {
    uint32_t *src = &_etext, *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;
    main();
    while (1);
}

void Default_Handler(void) { while (1); }
#define WEAK __attribute__((weak, alias("Default_Handler")))
/* System exceptions */
void NMI_Handler(void) WEAK;
void HardFault_Handler(void) WEAK;
void SVC_Handler(void) WEAK;
void PendSV_Handler(void) WEAK;
void SysTick_Handler(void) WEAK;
/* STM32F103 peripheral IRQs (must be weak so user can override) */
void WWDG_IRQHandler(void) WEAK;            /* 0 */
void PVD_IRQHandler(void) WEAK;             /* 1 */
void TAMPER_IRQHandler(void) WEAK;          /* 2 */
void RTC_IRQHandler(void) WEAK;             /* 3 */
void FLASH_IRQHandler(void) WEAK;           /* 4 */
void RCC_IRQHandler(void) WEAK;             /* 5 */
void EXTI0_IRQHandler(void) WEAK;           /* 6 */
void EXTI1_IRQHandler(void) WEAK;           /* 7 */
void EXTI2_IRQHandler(void) WEAK;           /* 8 */
void EXTI3_IRQHandler(void) WEAK;           /* 9 */
void EXTI4_IRQHandler(void) WEAK;           /* 10 */
void DMA1_Channel1_IRQHandler(void) WEAK;   /* 11 */
void DMA1_Channel2_IRQHandler(void) WEAK;   /* 12 */
void DMA1_Channel3_IRQHandler(void) WEAK;   /* 13 */
void DMA1_Channel4_IRQHandler(void) WEAK;   /* 14 */
void DMA1_Channel5_IRQHandler(void) WEAK;   /* 15 */
void DMA1_Channel6_IRQHandler(void) WEAK;   /* 16 */
void DMA1_Channel7_IRQHandler(void) WEAK;   /* 17 */
void ADC1_2_IRQHandler(void) WEAK;          /* 18 */
void USB_HP_CAN1_TX_IRQHandler(void) WEAK;  /* 19 */
void USB_LP_CAN1_RX0_IRQHandler(void) WEAK; /* 20 */
void CAN1_RX1_IRQHandler(void) WEAK;        /* 21 */
void CAN1_SCE_IRQHandler(void) WEAK;        /* 22 */
void EXTI9_5_IRQHandler(void) WEAK;         /* 23 */
void TIM1_BRK_IRQHandler(void) WEAK;        /* 24 */
void TIM1_UP_IRQHandler(void) WEAK;         /* 25 */
void TIM1_TRG_COM_IRQHandler(void) WEAK;    /* 26 */
void TIM1_CC_IRQHandler(void) WEAK;         /* 27 */
void TIM2_IRQHandler(void) WEAK;            /* 28 */
void TIM3_IRQHandler(void) WEAK;            /* 29 */
void TIM4_IRQHandler(void) WEAK;            /* 30 */
void I2C1_EV_IRQHandler(void) WEAK;         /* 31 */
void I2C1_ER_IRQHandler(void) WEAK;         /* 32 */
void I2C2_EV_IRQHandler(void) WEAK;         /* 33 */
void I2C2_ER_IRQHandler(void) WEAK;         /* 34 */
void SPI1_IRQHandler(void) WEAK;            /* 35 */
void SPI2_IRQHandler(void) WEAK;            /* 36 */
void USART1_IRQHandler(void) WEAK;          /* 37 */
void USART2_IRQHandler(void) WEAK;          /* 38 */
void USART3_IRQHandler(void) WEAK;          /* 39 */
void EXTI15_10_IRQHandler(void) WEAK;       /* 40 */
void RTC_Alarm_IRQHandler(void) WEAK;       /* 41 */
void USBWakeUp_IRQHandler(void) WEAK;       /* 42 */

__attribute__((section(".isr_vector")))
const uint32_t vectors[] = {
    /* Cortex-M3 system vectors (0-15) */
    (uint32_t)&_stack_top,        /*  0 — initial SP */
    (uint32_t)&Reset_Handler,     /*  1 — reset */
    (uint32_t)&NMI_Handler,       /*  2 */
    (uint32_t)&HardFault_Handler, /*  3 */
    0, 0, 0, 0, 0, 0, 0,          /*  4-10 — reserved/unused */
    (uint32_t)&SVC_Handler,       /* 11 */
    0, 0,                         /* 12-13 — reserved */
    (uint32_t)&PendSV_Handler,    /* 14 */
    (uint32_t)&SysTick_Handler,   /* 15 */
    /* External IRQs (16 + N) */
    (uint32_t)&WWDG_IRQHandler,            /* 16 = IRQ  0 */
    (uint32_t)&PVD_IRQHandler,             /* 17 = IRQ  1 */
    (uint32_t)&TAMPER_IRQHandler,          /* 18 = IRQ  2 */
    (uint32_t)&RTC_IRQHandler,             /* 19 = IRQ  3 */
    (uint32_t)&FLASH_IRQHandler,           /* 20 = IRQ  4 */
    (uint32_t)&RCC_IRQHandler,             /* 21 = IRQ  5 */
    (uint32_t)&EXTI0_IRQHandler,           /* 22 = IRQ  6 */
    (uint32_t)&EXTI1_IRQHandler,           /* 23 = IRQ  7 */
    (uint32_t)&EXTI2_IRQHandler,           /* 24 = IRQ  8 */
    (uint32_t)&EXTI3_IRQHandler,           /* 25 = IRQ  9 */
    (uint32_t)&EXTI4_IRQHandler,           /* 26 = IRQ 10 */
    (uint32_t)&DMA1_Channel1_IRQHandler,   /* 27 = IRQ 11 */
    (uint32_t)&DMA1_Channel2_IRQHandler,   /* 28 = IRQ 12 */
    (uint32_t)&DMA1_Channel3_IRQHandler,   /* 29 = IRQ 13 */
    (uint32_t)&DMA1_Channel4_IRQHandler,   /* 30 = IRQ 14 */
    (uint32_t)&DMA1_Channel5_IRQHandler,   /* 31 = IRQ 15 */
    (uint32_t)&DMA1_Channel6_IRQHandler,   /* 32 = IRQ 16 */
    (uint32_t)&DMA1_Channel7_IRQHandler,   /* 33 = IRQ 17 */
    (uint32_t)&ADC1_2_IRQHandler,          /* 34 = IRQ 18 */
    (uint32_t)&USB_HP_CAN1_TX_IRQHandler,  /* 35 = IRQ 19 */
    (uint32_t)&USB_LP_CAN1_RX0_IRQHandler, /* 36 = IRQ 20 */
    (uint32_t)&CAN1_RX1_IRQHandler,        /* 37 = IRQ 21 */
    (uint32_t)&CAN1_SCE_IRQHandler,        /* 38 = IRQ 22 */
    (uint32_t)&EXTI9_5_IRQHandler,         /* 39 = IRQ 23 */
    (uint32_t)&TIM1_BRK_IRQHandler,        /* 40 = IRQ 24 */
    (uint32_t)&TIM1_UP_IRQHandler,         /* 41 = IRQ 25 */
    (uint32_t)&TIM1_TRG_COM_IRQHandler,    /* 42 = IRQ 26 */
    (uint32_t)&TIM1_CC_IRQHandler,         /* 43 = IRQ 27 */
    (uint32_t)&TIM2_IRQHandler,            /* 44 = IRQ 28 */
    (uint32_t)&TIM3_IRQHandler,            /* 45 = IRQ 29 */
    (uint32_t)&TIM4_IRQHandler,            /* 46 = IRQ 30 */
    (uint32_t)&I2C1_EV_IRQHandler,         /* 47 = IRQ 31 */
    (uint32_t)&I2C1_ER_IRQHandler,         /* 48 = IRQ 32 */
    (uint32_t)&I2C2_EV_IRQHandler,         /* 49 = IRQ 33 */
    (uint32_t)&I2C2_ER_IRQHandler,         /* 50 = IRQ 34 */
    (uint32_t)&SPI1_IRQHandler,            /* 51 = IRQ 35 */
    (uint32_t)&SPI2_IRQHandler,            /* 52 = IRQ 36 */
    (uint32_t)&USART1_IRQHandler,          /* 53 = IRQ 37 */
    (uint32_t)&USART2_IRQHandler,          /* 54 = IRQ 38 */
    (uint32_t)&USART3_IRQHandler,          /* 55 = IRQ 39 */
    (uint32_t)&EXTI15_10_IRQHandler,       /* 56 = IRQ 40 */
    (uint32_t)&RTC_Alarm_IRQHandler,       /* 57 = IRQ 41 */
    (uint32_t)&USBWakeUp_IRQHandler,       /* 58 = IRQ 42 */
};
