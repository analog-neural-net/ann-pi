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

void audio_processing_init();
