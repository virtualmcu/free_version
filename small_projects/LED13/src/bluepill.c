// =============================================================================
// bluepill.c — implementation. You usually don't need to read this.
// =============================================================================
#include "bluepill.h"

#define _REG(addr)  (*(volatile uint32_t*)(uintptr_t)(addr))
#define RCC_APB2ENR _REG(0x40021018)
#define RCC_APB1ENR _REG(0x4002101C)

static const uint32_t GPIO_BASE[4] = {
    0x40010800u, 0x40010C00u, 0x40011000u, 0x40011400u
};
#define GPIO_CRL(b)  _REG((b)+0x00)
#define GPIO_CRH(b)  _REG((b)+0x04)
#define GPIO_IDR(b)  _REG((b)+0x08)
#define GPIO_ODR(b)  _REG((b)+0x0C)
#define GPIO_BSRR(b) _REG((b)+0x10)
#define GPIOA_BASE   0x40010800u
#define GPIOB_BASE   0x40010C00u
#define GPIOC_BASE   0x40011000u
#define GPIOA_CRL    GPIO_CRL(GPIOA_BASE)
#define GPIOA_CRH    GPIO_CRH(GPIOA_BASE)
#define GPIOB_CRL    GPIO_CRL(GPIOB_BASE)
#define GPIOB_CRH    GPIO_CRH(GPIOB_BASE)

#define USART1_BASE  0x40013800u
#define USART2_BASE  0x40004400u
#define USART_SR(b)  _REG((b)+0x00)
#define USART_DR(b)  _REG((b)+0x04)
#define USART_BRR(b) _REG((b)+0x08)
#define USART_CR1(b) _REG((b)+0x0C)

static inline uint8_t  _port(uint8_t pin){ return (pin>>4)&0x0F; }
static inline uint8_t  _idx (uint8_t pin){ return pin&0x0F; }
static inline uint32_t _portBase(uint8_t pin){ return GPIO_BASE[_port(pin)]; }
static void _enableGpioClock(uint8_t port){ RCC_APB2ENR |= (1u << (2+port)); }

void pinMode(uint8_t pin, PinMode mode){
    uint8_t port = _port(pin); uint8_t idx = _idx(pin);
    uint32_t base = _portBase(pin);
    _enableGpioClock(port);
    uint32_t shift = (idx & 7) * 4; uint32_t mask = 0xFu << shift;
    volatile uint32_t* reg = (idx < 8) ? &GPIO_CRL(base) : &GPIO_CRH(base);
    uint32_t cfg;
    switch (mode){
        case OUTPUT:         cfg = 0x2; break;
        case OUTPUT_OD:      cfg = 0x6; break;
        case INPUT:          cfg = 0x4; break;
        case INPUT_PULLUP:   cfg = 0x8; break;
        case INPUT_PULLDOWN: cfg = 0x8; break;
        default:             cfg = 0x4; break;
    }
    *reg = (*reg & ~mask) | ((cfg & 0xFu) << shift);
    if (mode == INPUT_PULLUP)        GPIO_BSRR(base) = (1u << idx);
    else if (mode == INPUT_PULLDOWN) GPIO_BSRR(base) = (1u << (idx+16));
}

void digitalWrite(uint8_t pin, uint8_t value){
    uint32_t base = _portBase(pin); uint8_t idx = _idx(pin);
    GPIO_BSRR(base) = (value ? 1u : (1u << 16)) << idx;
}

uint8_t digitalRead(uint8_t pin){
    uint32_t base = _portBase(pin); uint8_t idx = _idx(pin);
    return (GPIO_IDR(base) >> idx) & 1u;
}

// Timing — SysTick driven
//
// SysTick is the standard ARM Cortex-M system timer, mapped at
// 0xE000E010..0xE000E01C. We program it as a 1ms tick at 8MHz HSI,
// then count ticks via the COUNTFLAG bit. The same code works on
// real STM32F103 hardware running from HSI.
//
#define SYST_CSR  _REG(0xE000E010)
#define SYST_RVR  _REG(0xE000E014)
#define SYST_CVR  _REG(0xE000E018)
#define SYST_CSR_ENABLE     (1u << 0)
#define SYST_CSR_CLKSOURCE  (1u << 2)   // 1 = use processor clock
#define SYST_CSR_COUNTFLAG  (1u << 16)  // set when counter wraps

static volatile uint32_t _systick_ms = 0;
static int               _systick_started = 0;

// Configure SysTick for a 1ms tick @ 8MHz HSI default.
// Idempotent: safe to call multiple times.
static void _systick_init(void){
    if (_systick_started) return;
    SYST_CSR = 0;                   // disable
    SYST_RVR = 8000u - 1u;          // 1ms @ 8MHz
    SYST_CVR = 0;                   // clear current value + COUNTFLAG
    SYST_CSR = SYST_CSR_ENABLE | SYST_CSR_CLKSOURCE;
    _systick_started = 1;
}

// Poll-based 1ms tick advance. Increments _systick_ms each time
// the down-counter has wrapped since the last call.
static void _systick_pump(void){
    if (SYST_CSR & SYST_CSR_COUNTFLAG)
        _systick_ms++;
}

void delay(uint32_t ms){
    _systick_init();
    uint32_t start = _systick_ms;
    while ((uint32_t)(_systick_ms - start) < ms) {
        _systick_pump();
    }
}

void delayMicroseconds(uint32_t us){
    // 1ms granularity in this implementation; round up to 1ms minimum.
    if (us == 0) return;
    delay((us + 999u) / 1000u);
}

uint32_t millis(void){
    _systick_init();
    _systick_pump();
    return _systick_ms;
}
uint32_t micros(void){
    // 1ms-granular implementation: returns millis() * 1000.
    return millis() * 1000u;
}

// ─── analogRead: ADC1 single conversion ───────────────────────
//
// Arduino mapping (STM32F103):
//   A0=PA0/ch0  A1=PA1/ch1  A2=PA2/ch2  A3=PA3/ch3
//   A4=PA4/ch4  A5=PA5/ch5  A6=PA6/ch6  A7=PA7/ch7
//   A8=PB0/ch8  A9=PB1/ch9
// Returns 0..4095 (12-bit), but Arduino API wants 0..1023, so we
// shift right by 2 to match. User can divide further if needed.
//
#define ADC1_BASE  0x40012400u
#define ADC_SR     _REG(ADC1_BASE + 0x00)
#define ADC_CR1    _REG(ADC1_BASE + 0x04)
#define ADC_CR2    _REG(ADC1_BASE + 0x08)
#define ADC_SQR3   _REG(ADC1_BASE + 0x34)
#define ADC_DR     _REG(ADC1_BASE + 0x4C)
#define ADC_SR_EOC   (1u << 1)
#define ADC_CR2_ADON (1u << 0)
#define ADC_CR2_SWSTART (1u << 22)
#define ADC_CR2_EXTSEL_SWSTART (7u << 17)
#define ADC_CR2_EXTTRIG (1u << 20)

static int _adc_inited = 0;
static int8_t _pin_to_adc_channel(uint8_t pin){
    if (pin <= 7)  return pin;          // PA0..PA7 → ch0..ch7
    if (pin == PB0) return 8;
    if (pin == PB1) return 9;
    return -1;
}
static void _adc_init_once(void){
    if (_adc_inited) return;
    // Enable ADC1 clock (RCC_APB2ENR bit 9)
    RCC_APB2ENR |= (1u << 9);
    // Enable ADC (CR2 ADON). Real HW needs ~1us; emulator is OK immediately.
    ADC_CR2 = ADC_CR2_ADON;
    // Single conversion: regular channels SQR1=0 (1 conversion)
    // SWSTART as external trigger
    ADC_CR2 = ADC_CR2_ADON | ADC_CR2_EXTSEL_SWSTART | ADC_CR2_EXTTRIG;
    _adc_inited = 1;
}
uint16_t analogRead(uint8_t pin){
    int8_t ch = _pin_to_adc_channel(pin);
    if (ch < 0) return 0;
    _adc_init_once();
    // Configure the pin as analog input: MODE=00, CNF=00 in CRL/CRH
    if (pin <= 7) {
        RCC_APB2ENR |= (1u << 2); // IOPAEN
        uint32_t shift = pin * 4u;
        uint32_t v = GPIOA_CRL;
        v &= ~(0xFu << shift);
        GPIOA_CRL = v;
    } else if (pin == PB0 || pin == PB1) {
        RCC_APB2ENR |= (1u << 3); // IOPBEN
        uint32_t b = (pin == PB0) ? 0 : 1;
        uint32_t shift = b * 4u;
        uint32_t v = GPIOB_CRL;
        v &= ~(0xFu << shift);
        GPIOB_CRL = v;
    }
    // Select channel as 1st (and only) conversion in regular sequence
    ADC_SQR3 = (uint32_t)ch & 0x1Fu;
    // Clear EOC, start conversion
    ADC_SR = 0;
    ADC_CR2 |= ADC_CR2_SWSTART;
    // Wait for EOC
    while (!(ADC_SR & ADC_SR_EOC)){}
    uint16_t raw = (uint16_t)(ADC_DR & 0x0FFFu);  // 12-bit
    // Arduino classic returns 0..1023, so shift to 10-bit
    return raw >> 2;
}

// ─── analogWrite: PWM via TIM2/TIM3 ──────────────────────────────
//
// Arduino mapping (STM32F103 default AF):
//   PA0 → TIM2_CH1   PA1 → TIM2_CH2   PA2 → TIM2_CH3   PA3 → TIM2_CH4
//   PA6 → TIM3_CH1   PA7 → TIM3_CH2
//   PB0 → TIM3_CH3   PB1 → TIM3_CH4
// 8-bit resolution: ARR = 255. Duty cycle = value / 255.
//
#define TIM2_BASE  0x40000000u
#define TIM3_BASE  0x40000400u
#define TIM_CR1(b)    _REG((b) + 0x00)
#define TIM_CCMR1(b)  _REG((b) + 0x18)
#define TIM_CCMR2(b)  _REG((b) + 0x1C)
#define TIM_CCER(b)   _REG((b) + 0x20)
#define TIM_PSC(b)    _REG((b) + 0x28)
#define TIM_ARR(b)    _REG((b) + 0x2C)
#define TIM_CCR1(b)   _REG((b) + 0x34)
#define TIM_CCR2(b)   _REG((b) + 0x38)
#define TIM_CCR3(b)   _REG((b) + 0x3C)
#define TIM_CCR4(b)   _REG((b) + 0x40)
#define TIM_EGR(b)    _REG((b) + 0x14)

static uint8_t _pwm_initialized = 0;  // bitmask: bit0=TIM2, bit1=TIM3

static void _pwm_init_tim(uint32_t base, uint8_t bitmask_idx){
    if (_pwm_initialized & (1u << bitmask_idx)) return;
    // Enable timer clock (RCC_APB1ENR)
    if (base == TIM2_BASE) RCC_APB1ENR |= (1u << 0);
    else                   RCC_APB1ENR |= (1u << 1);
    TIM_PSC(base) = 0;                // no prescaler
    TIM_ARR(base) = 255;              // 8-bit Arduino-classic
    // PWM mode 1 on all 4 channels: CCMR1.OC1M=110, OC2M=110; CCMR2.OC3M, OC4M
    TIM_CCMR1(base) = (6u << 4) | (6u << 12);
    TIM_CCMR2(base) = (6u << 4) | (6u << 12);
    // Enable outputs CCxE for all 4
    TIM_CCER(base) = (1u << 0) | (1u << 4) | (1u << 8) | (1u << 12);
    // Generate update event to load PSC/ARR/CCMR
    TIM_EGR(base) = 1;
    // Enable counter
    TIM_CR1(base) = 1;
    _pwm_initialized |= (1u << bitmask_idx);
}

void analogWrite(uint8_t pin, uint8_t value){
    // Determine timer + channel + GPIO port/pin
    uint32_t base = 0;
    uint8_t  bit  = 0;   // tim bitmask index
    uint8_t  ch   = 0;   // 1..4
    uint8_t  gpio_pin = 0;
    int isPortB = 0;
    switch (pin) {
        case PA0: base = TIM2_BASE; bit = 0; ch = 1; gpio_pin = 0; break;
        case PA1: base = TIM2_BASE; bit = 0; ch = 2; gpio_pin = 1; break;
        case PA2: base = TIM2_BASE; bit = 0; ch = 3; gpio_pin = 2; break;
        case PA3: base = TIM2_BASE; bit = 0; ch = 4; gpio_pin = 3; break;
        case PA6: base = TIM3_BASE; bit = 1; ch = 1; gpio_pin = 6; break;
        case PA7: base = TIM3_BASE; bit = 1; ch = 2; gpio_pin = 7; break;
        case PB0: base = TIM3_BASE; bit = 1; ch = 3; gpio_pin = 0; isPortB = 1; break;
        case PB1: base = TIM3_BASE; bit = 1; ch = 4; gpio_pin = 1; isPortB = 1; break;
        default:
            // Pin not PWM-capable: fall back to digitalWrite
            digitalWrite(pin, value > 127 ? HIGH : LOW);
            return;
    }
    // Configure GPIO pin as alternate function push-pull, 50MHz: MODE=11, CNF=10
    uint32_t cfg = 0xBu;  // 1011 = AF push-pull, 50 MHz
    if (isPortB) {
        RCC_APB2ENR |= (1u << 3);  // IOPBEN
        uint32_t shift = gpio_pin * 4u;
        uint32_t v = GPIOB_CRL;
        v &= ~(0xFu << shift);
        v |=  (cfg  << shift);
        GPIOB_CRL = v;
    } else {
        RCC_APB2ENR |= (1u << 2);  // IOPAEN
        uint32_t shift = gpio_pin * 4u;
        uint32_t v = GPIOA_CRL;
        v &= ~(0xFu << shift);
        v |=  (cfg  << shift);
        GPIOA_CRL = v;
    }
    _pwm_init_tim(base, bit);
    // Set duty: CCRx = value (range 0..255 → 0..255/255 = 0..100% duty)
    switch (ch) {
        case 1: TIM_CCR1(base) = value; break;
        case 2: TIM_CCR2(base) = value; break;
        case 3: TIM_CCR3(base) = value; break;
        case 4: TIM_CCR4(base) = value; break;
    }
}

// ─── attachInterrupt: EXTI lines ─────────────────────────────────
//
// Each GPIO pin number maps to EXTI line N (regardless of port).
// E.g. PA3, PB3, PC3 all share EXTI line 3 — AFIO_EXTICR selects
// which port drives the line.
//
// We store user ISRs in a 16-slot table indexed by EXTI line.
// The EXTIx_IRQHandler ISRs dispatch to the user callback.
//
#define AFIO_EXTICR1  _REG(0x40010008)
#define AFIO_EXTICR2  _REG(0x4001000C)
#define AFIO_EXTICR3  _REG(0x40010010)
#define AFIO_EXTICR4  _REG(0x40010014)
#define EXTI_IMR      _REG(0x40010400)
#define EXTI_RTSR     _REG(0x40010408)
#define EXTI_FTSR     _REG(0x4001040C)
#define EXTI_PR       _REG(0x40010414)
#define NVIC_ISER0    _REG(0xE000E100)
#define NVIC_ISER1    _REG(0xE000E104)

static ISR_t _user_isr[16] = {0};

static void _exti_set_port(uint8_t line, uint8_t portIdx){
    // EXTICR is 4 registers × 4 nibbles each, line 0..15
    uint32_t regIdx = line / 4u;
    uint32_t shift  = (line % 4u) * 4u;
    volatile uint32_t* regs[4] = {
        &AFIO_EXTICR1, &AFIO_EXTICR2, &AFIO_EXTICR3, &AFIO_EXTICR4
    };
    uint32_t v = *regs[regIdx];
    v &= ~(0xFu << shift);
    v |=  ((uint32_t)portIdx << shift);
    *regs[regIdx] = v;
}

static void _nvic_enable(uint8_t irq){
    if (irq < 32) NVIC_ISER0 = (1u << irq);
    else          NVIC_ISER1 = (1u << (irq - 32));
}

void attachInterrupt(uint8_t pin, ISR_t isr, uint8_t mode){
    // Determine GPIO port index from pin number
    uint8_t portIdx;
    uint8_t line;
    if      (pin <= 15) { portIdx = 0; line = pin; }
    else if (pin <= 31) { portIdx = 1; line = pin - 16; }
    else if (pin <= 47) { portIdx = 2; line = pin - 32; }
    else return;
    // Configure pin as input pull-up by default (safe choice)
    pinMode(pin, INPUT_PULLUP);
    // Enable AFIO clock (RCC_APB2ENR bit 0)
    RCC_APB2ENR |= (1u << 0);
    // Route EXTI line to chosen port
    _exti_set_port(line, portIdx);
    // Configure edge trigger
    uint32_t mask = (1u << line);
    if (mode == RISING || mode == CHANGE) EXTI_RTSR |= mask; else EXTI_RTSR &= ~mask;
    if (mode == FALLING || mode == CHANGE) EXTI_FTSR |= mask; else EXTI_FTSR &= ~mask;
    // Unmask interrupt
    EXTI_IMR |= mask;
    // Save user ISR
    _user_isr[line] = isr;
    // Enable NVIC IRQ for this line.
    // STM32F103: EXTI0=6, EXTI1=7, EXTI2=8, EXTI3=9, EXTI4=10,
    //            EXTI9_5=23, EXTI15_10=40
    uint8_t irq;
    if      (line <= 4)  irq = 6 + line;
    else if (line <= 9)  irq = 23;
    else                 irq = 40;
    _nvic_enable(irq);
}

void detachInterrupt(uint8_t pin){
    uint8_t line;
    if      (pin <= 15) line = pin;
    else if (pin <= 31) line = pin - 16;
    else if (pin <= 47) line = pin - 32;
    else return;
    EXTI_IMR &= ~(1u << line);
    _user_isr[line] = 0;
}

// ── EXTI ISR dispatchers ────────────────────────────────────────
// These are the real STM32 ISR names; they override the weak defaults
// in startup.c. Each clears the EXTI pending bit then calls the user
// callback installed via attachInterrupt.
//
static void _exti_dispatch(uint8_t line){
    EXTI_PR = (1u << line);  // write 1 to clear pending
    if (_user_isr[line]) _user_isr[line]();
}
void EXTI0_IRQHandler(void)   { _exti_dispatch(0); }
void EXTI1_IRQHandler(void)   { _exti_dispatch(1); }
void EXTI2_IRQHandler(void)   { _exti_dispatch(2); }
void EXTI3_IRQHandler(void)   { _exti_dispatch(3); }
void EXTI4_IRQHandler(void)   { _exti_dispatch(4); }
void EXTI9_5_IRQHandler(void) {
    for (uint8_t l = 5; l <= 9; ++l)
        if (EXTI_PR & (1u << l)) _exti_dispatch(l);
}
void EXTI15_10_IRQHandler(void) {
    for (uint8_t l = 10; l <= 15; ++l)
        if (EXTI_PR & (1u << l)) _exti_dispatch(l);
}

#define SR_TXE  (1u << 7)
#define SR_RXNE (1u << 5)
#define CR1_UE  (1u << 13)
#define CR1_TE  (1u << 3)
#define CR1_RE  (1u << 2)

static void _usart_begin(uint32_t base, uint32_t baud){
    if (base == USART1_BASE){ RCC_APB2ENR |= (1u<<14); RCC_APB2ENR |= (1u<<2); }
    else if (base == USART2_BASE){ RCC_APB1ENR |= (1u<<17); RCC_APB2ENR |= (1u<<2); }
    USART_BRR(base) = (baud > 0) ? (8000000u/baud) : 0x208u;
    USART_CR1(base) = CR1_UE | CR1_TE | CR1_RE;
}
static void _usart_write(uint32_t base, uint8_t b){
    while (!(USART_SR(base) & SR_TXE)){}
    USART_DR(base) = b;
}
static int _usart_available(uint32_t base){ return (USART_SR(base) & SR_RXNE) ? 1 : 0; }
static int _usart_read(uint32_t base){
    if (!(USART_SR(base) & SR_RXNE)) return -1;
    return (int)(USART_DR(base) & 0xFFu);
}
static void _usart_print(uint32_t base, const char* s){
    if (!s) return;
    while (*s) _usart_write(base, (uint8_t)*s++);
}
static void _usart_println(uint32_t base, const char* s){
    _usart_print(base, s); _usart_write(base,'\r'); _usart_write(base,'\n');
}
static void _itoa10(int32_t v, char* buf){
    char tmp[12]; int n = 0; int neg = (v < 0);
    uint32_t u = (uint32_t)(neg ? -v : v);
    if (u == 0) tmp[n++] = '0';
    while (u){ tmp[n++] = '0' + (u % 10); u /= 10; }
    int p = 0;
    if (neg) buf[p++] = '-';
    while (n--) buf[p++] = tmp[n];
    buf[p] = 0;
}
static void _usart_printInt(uint32_t base, int32_t v){
    char buf[12]; _itoa10(v, buf); _usart_print(base, buf);
}
static void _usart_printlnInt(uint32_t base, int32_t v){
    _usart_printInt(base, v); _usart_write(base,'\r'); _usart_write(base,'\n');
}

static void s1_begin(uint32_t b){ _usart_begin(USART1_BASE,b); }
static void s1_end(void){ USART_CR1(USART1_BASE) = 0; }
static int  s1_avail(void){ return _usart_available(USART1_BASE); }
static int  s1_read(void){ return _usart_read(USART1_BASE); }
static void s1_write(uint8_t b){ _usart_write(USART1_BASE,b); }
static void s1_print(const char* s){ _usart_print(USART1_BASE,s); }
static void s1_println(const char* s){ _usart_println(USART1_BASE,s); }
static void s1_pi(int32_t v){ _usart_printInt(USART1_BASE,v); }
static void s1_pli(int32_t v){ _usart_printlnInt(USART1_BASE,v); }

static void s2_begin(uint32_t b){ _usart_begin(USART2_BASE,b); }
static void s2_end(void){ USART_CR1(USART2_BASE) = 0; }
static int  s2_avail(void){ return _usart_available(USART2_BASE); }
static int  s2_read(void){ return _usart_read(USART2_BASE); }
static void s2_write(uint8_t b){ _usart_write(USART2_BASE,b); }
static void s2_print(const char* s){ _usart_print(USART2_BASE,s); }
static void s2_println(const char* s){ _usart_println(USART2_BASE,s); }
static void s2_pi(int32_t v){ _usart_printInt(USART2_BASE,v); }
static void s2_pli(int32_t v){ _usart_printlnInt(USART2_BASE,v); }

SerialPort Serial = {
    s1_begin, s1_end, s1_avail, s1_read, s1_write,
    s1_print, s1_println, s1_pi, s1_pli
};
SerialPort Serial2 = {
    s2_begin, s2_end, s2_avail, s2_read, s2_write,
    s2_print, s2_println, s2_pi, s2_pli
};
