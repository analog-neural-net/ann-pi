#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;

// Function to load a CSV file into a 2D vector (Matrix)
vector<vector<double>> loadMatrixCSV(const string& filename, int rows, int cols) {
    ifstream file(filename);
    vector<vector<double>> matrix(rows, vector<double>(cols, 0.0));

    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
    for (int i = 0; i < rows; i++) {
        getline(file, line);
        stringstream lineStream(line);
        string cell;
        for (int j = 0; j < cols; j++) {
            getline(lineStream, cell, ',');
            matrix[i][j] = stod(cell);
        }
    }
    file.close();
    return matrix;
}

// Function to load a CSV file into a 1D vector (Column Vector)
vector<double> loadVectorCSV(const string& filename, int size) {
    ifstream file(filename);
    vector<double> vec(size, 0.0);

    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
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
    vector<vector<double>> pca_components = loadMatrixCSV("./data/pca_components.csv", num_components, num_features);

    // Load mean vector (784x1)
    vector<double> mean_vector = loadVectorCSV("./data/mean.csv", num_features);


    return 0;
}
