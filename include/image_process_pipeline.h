#ifndef IMAGE_PROCESS_PIPELINE_H
#define IMAGE_PROCESS_PIPELINE_H

#include <iostream>
//#include <Eigen/Dense>
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <cstdint>

#define BLACK_THRESHOLD 130
#define WHITE_THRESHOLD 200

#define DOWNSAMPLE_SIZE 24

#define FEATURES 576
#define COMPONENTS 12
// typedef enum{
//     THRESHOLD_UP = 0,
//     THRESHOLD_DOWN = 1,
// }DIRECTION

extern std::vector<std::vector<double>> __pca_components;
extern std::vector<double> __mean_vector;

const float GAUSSIAN_KERNEL[3][3] = {
  { 1/16.0, 2/16.0, 1/16.0 },
  { 2/16.0, 4/16.0, 2/16.0 },
  { 1/16.0, 2/16.0, 1/16.0 } 
};

void image_processing_init();

void process_image(const std::vector<uint8_t>& image, 
                   int width,
                   int height,
                   std::vector<double>& out);
                   
                   
/*
void downsampleInterArea(const std::vector<uint8_t>& image, 
                         int oldWidth, 
                         int oldHeight, 
                         int newWidth, 
                         int newHeight,
                         std::vector<uint8_t>& out);

void threshold(const std::vector<uint8_t>& image,
               uint8_t threshold,
               std::vector<uint8_t>& out);
*/

void pcaProject(const std::vector<double>& image, 
                std::vector<double>& out);
#endif
