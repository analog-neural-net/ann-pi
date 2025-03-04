#ifndef IMAGE_PROCESS_PIPELINE_H
#define IMAGE_PROCESS_PIPELINE_H

#include <iostream>
#include <Eigen/Dense>
#include <vector>


void processImage(const std::vector<double>& image, 
                  const std::vector<std::vector<double>>& pca_components, 
                  const std::vector<double>& mean,
                  std::vector<double>& out);

#endif
