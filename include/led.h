// WS2811 LED Ring Control for Raspberry Pi 4B (C++)
// For a 6-unit LED ring
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ws2811.h>

// LED strip configuration
#define LED_COUNT      8      // Number of LED pixels in your ring
#define LED_PIN        18     // GPIO pin connected to the pixels (18 uses PWM)
#define LED_FREQ_HZ    800000 // LED signal frequency in hertz (usually 800khz)
#define LED_DMA        10     // DMA channel to use for generating signal
#define LED_BRIGHTNESS 100    // Set to 0 for darkest and 255 for brightest
#define LED_CHANNEL    0      // PWM channel to use
#define LED_INVERT     0      // Invert the signal (when using NPN transistor level shift)

// Color definitions
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_WHITE     0x00FFE6E6
#define COLOR_PURPLE    0x008000FF
#define COLOR_YELLOW    0x00FFFF00
#define COLOR_CYAN      0x0000FFFF
#define COLOR_BLACK     0x00000000

extern ws2811_t ledstring;

// Function forward declarations
void colorWipe(uint32_t color, int wait_ms);
void theaterChase(uint32_t color, int wait_ms, int iterations);
void rainbow(int wait_ms, int iterations);
void rainbowCycle(int wait_ms, int iterations);
void solidColor(uint32_t color);
void pulse(uint32_t color, int cycles, int wait_ms);
uint32_t wheel(uint8_t pos);
void setup_ws2811(void);
