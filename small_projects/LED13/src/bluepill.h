// =============================================================================
// bluepill.h — Arduino-style API for STM32F103 Blue Pill
// Version: v2 (2026-05-12)
// =============================================================================
//
// Quick example:
//
//     #include "bluepill.h"
//     int main(void) {
//         pinMode(PC13, OUTPUT);
//         while (1) {
//             digitalWrite(PC13, HIGH); delay(1000);
//             digitalWrite(PC13, LOW);  delay(1000);
//         }
//     }
//
// Supported features:
//   - pinMode, digitalWrite, digitalRead
//   - delay, delayMicroseconds (1ms resolution), millis, micros
//   - analogRead (10-bit ADC, channels A0..A9)
//   - analogWrite (8-bit PWM on PA0..PA3, PA6..PA7, PB0..PB1)
//   - attachInterrupt / detachInterrupt (EXTI lines)
//   - Serial / Serial2 (UART)
//
// All functions use STM32 standard registers — code compiles and runs
// unchanged on real Blue Pill hardware.
// =============================================================================
#ifndef BLUEPILL_H
#define BLUEPILL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BP_PIN(port, pin) ((uint8_t)(((port) << 4) | ((pin) & 0x0F)))

// Port A
#define PA0 BP_PIN(0,0)
#define PA1 BP_PIN(0,1)
#define PA2 BP_PIN(0,2)
#define PA3 BP_PIN(0,3)
#define PA4 BP_PIN(0,4)
#define PA5 BP_PIN(0,5)
#define PA6 BP_PIN(0,6)
#define PA7 BP_PIN(0,7)
#define PA8 BP_PIN(0,8)
#define PA9 BP_PIN(0,9)
#define PA10 BP_PIN(0,10)
#define PA11 BP_PIN(0,11)
#define PA12 BP_PIN(0,12)
#define PA13 BP_PIN(0,13)
#define PA14 BP_PIN(0,14)
#define PA15 BP_PIN(0,15)

// Port B
#define PB0 BP_PIN(1,0)
#define PB1 BP_PIN(1,1)
#define PB2 BP_PIN(1,2)
#define PB3 BP_PIN(1,3)
#define PB4 BP_PIN(1,4)
#define PB5 BP_PIN(1,5)
#define PB6 BP_PIN(1,6)
#define PB7 BP_PIN(1,7)
#define PB8 BP_PIN(1,8)
#define PB9 BP_PIN(1,9)
#define PB10 BP_PIN(1,10)
#define PB11 BP_PIN(1,11)
#define PB12 BP_PIN(1,12)
#define PB13 BP_PIN(1,13)
#define PB14 BP_PIN(1,14)
#define PB15 BP_PIN(1,15)

// Port C — PC13 is the on-board LED on Blue Pill
#define PC0 BP_PIN(2,0)
#define PC1 BP_PIN(2,1)
#define PC2 BP_PIN(2,2)
#define PC3 BP_PIN(2,3)
#define PC4 BP_PIN(2,4)
#define PC5 BP_PIN(2,5)
#define PC6 BP_PIN(2,6)
#define PC7 BP_PIN(2,7)
#define PC8 BP_PIN(2,8)
#define PC9 BP_PIN(2,9)
#define PC10 BP_PIN(2,10)
#define PC11 BP_PIN(2,11)
#define PC12 BP_PIN(2,12)
#define PC13 BP_PIN(2,13)
#define PC14 BP_PIN(2,14)
#define PC15 BP_PIN(2,15)

#define LED_BUILTIN PC13

typedef enum {
    INPUT          = 0,
    OUTPUT         = 1,
    INPUT_PULLUP   = 2,
    INPUT_PULLDOWN = 3,
    OUTPUT_OD      = 4
} PinMode;

#define LOW  0
#define HIGH 1

// Digital I/O
void pinMode(uint8_t pin, PinMode mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);

// Timing
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
uint32_t millis(void);   // milliseconds since first delay/millis call
uint32_t micros(void);   // microseconds since first delay/millis call (1ms granularity)

// Analog (10-bit ADC, 8-bit PWM — Arduino classic)
uint16_t analogRead(uint8_t pin);            // returns 0..1023
void     analogWrite(uint8_t pin, uint8_t value);  // value 0..255

// External interrupts (Arduino-style)
#define RISING   1
#define FALLING  2
#define CHANGE   3
#define LOW_LVL  4   // 'LOW' is taken by digital level
typedef void (*ISR_t)(void);
void attachInterrupt(uint8_t pin, ISR_t isr, uint8_t mode);
void detachInterrupt(uint8_t pin);

// Serial (USART1 on PA9/PA10, USART2 on PA2/PA3)
typedef struct {
    void (*begin)(uint32_t baud);
    void (*end)(void);
    int  (*available)(void);
    int  (*read)(void);
    void (*write)(uint8_t b);
    void (*print)(const char* s);
    void (*println)(const char* s);
    void (*printInt)(int32_t v);
    void (*printlnInt)(int32_t v);
} SerialPort;
extern SerialPort Serial;
extern SerialPort Serial2;

// Bit / math helpers
#define bit(b)             (1UL << (b))
#define bitRead(v,b)       (((v) >> (b)) & 1)
#define bitSet(v,b)        ((v) |=  (1UL << (b)))
#define bitClear(v,b)      ((v) &= ~(1UL << (b)))
#define bitWrite(v,b,x)    ((x) ? bitSet(v,b) : bitClear(v,b))
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#define constrain(x,lo,hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))
#define map(x,iL,iH,oL,oH) (((x)-(iL))*((oH)-(oL))/((iH)-(iL))+(oL))

#ifdef __cplusplus
}
#endif
#endif // BLUEPILL_H
