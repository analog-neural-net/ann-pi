#include "image_process_pipeline.h"

#include <iostream>
#include <Eigen/Dense>
#include <vector>


void gemv(const std::vector<std::vector<double>>& matrix, const std::vector<double>& vec, std::vector<double>& out) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    out.assign(rows, 0.0);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            out[i] += matrix[i][j] * vec[j];
        }
    }
}


void pcaProject(const std::vector<double>& image, 
                const std::vector<std::vector<double>>& pca_components, 
                const std::vector<double>& mean,
                std::vector<double>& out) {
    
    int num_components = pca_components.size();  
    int num_features = pca_components[0].size(); 

    out.assign(num_components, 0.0);

    if (image.size() != num_features || mean.size() != num_features) {
        std::cerr << "Error: Image, mean vector, and PCA components size mismatch!\n";
        exit(1);
    }

    std::vector<double> centered_image(num_features, 0.0);

    for (int i = 0; i < num_features; i++) {
        centered_image[i] = image[i] - mean[i];
    }

    for (int i = 0; i < num_components; i++) { 
        for (int j = 0; j < num_features; j++) { 
            out[i] += pca_components[i][j] * centered_image[j];
        }
    }
}

void downsample(){

}

void crop(){

}


void processImage(const std::vector<double>& image, std::vector<double>& out){
    
    //BEHOLD! The image processing pipeline!

    //Step 1: Crop the image
    crop();

    //Step 2: Downsample the image
    downsample();
    
    //Step 3: Project to PCA space
    //pcaProject();
}

