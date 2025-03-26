
#include <thread>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "image_process_pipeline.h"
#include "utilities.h"
#include "gpio.h"
#include "uart.h"
#include "led.h"
#include "camera.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

int main() {
    // Initialize hardware and image processing pipeline
    gpio_init();
    uart_init();
    image_processing_init();

    gpio_func_select(OUTPUT, 16);
    gpio_func_select(INPUT, 27);
    gpio_func_select(ALT4, 4);
    gpio_func_select(ALT4, 5);
    
    gpio_pull_resistor(PULL_UP, 27);
    gpio_set(21);
    setup_ws2811();
    solidColor(COLOR_WHITE);
    
    CameraContext ctx;
    if (!init_camera(ctx, 1440, 1440)){
        std::cerr << "Camera init failed!!" << std::endl;
    }
    
    std::vector<uint8_t> image_data;
    while(true) {
        int flag = gpio_read(27); // Check the push button
        if(!flag) {
            uart_send_string("ANN-E");
            printf("image sent\n");
            /*
            std::cerr << "Image capture started\n";
            capture_grayscale_image(ctx, image_data);
            std::cerr << "Image capture complete\n";
            
            // Optionally, save the image locally for debugging
            stbi_write_jpg("data/image.jpg", 1440, 1440, 1, image_data.data(), 100);
            std::cout << "1";
            */
            // Debounce delay (adjust as needed)
            usleep(500000); // 500ms
        }
    }

    return 0;
}
