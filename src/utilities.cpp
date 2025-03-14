#include "utilities.h"

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
