#include "led.h"

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

// Setup signal handlers
void setup_ws2811(void) {
    
    ws2811_return_t ret;
    
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS) {
        std::cerr << "ws2811_init failed: " << ws2811_get_return_t_str(ret) << std::endl;
    }
    
}

// Animation functions

// Color wipe animation
void colorWipe(uint32_t color, int wait_ms) {
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = color;
        ws2811_render(&ledstring);
        usleep(wait_ms * 1000);
    }
}

// Theater chase animation
void theaterChase(uint32_t color, int wait_ms, int iterations) {
    for (int j = 0; j < iterations; j++) {
        for (int q = 0; q < 3; q++) {
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
        
        // Increase brightness
        for (int brightness = 0; brightness < 255; brightness += 5) {
            ledstring.channel[0].brightness = brightness;
            solidColor(color);
            usleep(wait_ms * 1000);
        }
        
        // Decrease brightness
        for (int brightness = 255; brightness > 0; brightness -= 5) {
            ledstring.channel[0].brightness = brightness;
            solidColor(color);
            usleep(wait_ms * 1000);
        }
    }
    
    // Reset brightness to the original value
    ledstring.channel[0].brightness = original_brightness;
}

// Helper function to create rainbow effect
uint32_t wheel(uint8_t pos) {
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
