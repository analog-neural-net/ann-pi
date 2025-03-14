#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <cstdlib>
#include <vector>

// UART Base Address
#define UART0_BASE  0xFE201000
#define UART3_OFFSET 0x600
#define BLOCK_SIZE  4096

// UART Register Offsets
#define UART_DR     0x00  // Data Register -> read/write data from/to uart
#define UART_FR     0x18  // Flag Register -> indicates data status
#define UART_IBRD   0x24  // Integer Baud Rate Divisor -> used to set baud rate
#define UART_FBRD   0x28  // Fractional Baud Rate Divisor -> used to set baud rate
#define UART_LCRH   0x2C  // Line Control Register -> configures data format and other things fr
#define UART_CR     0x30  // Control Register -> uart enable

volatile uint32_t* uart_base;

void uart_init() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        std::cerr << "Failed to open /dev/mem. Try running with sudo." << std::endl;
        exit(1);
    }

    uart_base = (volatile uint32_t*) mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, UART0_BASE);
    close(fd);

    if (uart_base == MAP_FAILED) {
        std::cerr << "Memory mapping failed." << std::endl;
        exit(1);
    }

    uart_base = (volatile uint32_t*)((char*)uart_base + UART3_OFFSET);

    // Configure UART
    uart_base[UART_CR / 4] = 0;  
    
    // this flushes the rx and tx fifo
    uart_base[UART_LCRH / 4] |= (1 << 4);
    uart_base[UART_LCRH / 4] &= ~(1 << 4);
    
    // this sets the baud rate
    uart_base[UART_IBRD / 4] = 26; 
    uart_base[UART_FBRD / 4] = 3;
    
    // word is 8-hit + enables fifo  
    uart_base[UART_LCRH / 4] = (3 << 5) | (1 << 4);
    
    // master enable
    uart_base[UART_CR / 4] = (1 << 0) | (1 << 8) | (1 << 9);
    
    /*
    
    while (!(uart_base[UART_FR / 4] & (1 << 4))){
        volatile uint32_t dummy = uart_base[UART_DR / 4];
        (void)dummy;
        
        std::cout << "destroyed\n";
    } 
    */   
}

void uart_send_char(char c) {
    while (uart_base[UART_FR / 4] & (1 << 5)) {} // wait for TX ready
    uart_base[UART_DR / 4] = c;
}

void uart_send_int32(int32_t int32) {
    uint8_t bytes[4]; 

    // convert int32 to byte array 
    bytes[0] = (int32 >> 0)  & 0xFF;
    bytes[1] = (int32 >> 8)  & 0xFF;
    bytes[2] = (int32 >> 16) & 0xFF;
    bytes[3] = (int32 >> 24) & 0xFF;

    // send each byte over UART
    for (int i = 0; i < 4; i++) {
        while (uart_base[UART_FR / 4] & (1 << 5)) {} // wait for TX ready
        uart_base[UART_DR / 4] = bytes[i];
    }
}

void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

char uart_receive_char() {
    
    while (uart_base[UART_FR / 4] & (1 << 4)) {} // wait for RX ready
    return uart_base[UART_DR / 4] & 0xFF;
}

void uart_receive_string(int length, char* buf) {
    
    int i = 0;
    
    while(i < length){
        buf[i] = uart_receive_char();
        i++;
        std::cout << buf[i] << '\n';
    }
    buf[i] = '\0';
    
}

void uart_send_pca_data(std::vector<int32_t> &pca_projection) {
    
    uart_send_string("ANN-E");
    
    for (int i = 0; i < pca_projection.size(); i++) {
        uart_send_int32(pca_projection[i]);
    } 
     
}
#endif // UART_DRIVER_H
