#ifndef IMAGE_PROCESS_PIPELINE_H
#define IMAGE_PROCESS_PIPELINE_H

#include <iostream>
#include <Eigen/Dense>
#include <vector>

#define BLACK_THRESHOLD 40
#define WHITE_THRESHOLD 200

#define DOWNSAMPLE_SIZE 28

#define FEATURES 784
#define COMPONENTS 12
// typedef enum{
//     THRESHOLD_UP = 0,
//     THRESHOLD_DOWN = 1,
// }DIRECTION

std::vector<std::vector<double>> __pca_components;
std::vector<double> __mean_vector;

void image_processing_init();

void process_image(const std::vector<double>& image, 
                  const std::vector<std::vector<double>>& pca_components, 
                  const std::vector<double>& mean,
                  std::vector<double>& out);

void downsampleInterArea(const std::vector<uint8_t>& image, 
                         int oldWidth, 
                         int oldHeight, 
                         int newWidth, 
                         int newHeight,
                         std::vector<uint8_t>& out);

void threshold(const std::vector<uint8_t>& image,
               uint8_t threshold,
               std::vector<uint8_t>& out);

#endif
