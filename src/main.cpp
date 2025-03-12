#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "image_process_pipeline.h"
#include "gpio.h"
#include "uart.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

int main() {
    gpio_init();
    uart_init();
    image_processing_init();

    gpio_func_select(INPUT, 24);
    gpio_func_select(ALT0, 14);
    gpio_func_select(ALT0, 15);

    // README!!!!:
    // THE CROP FUNCTION HAS **NOT** BEEN IMPLEMENTED
    // FOR THE TIME BEING CHANGE THE WIDTH AND HEIGHT 
    // PARAMETERS IN THE execlp() CALL!!! AND TUNE IT
    // TO ISOLATE JUST THE WHITE PAPER!!!!!!!!!!!!!!!
    
    while(true){
        
        int flag = gpio_read(24); // Push button
        if(flag){
            pid_t pid = fork();
            
            if (pid == 0) {  // Camera child process
                execlp("libcamera-still", "libcamera-still", "-o", "data/image.jpg", "--width", "1280", "--height", "720", "--immediate", (char*)NULL);
                exit(1);
            } else{
                std::cerr << "Image capture started\n";
                int status;
                waitpid(pid, &status, 0); // Waits for picture to be taken
                std::cerr << "Image capture completed\n";
                int width, height, channels;
                unsigned char* img = stbi_load("data/image.jpg", &width, &height, &channels, 1);
                std::vector<uint8_t> image_data(img, img + (width * height));
                stbi_image_free(img);
                
                std::vector<double> pca_coefficients;
                
                process_image(image_data, width, height, pca_coefficients);
                
                for (int i = 0; i < pca_coefficients.size(); i++){
                    printf("%lf\n", pca_coefficients[i]);
                }
                
                std::vector<int32_t> pca_projection;
                pca_projection.assign(pca_coefficients.size(), 0);
                
                for (int i = 0; i < pca_projection.size(); i++){
                    pca_projection[i] = static_cast<int32_t>(pca_coefficients[i] * 10000);
                }
                
                uart_send_pca_data(pca_projection);                
            }
        }
    }
}
