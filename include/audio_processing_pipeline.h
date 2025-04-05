#ifndef IMAGE_PROCESS_PIPELINE_H
#define IMAGE_PROCESS_PIPELINE_H

#include <iostream>
//#include <Eigen/Dense>
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <cstdlib>


#define AUDIO_SAMPLE_RATE 22050
#define AUDIO_WINDOW_SIZE 2048
#define AUDIO_HOP_SIZE 512
#define AUDIO_NUM_MFCC 12
#define AUDIO_PCA_FEATURES 12
#define AUDIO_PCA_COMPONENTS 12


extern std::vector<std::vector<double>> __pca_components;
extern std::vector<double> __mean_vector;

void audio_processing_init();
#endif
