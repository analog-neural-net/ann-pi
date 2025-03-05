#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "image_process_pipeline.h"

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

    return 0;
}
