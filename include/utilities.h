#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

// Function to load a CSV file into a 2D vector (Matrix)
std::vector<std::vector<double>> loadMatrixCSV(const std::string& filename, int rows, int cols);

// Function to load a CSV file into a 1D vector (Column Vector)
std::vector<double> loadVectorCSV(const std::string& filename, int size);

#endif
