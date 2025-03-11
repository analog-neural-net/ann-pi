#include "image_process_pipeline.h"
#include "utilities.h"


void image_processing_init(){
    __pca_components = loadMatrixCSV("./data/pca_components.csv", COMPONENTS, FEATURES);
    __mean_vector = loadVectorCSV("./data/mean.csv", FEATURES);
}

void gemv(const std::vector<std::vector<double>>& matrix,
          const std::vector<double>& vec, 
          std::vector<double>& out) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    out.assign(rows, 0.0);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            out[i] += matrix[i][j] * vec[j];
        }
    }
}

void pcaProject(const std::vector<double>& image, 
                const std::vector<std::vector<double>>& pca_components, 
                const std::vector<double>& mean,
                std::vector<double>& out) {
    
    int num_components = pca_components.size();  
    int num_features = pca_components[0].size(); 

    out.assign(num_components, 0.0);

    if (image.size() != num_features || mean.size() != num_features) {
        std::cerr << "Error: Image, mean vector, and PCA components size mismatch!\n";
        exit(1);
    }

    std::vector<double> centered_image(num_features, 0.0);

    for (int i = 0; i < num_features; i++) {
        centered_image[i] = image[i] - mean[i];
    }

    for (int i = 0; i < num_components; i++) { 
        for (int j = 0; j < num_features; j++) { 
            out[i] += pca_components[i][j] * centered_image[j];
        }
    }
}

void downsampleInterArea(const std::vector<uint8_t>& image, 
                         int oldWidth, 
                         int oldHeight, 
                         int newWidth, 
                         int newHeight,
                         std::vector<uint8_t>& out) {

    double scaleX = static_cast<double>(oldWidth) / newWidth;
    double scaleY = static_cast<double>(oldHeight) / newHeight;

    out.assign(newWidth * newHeight, 0);

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int startX = static_cast<int>(x * scaleX);
            int endX = static_cast<int>((x + 1) * scaleX);
            int startY = static_cast<int>(y * scaleY);
            int endY = static_cast<int>((y + 1) * scaleY);

            endX = std::min(endX, oldWidth);
            endY = std::min(endY, oldHeight);

            // Compute the average intensity over the mapped region
            int sum = 0;
            int count = 0;
            for (int yy = startY; yy < endY; ++yy) {
                for (int xx = startX; xx < endX; ++xx) {
                    sum += image[yy * oldWidth + xx];
                    count++;
                }
            }

            // Store the downsampled pixel value 
            out[y * newWidth + x] = (count > 0) ? static_cast<uint8_t>(sum / count) : 0;
        }
    }
}

void downsample(){

}

void crop(){

}

void invert(const std::vector<uint8_t>& image,
            std::vector<uint8_t>& out) {
    
    out.assign(image.size(), 0);

    for (int i = 0; i < image.size(); i++){
        out[i] = 255 - image[i];
    }

}

void threshold(const std::vector<uint8_t>& image,
               uint8_t threshold,
               std::vector<uint8_t>& out){
            
    out.assign(image.size(), 0);

    for (int i = 0; i < image.size(); i++){
        out[i] = image[i] < threshold ? 0 : 255;
    }
}

uint8_t computeThreshold(const std::vector<uint8_t>& image){

    std::vector<uint32_t> bins = std::vector<uint32_t>(256, 0);

    for (int i = 0; i < image.size(); i++){
        bins[image[i]]++;
    }

    std::vector<int> peaks;
    for (int i = 1; i < 255; i++) { 
        if (bins[i] > bins[i - 1] && bins[i] > bins[i + 1]) {
            peaks.push_back(i);
        }
    }

    int peak1 = 0, peak2 = 0;
    if (peaks.size() >= 2) {
        std::sort(peaks.begin(), peaks.end(), [&bins](int a, int b) {
            return bins[a] > bins[b];
        });

        peak1 = peaks[0]; 
        peak2 = peaks[1];
    }

    if (peak1 > peak2) std::swap(peak1, peak2);


    int minIndex = peak1;
    uint32_t minValue = bins[peak1];
    for (int i = peak1; i <= peak2; i++) {
        if (bins[i] < minValue) {
            minValue = bins[i];
            minIndex = i;
        }
    }

    return static_cast<uint8_t>(minIndex);
}

void process_image(const std::vector<double>& image, 
                  const std::vector<std::vector<double>>& pca_components, 
                  const std::vector<double>& mean,
                  std::vector<double>& out){
    
    //BEHOLD! The image processing pipeline!

    //Step 1: Crop the image (white border)
    crop();

    //Compute threshold
    //uint8_t threshold = computeThreshold();
    uint8_t threshold = BLACK_THRESHOLD;

    //Step 2: Contrast boost
    //threshold();

    //Step 3: Downsample the image
    //downsample();

    //Step 4: Invert the image
    //invert();

    //Step 3: Project to PCA space
    pcaProject(image, pca_components, mean, out);
}

