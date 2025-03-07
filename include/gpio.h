#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef GPIO_H
#define GPIO_H  

#define GPIO_BASE_ADDR  0xFE200000 
#define BLOCK_SIZE      4096

// Function Select Registers (GPIO Function Select)
#define GPFSEL0_OFFSET  0x0000
#define GPFSEL1_OFFSET  0x0004
#define GPFSEL2_OFFSET  0x0008
#define GPFSEL3_OFFSET  0x000C
#define GPFSEL4_OFFSET  0x0010
#define GPFSEL5_OFFSET  0x0014

// Set/Clear Registers
#define GPSET0_OFFSET   0x001C
#define GPSET1_OFFSET   0x0020
#define GPCLR0_OFFSET   0x0028
#define GPCLR1_OFFSET   0x002C

// Read Registers
#define GPLEV0_OFFSET   0x0034
#define GPLEV1_OFFSET   0x0038

// Pull-up/down Control (Pi 4 and newer)
#define GPIO_PUP_PDN_CNTRL_REG0_OFFSET  0x00E4
#define GPIO_PUP_PDN_CNTRL_REG1_OFFSET  0x00E8
#define GPIO_PUP_PDN_CNTRL_REG2_OFFSET  0x00EC
#define GPIO_PUP_PDN_CNTRL_REG3_OFFSET  0x00F0

typedef enum {
    INPUT  = 0,
    OUTPUT = 1,
    ALT5   = 2,
    ALT4   = 3,
    ALT0   = 4,
    ALT1   = 5,
    ALT2   = 6,
    ALT3   = 7
} FUNCTION;

volatile uint32_t *gpio_base; 

// Initialize GPIO by memory mapping
void gpio_init() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/mem. Try running with sudo.");
        exit(1);
    }

    gpio_base = (volatile uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_ADDR);
    close(fd);

    if (gpio_base == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
}

// Set GPIO pin function
void gpio_func_select(FUNCTION func, uint8_t pin) {
    uint8_t reg_num = pin / 10;
    uint8_t pin_pos = (pin % 10) * 3;
    volatile uint32_t *gpfsel = gpio_base + (GPFSEL0_OFFSET / 4) + reg_num;

    *gpfsel &= ~(7 << pin_pos);
    *gpfsel |= (func << pin_pos); 
}

// Set GPIO pin HIGH
void gpio_set(uint8_t pin) {
    *(gpio_base + (GPSET0_OFFSET / 4) + (pin / 32)) = (1 << (pin % 32));
}

// Set GPIO pin LOW
void gpio_clear(uint8_t pin) {
    *(gpio_base + (GPCLR0_OFFSET / 4) + (pin / 32)) = (1 << (pin % 32));
}

// Read GPIO pin level
uint8_t gpio_read(uint8_t pin) {
    return (*(gpio_base + (GPLEV0_OFFSET / 4) + (pin / 32)) & (1 << (pin % 32))) ? 1 : 0;
}

#endif
