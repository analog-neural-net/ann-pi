#include <cstdint>

#ifndef GPIO_H
#define GPIO_H  

typedef enum {
    INPUT,
    OUTPUT,
    ALT5,
    ALT4,
    ALT0,
    ALT1,
    ALT2,
    ALT3
} FUNCTION;

#define GPFSEL0                      GPIO_BASE_ADDR + GPFSEL0_OFFSET
#define GPFSEL1                      GPIO_BASE_ADDR + GPFSEL1_OFFSET
#define GPFSEL2                      GPIO_BASE_ADDR + GPFSEL2_OFFSET
#define GPFSEL3                      GPIO_BASE_ADDR + GPFSEL3_OFFSET
#define GPFSEL4                      GPIO_BASE_ADDR + GPFSEL4_OFFSET
#define GPFSEL5                      GPIO_BASE_ADDR + GPFSEL5_OFFSET
#define GPSET0                       GPIO_BASE_ADDR + GPSET0_OFFSET
#define GPSET1                       GPIO_BASE_ADDR + GPSET1_OFFSET
#define GPCLR0                       GPIO_BASE_ADDR + GPCLR0_OFFSET
#define GPCLR1                       GPIO_BASE_ADDR + GPCLR1_OFFSET
#define GPLEV0                       GPIO_BASE_ADDR + GPLEV0_OFFSET
#define GPLEV1                       GPIO_BASE_ADDR + GPLEV1_OFFSET
#define GPEDS0                       GPIO_BASE_ADDR + GPEDS0_OFFSET
#define GPEDS1                       GPIO_BASE_ADDR + GPEDS1_OFFSET
#define GPREN0                       GPIO_BASE_ADDR + GPREN0_OFFSET
#define GPREN1                       GPIO_BASE_ADDR + GPREN1_OFFSET
#define GPFEN0                       GPIO_BASE_ADDR + GPFEN0_OFFSET
#define GPFEN1                       GPIO_BASE_ADDR + GPFEN1_OFFSET
#define GPHEN0                       GPIO_BASE_ADDR + GPHEN0_OFFSET
#define GPHEN1                       GPIO_BASE_ADDR + GPHEN1_OFFSET
#define GPLEN0                       GPIO_BASE_ADDR + GPLEN0_OFFSET
#define GPLEN1                       GPIO_BASE_ADDR + GPLEN1_OFFSET
#define GPAREN0                      GPIO_BASE_ADDR + GPAREN0_OFFSET
#define GPAREN1                      GPIO_BASE_ADDR + GPAREN1_OFFSET
#define GPAFEN0                      GPIO_BASE_ADDR + GPAFEN0_OFFSET
#define GPAFEN1                      GPIO_BASE_ADDR + GPAFEN1_OFFSET
#define GPIO_PUP_PDN_CNTRL_REG0      GPIO_BASE_ADDR + GPIO_PUP_PDN_CNTRL_REG0_OFFSET
#define GPIO_PUP_PDN_CNTRL_REG1      GPIO_BASE_ADDR + GPIO_PUP_PDN_CNTRL_REG1_OFFSET
#define GPIO_PUP_PDN_CNTRL_REG2      GPIO_BASE_ADDR + GPIO_PUP_PDN_CNTRL_REG2_OFFSET
#define GPIO_PUP_PDN_CNTRL_REG3      GPIO_BASE_ADDR + GPIO_PUP_PDN_CNTRL_REG3_OFFSET

#define GPIO_BASE_ADDR                      0x7E200000u
#define GPFSEL0_OFFSET                      0x00000000u
#define GPFSEL1_OFFSET                      0x00000004u
#define GPFSEL2_OFFSET                      0x00000008u
#define GPFSEL3_OFFSET                      0x0000000Cu
#define GPFSEL4_OFFSET                      0x00000010u
#define GPFSEL5_OFFSET                      0x00000014u
#define GPSET0_OFFSET                       0x0000001Cu
#define GPSET1_OFFSET                       0x00000020u
#define GPCLR0_OFFSET                       0x00000028u
#define GPCLR1_OFFSET                       0x0000002Cu
#define GPLEV0_OFFSET                       0x00000034u
#define GPLEV1_OFFSET                       0x00000038u
#define GPEDS0_OFFSET                       0x00000040u
#define GPEDS1_OFFSET                       0x00000044u
#define GPREN0_OFFSET                       0x0000004Cu
#define GPREN1_OFFSET                       0x00000050u
#define GPFEN0_OFFSET                       0x00000058u
#define GPFEN1_OFFSET                       0x0000005Cu
#define GPHEN0_OFFSET                       0x00000064u
#define GPHEN1_OFFSET                       0x00000068u
#define GPLEN0_OFFSET                       0x00000070u
#define GPLEN1_OFFSET                       0x00000074u
#define GPAREN0_OFFSET                      0x0000007Cu
#define GPAREN1_OFFSET                      0x00000080u
#define GPAFEN0_OFFSET                      0x00000088u
#define GPAFEN1_OFFSET                      0x0000008Cu
#define GPIO_PUP_PDN_CNTRL_REG0_OFFSET      0x000000E4u
#define GPIO_PUP_PDN_CNTRL_REG1_OFFSET      0x000000E8u
#define GPIO_PUP_PDN_CNTRL_REG2_OFFSET      0x000000ECu
#define GPIO_PUP_PDN_CNTRL_REG3_OFFSET      0x000000F0u

void __gpio_setbit(uint8_t pin, uint32_t* reg){
    *reg |= 1 << pin;
}

void __gpio_clearbit(uint8_t pin, uint32_t* reg){
    *reg &= ~(1 << pin);
}

void __gpio_togglebit(uint8_t pin, uint32_t* reg){
    *reg ^= 1 << pin;
}

void gpio_func_select(FUNCTION func, uint8_t pin){
    uint8_t reg_num = pin/10;
    uint8_t pin_pos = pin%10;

    if (reg_num == 0){
        *((uint32_t*)GPFSEL0) |= (func << 3*pin_pos); 
    }
    if (reg_num == 1){
        *((uint32_t*)GPFSEL1) |= (func << 3*pin_pos); 
    }
    if (reg_num == 2){
        *((uint32_t*)GPFSEL2) |= (func << 3*pin_pos); 
    }
    if (reg_num == 3){
        *((uint32_t*)GPFSEL3) |= (func << 3*pin_pos); 
    }
    if (reg_num == 4){
        *((uint32_t*)GPFSEL4) |= (func << 3*pin_pos); 
    }
    if (reg_num == 5){
        *((uint32_t*)GPFSEL5) |= (func << 3*pin_pos); 
    }
}




#endif

