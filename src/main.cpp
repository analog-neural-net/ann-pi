#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "image_process_pipeline.h"
#include "gpio.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

// Function to load a CSV file into a 2D vector (Matrix)
std::vector<std::vector<double>> loadMatrixCSV(const std::string& filename, int rows, int cols) {
    std::ifstream file(filename);
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols, 0.0));

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    for (int i = 0; i < rows; i++) {
        getline(file, line);
        std::stringstream lineStream(line);
        std::string cell;
        for (int j = 0; j < cols; j++) {
            getline(lineStream, cell, ',');
            matrix[i][j] = stod(cell);
        }
    }
    file.close();
    return matrix;
}

// Function to load a CSV file into a 1D vector (Column Vector)
std::vector<double> loadVectorCSV(const std::string& filename, int size) {
    std::ifstream file(filename);
    std::vector<double> vec(size, 0.0);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    for (int i = 0; i < size; i++) {
        getline(file, line);
        vec[i] = stod(line);
    }
    file.close();
    return vec;
}

int main() {
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


    const char* output_filename = "data/downsampled_cpp.jpg";
    //write downsampled image to a jpg
    int quality = 100;  // JPG quality

    bool success = stbi_write_jpg(output_filename, 28, 28, 1, downsampled_image.data(), quality);

    std::cout << "Loaded image: " << width << "x" << height << " with " << channels << " channels" << std::endl;

    return 0;
    //gpio_func_select(ALT3, 23); // Set GPIO pin 23 to alternate function 3
    //gpio_func_select(INPUT, 18); // Set GPIO pin 18 to input

    return 0;
}
