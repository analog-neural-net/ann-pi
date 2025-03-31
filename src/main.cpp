#include <thread>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> // Added for FIFO write

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
    if (!init_camera(ctx, 1440, 1440, 14)){
        std::cerr << "Camera init failed!!" << std::endl;
    }
    
    int flag_buf = 1;
    
    std::vector<uint8_t> image_data;
    std::vector<double> pca_coefficients;
    while(true) {
        int flag = gpio_read(27); // Check the push button
        if(!flag && flag_buf) {

            std::cerr << "Image capture started\n";
            capture_grayscale_image(ctx, image_data);
            std::cerr << "Image capture complete\n";
            
            stbi_write_jpg("data/image.jpg", 1440, 1440, 1, image_data.data(), 100);
            
            process_image(image_data, 1440, 1440, pca_coefficients);
            
            std::vector<int32_t> pca_coefficients_send;
            pca_coefficients_send.assign(pca_coefficients.size(), 0);
            
            for (int n = 0; n < pca_coefficients.size(); n++){
                pca_coefficients_send[n] = (pca_coefficients[n] * 10000);
            }
            
            uart_send_pca_data(pca_coefficients_send);
            
            std::cerr << "Data sent to STM!\n";
            
            
            int i = 0;
            uint8_t bytes[4];
            int32_t softmax_result[10];
            std::vector<double> softmax_doubles;
            
            for (i = 0; i < 12; i++){
                std::cerr << pca_coefficients_send[i]/10000.0 << std::endl;
            }
            /*
            
            while(i < 10){
                for (int j = 0; j < 4; j++){
                    bytes[j] =  uart_receive_char();
                }
                softmax_result[i] = 0;
                softmax_result[i] |= (bytes[3]);
                softmax_result[i] |= (bytes[2] << 8);
                softmax_result[i] |= (bytes[1] << 16);
                softmax_result[i] |= (bytes[0] << 24);
                i++;
            }
            std::vector<double> softmax_doubles;
            softmax_doubles.assign(10, 0);
            for (int x = 0; x < 10; x++){
                std::cout << softmax_result[x]/10000.0 << std::endl;
                softmax_doubles[x] = softmax_result[x]/10000.0;
            }
            */
            
            writeVectorToCSV("./data/softmax_results.csv", softmax_doubles);
            
            // Write "1" to the FIFO instead of stdout
            int fd = open("/tmp/cpp_to_py_fifo", O_WRONLY | O_NONBLOCK);
            
            if (fd == -1) {
                std::cerr << "failed to open FIFO\n";
            } else {
                //std::cerr << "❌ Failed to open FIFO for writing\n";
                write(fd, "1\n", 2);
                close(fd);
            }
        }
        flag_buf = flag;
        usleep(10000); // 10ms
    }

    return 0;
}
