#ifndef IMAGE_PROCESS_PIPELINE_H
#define IMAGE_PROCESS_PIPELINE_H

#include <Eigen/Dense>

Eigen::MatrixXd processImage(const Eigen::VectorXd& image_vector, int num_components);

#endif
