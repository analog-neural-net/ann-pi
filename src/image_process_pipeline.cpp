#include "image_process_pipeline.h"
#include "utilities.h"
#include "stb/stb_image_write.h"
#include "math.h"

std::vector<std::vector<double>> __pca_components;
std::vector<double> __mean_vector;

void image_processing_init(){
    __pca_components = loadMatrixCSV("./data/pca_components.csv", COMPONENTS, FEATURES);
    __mean_vector = loadVectorCSV("./data/mean.csv", FEATURES);

    std::cerr << "Loaded PCA Components: " << __pca_components.size() << " x " 
              << (__pca_components.empty() ? 0 : __pca_components[0].size()) << std::endl;
    std::cerr << "Loaded Mean Vector Size: " << __mean_vector.size() << std::endl;
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

void find_bounding_box(const std::vector<uint8_t>& image, int width, int height,
                        int& min_x, int& max_x, int& min_y, int& max_y, uint8_t threshold) {
    min_x = width, max_x = 0, min_y = height, max_y = 0;

    int row_threshold = 30;  
    int col_threshold = 30; 

    // ignore noise near top/bottom edges
    for (int y = 100; y < height - 100; y++) { 
        int black_pixel_count = 0;
        for (int x = 0; x < width; x++) {
            if (image[y * width + x] < threshold) black_pixel_count++;
        }
        if (black_pixel_count > row_threshold) { 
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }

    // ignore noise near left/right edges
    for (int x = 100; x < width - 100; x++) { 
        int black_pixel_count = 0;
        for (int y = 0; y < height; y++) {
            if (image[y * width + x] < threshold) black_pixel_count++;
        }
        if (black_pixel_count > col_threshold) {
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
        }
    }

    std::cerr << "Filtered Bounding Box: (" << min_x << "," << min_y << ") to (" << max_x << "," << max_y << ")\n";
}

void crop_to_square(const std::vector<uint8_t>& image, int width, int height,
                    std::vector<uint8_t>& cropped, int& new_size, uint8_t threshold) {
    int min_x, max_x, min_y, max_y;
    find_bounding_box(image, width, height, min_x, max_x, min_y, max_y, threshold);

    int digit_width = max_x - min_x + 1;
    int digit_height = max_y - min_y + 1;
    new_size = (digit_width > digit_height) ? digit_width : digit_height; // square cropping logic

    int center_x = (min_x + max_x) / 2;
    int center_y = (min_y + max_y) / 2;
    int half_size = new_size / 2;

    int crop_x1 = center_x - half_size;
    int crop_y1 = center_y - half_size;
    int crop_x2 = crop_x1 + new_size - 1;
    int crop_y2 = crop_y1 + new_size - 1;

    // clamp
    if (crop_x1 < 0) { crop_x2 -= crop_x1; crop_x1 = 0; }
    if (crop_y1 < 0) { crop_y2 -= crop_y1; crop_y1 = 0; }
    if (crop_x2 >= width) { crop_x1 -= (crop_x2 - width + 1); crop_x2 = width - 1; }
    if (crop_y2 >= height) { crop_y1 -= (crop_y2 - height + 1); crop_y2 = height - 1; }

    new_size = crop_x2 - crop_x1 + 1;
    if (new_size > (crop_y2 - crop_y1 + 1)) new_size = crop_y2 - crop_y1 + 1;

    std::cerr << "Cropping to: (" << crop_x1 << "," << crop_y1 << ") to (" 
              << crop_x2 << "," << crop_y2 << "), size: " << new_size << "x" << new_size << "\n";
    cropped.assign(new_size * new_size, 255);

    for (int y = crop_y1; y < crop_y2; y++) {
        for (int x = crop_x1; x < crop_x2; x++) {
            cropped[(y-crop_y1)*new_size + (x-crop_x1)] = image[y*width + x];
        }
    }
}


void pcaProject(const std::vector<uint8_t>& image, 
                std::vector<double>& out) {
    
    int num_components = __pca_components.size();  
    int num_features = __pca_components[0].size(); 

    out.assign(num_components, 0.0);

    if (image.size() != num_features || __mean_vector.size() != num_features) {
        std::cerr << "Error: Image, mean vector, and PCA components size mismatch!\n";
        exit(1);
    }

    std::vector<double> centered_image(num_features, 0.0);

    for (int i = 0; i < num_features; i++) {
        centered_image[i] = image[i] - __mean_vector[i];
    }

    double max = 0;

    for (int i = 0; i < num_components; i++) { 
        for (int j = 0; j < num_features; j++) { 
            out[i] += __pca_components[i][j] * centered_image[j];
            
        }
        
        if (fabs(out[i]) > max){
            max = fabs(out[i]);
        }
    }
    
    
    if (max == 0){
        return;
    }
    
    for (int i = 0; i < num_components; i++){
        out[i] = 2*(out[i]/max);
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

void darken_image(std::vector<uint8_t>& image,
                  uint8_t threshold,
                  float factor = 0.8f) {
                      
    for (int i = 0; i < image.size(); i++) {
        if (image[i] < threshold) {
            image[i] = static_cast<uint8_t>(image[i] * factor);
        }
    }
}

void invert(const std::vector<uint8_t>& image,
            std::vector<uint8_t>& out) {
    
    out.assign(image.size(), 0.0);
    
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

/*
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
*/

void gaussian_blur(const std::vector<uint8_t>& image, 
                   int width, 
                   int height, 
                   std::vector<uint8_t>& out){
                       
    out.assign(image.size(), 0.0);
    
    for (int y = 1; y < height - 1; y++){
        for (int x = 1; x < width - 1; x++){
            float sum = 0.0f;
            
            for (int ky = -1; ky <= 1; ky++){
                for (int kx = -1; kx <= 1; kx++){
                    int pixelX = x + kx;
                    int pixelY = y + ky;
                    int index = pixelY * width + pixelX;
                    
                    sum += image[index] * GAUSSIAN_KERNEL[ky + 1][kx +1];   
                }
                
            }
            
            out[y * width + x] = static_cast<uint8_t>(sum);
        }
        
    }
    
    for (int x = 0; x < width; x++){
        out[x] = image[x];
        out[(height - 1) * width + x] = image[(height - 1) * width + x];
    }
    for (int y = 0; y < height; y++){
        out[y * width] = image[y*width];
        out[y*width+(width-1)] = image[y*width+(width-1)];
    }

}

void process_image(const std::vector<uint8_t>& image, 
                   int width,
                   int height, 
                   std::vector<double>& out){
    
    //BEHOLD! The image processing pipeline!

    //Step 1: Crop the image (white border)/smart crop (digit bounds)
    std::vector<uint8_t> cropped_centered;
    int new_size;
    crop_to_square(image, width, height, cropped_centered, new_size, BLACK_THRESHOLD);


    int quality = 100;  // JPG quality
    bool success = stbi_write_jpg("data/step_1.jpg", new_size, new_size, 1, cropped_centered.data(), quality);    


    //Step 2: Contrast boost
    std::vector<uint8_t> thresholded_image;
    threshold(cropped_centered, BLACK_THRESHOLD, thresholded_image);

    quality = 100;  // JPG quality
    success = stbi_write_jpg("data/step_2.jpg", new_size, new_size, 1, thresholded_image.data(), quality);    

    //Step 3: Downsample the image
    std::vector<uint8_t> downsampled_image;
    downsampleInterArea(thresholded_image, new_size, new_size, DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, downsampled_image);
    
    quality = 100;  // JPG quality
    success = stbi_write_jpg("data/step_3.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, downsampled_image.data(), quality);    

    //Step 4: Contrast boost again
    threshold(downsampled_image, WHITE_THRESHOLD, thresholded_image);
    
    quality = 100;  // JPG quality

    success = stbi_write_jpg("data/step_4.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, thresholded_image.data(), quality);    
    
    //Step 5: Apply a gaussian blur
    std::vector<uint8_t> blurred_image;
    gaussian_blur(thresholded_image, DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, blurred_image);
    
    quality = 100;  // JPG quality
    success = stbi_write_jpg("data/step_5.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, blurred_image.data(), quality);    

    //Step 6: Darken the gaussian blur
    darken_image(blurred_image, WHITE_THRESHOLD);
    
    quality = 100;  // JPG quality
    success = stbi_write_jpg("data/step_6.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, blurred_image.data(), quality);    

    //Step 7: Invert and crop to 20x20 (will most likely be reverted to 28x28)
    std::vector<uint8_t> inverted;
    invert(blurred_image, inverted);
    
    std::vector<uint8_t> inverted_cropped;
    
    inverted_cropped.assign(FEATURES, 0.0);
    
    for (int i = 3; i < DOWNSAMPLE_SIZE-5; i++){
        for (int j = 4; j < DOWNSAMPLE_SIZE-4; j++){
            inverted_cropped[(i-3) + 20*(j-4)] = inverted[i + DOWNSAMPLE_SIZE*j];
        }
    } 
    
    quality = 100;  // JPG quality
    success = stbi_write_jpg("data/step_7.jpg", 20, 20, 1, inverted_cropped.data(), quality);   

    //Step 8: Project to PCA space
    pcaProject(inverted_cropped, out);
    
}

