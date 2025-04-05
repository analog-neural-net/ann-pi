#include "audio_process_pipeline.h"
#include "utilities.h"
#include "math.h"

std::vector<std::vector<double>> __pca_components;
std::vector<double> __mean_vector;

void audio_processing_init(){
    __pca_components = loadMatrixCSV("./data/pca_components_audio.csv", COMPONENTS, FEATURES);
    __mean_vector = loadVectorCSV("./data/mean_audio.csv", FEATURES);

    std::cerr << "Loaded PCA Components: " << __pca_components.size() << " x " 
              << (__pca_components.empty() ? 0 : __pca_components[0].size()) << std::endl;
    std::cerr << "Loaded Mean Vector Size: " << __mean_vector.size() << std::endl;
}
