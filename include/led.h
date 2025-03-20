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
#define LED_COUNT      6      // Number of LED pixels in your ring
#define LED_PIN        18     // GPIO pin connected to the pixels (18 uses PWM)
#define LED_FREQ_HZ    800000 // LED signal frequency in hertz (usually 800khz)
#define LED_DMA        10     // DMA channel to use for generating signal
#define LED_BRIGHTNESS 128    // Set to 0 for darkest and 255 for brightest
#define LED_CHANNEL    0      // PWM channel to use
#define LED_INVERT     0      // Invert the signal (when using NPN transistor level shift)

// Color definitions
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_PURPLE    0x008000FF
#define COLOR_YELLOW    0x00FFFF00
#define COLOR_CYAN      0x0000FFFF
#define COLOR_BLACK     0x00000000

ws2811_t ledstring = {
    .freq = LED_FREQ_HZ,
    .dmanum = LED_DMA,
    .channel = {
        [0] = {
            .gpionum = LED_PIN,
            .invert = LED_INVERT,
            .count = LED_COUNT,
            .strip_type = WS2811_STRIP_GRB,
            .brightness = LED_BRIGHTNESS,
        },
        [1] = {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

// Global signal handler variable
static volatile int running = 1;

// Function forward declarations
void colorWipe(uint32_t color, int wait_ms);
void theaterChase(uint32_t color, int wait_ms, int iterations);
void rainbow(int wait_ms, int iterations);
void rainbowCycle(int wait_ms, int iterations);
void solidColor(uint32_t color);
void pulse(uint32_t color, int cycles, int wait_ms);
static uint32_t wheel(uint8_t pos);
static void ctrl_c_handler(int signum);
static void setup_handlers(void);

// Handle Ctrl-C gracefully
static void ctrl_c_handler(int signum) {
    (void)(signum);
    running = 0;
}

// Setup signal handlers
static void setup_handlers(void) {
    struct sigaction sa = {
        .sa_handler = ctrl_c_handler,
    };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

// Main program
int main() {
    ws2811_return_t ret;

    // Setup handlers for termination signals
    setup_handlers();

    // Initialize LED strip
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS) {
        std::cerr << "ws2811_init failed: " << ws2811_get_return_t_str(ret) << std::endl;
        return ret;
    }

    std::cout << "Press Ctrl-C to quit." << std::endl;

    // Main loop
    while (running) {
        std::cout << "Color wipe animations." << std::endl;
        colorWipe(COLOR_RED, 50);     // Red wipe
        if (!running) break;
        colorWipe(COLOR_GREEN, 50);   // Green wipe
        if (!running) break;
        colorWipe(COLOR_BLUE, 50);    // Blue wipe
        if (!running) break;
        
        std::cout << "Theater chase animations." << std::endl;
        theaterChase(COLOR_WHITE, 50, 10);   // White theater chase
        if (!running) break;
        theaterChase(COLOR_RED, 50, 10);     // Red theater chase
        if (!running) break;
        theaterChase(COLOR_BLUE, 50, 10);    // Blue theater chase
        if (!running) break;
        
        std::cout << "Rainbow animations." << std::endl;
        rainbow(20, 1);
        if (!running) break;
        rainbowCycle(20, 1);
        if (!running) break;
        
        std::cout << "Solid colors." << std::endl;
        solidColor(COLOR_RED);     // Solid red
        usleep(1000000);           // Sleep for 1 second
        if (!running) break;
        solidColor(COLOR_GREEN);   // Solid green
        usleep(1000000);           // Sleep for 1 second
        if (!running) break;
        solidColor(COLOR_BLUE);    // Solid blue
        usleep(1000000);           // Sleep for 1 second
        if (!running) break;
        
        std::cout << "Pulsing effects." << std::endl;
        pulse(COLOR_RED, 3, 5);    // Pulsing red
        if (!running) break;
        pulse(COLOR_GREEN, 3, 5);  // Pulsing green
        if (!running) break;
        pulse(COLOR_BLUE, 3, 5);   // Pulsing blue
        if (!running) break;
    }

    // Clear the LED strip on exit
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = COLOR_BLACK;
    }
    ws2811_render(&ledstring);

    // Cleanup
    ws2811_fini(&ledstring);

    return 0;
}

// Animation functions

// Color wipe animation
void colorWipe(uint32_t color, int wait_ms) {
    for (int i = 0; i < LED_COUNT; i++) {
        if (!running) return;
        ledstring.channel[0].leds[i] = color;
        ws2811_render(&ledstring);
        usleep(wait_ms * 1000);
    }
}

// Theater chase animation
void theaterChase(uint32_t color, int wait_ms, int iterations) {
    for (int j = 0; j < iterations; j++) {
        for (int q = 0; q < 3; q++) {
            if (!running) return;
            for (int i = 0; i < LED_COUNT; i += 3) {
                if (i + q < LED_COUNT) {
                    ledstring.channel[0].leds[i + q] = color;
                }
            }
            ws2811_render(&ledstring);
            usleep(wait_ms * 1000);
            
            for (int i = 0; i < LED_COUNT; i += 3) {
                if (i + q < LED_COUNT) {
                    ledstring.channel[0].leds[i + q] = COLOR_BLACK;
                }
            }
        }
    }
}

// Rainbow animation
void rainbow(int wait_ms, int iterations) {
    for (int j = 0; j < 256 * iterations; j++) {
        if (!running) return;
        for (int i = 0; i < LED_COUNT; i++) {
            ledstring.channel[0].leds[i] = wheel((i + j) & 255);
        }
        ws2811_render(&ledstring);
        usleep(wait_ms * 1000);
    }
}

// Rainbow cycle animation
void rainbowCycle(int wait_ms, int iterations) {
    for (int j = 0; j < 256 * iterations; j++) {
        if (!running) return;
        for (int i = 0; i < LED_COUNT; i++) {
            ledstring.channel[0].leds[i] = wheel(((i * 256 / LED_COUNT) + j) & 255);
        }
        ws2811_render(&ledstring);
        usleep(wait_ms * 1000);
    }
}

// Set all LEDs to a solid color
void solidColor(uint32_t color) {
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = color;
    }
    ws2811_render(&ledstring);
}

// Pulse animation
void pulse(uint32_t color, int cycles, int wait_ms) {
    int original_brightness = ledstring.channel[0].brightness;
    
    for (int cycle = 0; cycle < cycles; cycle++) {
        if (!running) return;
        
        // Increase brightness
        for (int brightness = 0; brightness < 255; brightness += 5) {
            if (!running) return;
            ledstring.channel[0].brightness = brightness;
            solidColor(color);
            usleep(wait_ms * 1000);
        }
        
        // Decrease brightness
        for (int brightness = 255; brightness > 0; brightness -= 5) {
            if (!running) return;
            ledstring.channel[0].brightness = brightness;
            solidColor(color);
            usleep(wait_ms * 1000);
        }
    }
    
    // Reset brightness to the original value
    ledstring.channel[0].brightness = original_brightness;
}

// Helper function to create rainbow effect
static uint32_t wheel(uint8_t pos) {
    pos = 255 - pos;
    if (pos < 85) {
        return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
    } else if (pos < 170) {
        pos -= 85;
        return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
    } else {
        pos -= 170;
        return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
    }
}
