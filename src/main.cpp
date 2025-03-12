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

    /*
    int num_features = 784;
    int num_components = 12;

    // Load PCA components (12x784)
    std::vector<std::vector<double>> pca_components = loadMatrixCSV("./data/pca_components.csv", num_components, num_features);

    // Load mean vector (784x1)
    std::vector<double> mean_vector = loadVectorCSV("./data/mean.csv", num_features);
    // Load test image vector (784x1)
    std::vector<double> test_image = loadVectorCSV("./data/test_image.csv", num_features);

    std::vector<double> test_image_pca_coefficients = loadVectorCSV("./data/test_image_pca_coefficients.csv", num_components);
    
    std::vector<double> pca_projection;

    processImage(test_image, pca_components, mean_vector, pca_projection);

    for (int i = 0; i < pca_projection.size(); i++){
        printf("Error: %lf%\n", std::fabs((test_image_pca_coefficients[i]-pca_projection[i])/test_image_pca_coefficients[i])*100);
    }
    */

/*
    int width, height, channels;
    unsigned char* img = stbi_load("data/720p_test_8.jpg", &width, &height, &channels, 1);

    if (!img) {
        std::cerr << "Error: Could not load image!" << std::endl;
        return -1;
    }

    std::vector<uint8_t> image_data(img, img + (width * height));
    std::vector<uint8_t> thresholded_image = std::vector<uint8_t>();
    std::vector<uint8_t> downsampled_image = std::vector<uint8_t>();

    stbi_image_free(img);

    threshold(image_data, BLACK_THRESHOLD, thresholded_image);

    downsampleInterArea(thresholded_image, width, height, 28, 28, downsampled_image);

    threshold(downsampled_image, WHITE_THRESHOLD, thresholded_image);

    //apply gaussian blur on thresholded image
    
    //darken
    
    //invert
    
    //project onto pca space
    
    uart_send_string("Hello World!")

    const char* output_filename = "data/downsampled_cpp.jpg";
    //write downsampled image to a jpg
    int quality = 100;  // JPG quality

    bool success = stbi_write_jpg(output_filename, 28, 28, 1, downsampled_image.data(), quality);

    std::cout << "Loaded image: " << width << "x" << height << " with " << channels << " channels" << std::endl;

    //gpio_func_select(ALT3, 23); // Set GPIO pin 23 to alternate function 3
    //gpio_func_select(INPUT, 18); // Set GPIO pin 18 to input

*/
/*
    while(true){
    int input = gpio_read(24);
    if (input){
    //printf("%d", gpio_read(24));
    uart_send_string("Hello World!");
    sleep(1);
    
    }
    
    }
    while(true){}
        return 0;
*/
    
    while(true){
        
        int flag = gpio_read(24);
        if(flag){
            pid_t pid = fork();
            
            if (pid == 0) {
                execlp("libcamera-still", "libcamera-still", "-o", "data/image.jpg", "--width", "1280", "--height", "720", "--immediate", (char*)NULL);
                exit(1);
            } else{
                std::cerr << "Image capture started\n";
                int status;
                waitpid(pid, &status, 0);
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
