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

inline int clamp(int val, int min_val, int max_val) {
    return std::max(min_val, std::min(val, max_val));
}

inline float clampf(float val, float min_val, float max_val) {
    return std::max(min_val, std::min(val, max_val));
}

inline int roundf_to_int(float val) {
    return static_cast<int>(std::round(val));
}

float cubic_interpolate(float p0, float p1, float p2, float p3, float t) {
    return p1 + 0.5f * t * (
        p2 - p0 + t * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 +
        t * (3.0f * (p1 - p2) + p3 - p0)));
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

void rotate_image(const std::vector<uint8_t>& image,
                  std::vector<uint8_t>& out){
                      
    out.assign(image.size(), 0);
    
    for (int i = image.size(), j = 0; i >= 0; i--, j++){
        out[j] = image[i];
    }
}

void find_bounding_box(const std::vector<uint8_t>& image, int width, int height,
                        int& min_x, int& max_x, int& min_y, int& max_y, uint8_t threshold) {
    min_x = width, max_x = 0, min_y = height, max_y = 0;

    int row_threshold = 5;  
    int col_threshold = 5; 

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

    //std::cerr << "Filtered Bounding Box: (" << min_x << "," << min_y << ") to (" << max_x << "," << max_y << ")\n";
}

void add_circular_brightness(const std::vector<uint8_t>& image,
                             int width,
                             int height,
                             std::vector<uint8_t>& out,
                             double adjustment_factor = 1.2,
                             double gamma = 8.0) {
    out.assign(image.size(), 0);

    int cx = width / 2;
    int cy = height / 2;

    std::vector<float> dist_map(width * height, 0.0f);
    float max_dist_sq = 0.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dx = x - cx;
            int dy = y - cy;
            float dist_sq = static_cast<float>(dx * dx + dy * dy);
            dist_map[y * width + x] = dist_sq;
            if (dist_sq > max_dist_sq) {
                max_dist_sq = dist_sq;
            }
        }
    }


    for (int i = 0; i < width * height; ++i) {
        float normalized = dist_map[i] / max_dist_sq; 
        float edge_boost = std::pow(normalized, gamma); 
        float factor = 1.0f + static_cast<float>(adjustment_factor) * edge_boost;

        float adjusted = image[i] * factor;
        out[i] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, adjusted)));
    }
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

    /*
    std::cerr << "Cropping to: (" << crop_x1 << "," << crop_y1 << ") to (" 
              << crop_x2 << "," << crop_y2 << "), size: " << new_size << "x" << new_size << "\n";
    */
    cropped.assign(new_size * new_size, 255);

    for (int y = crop_y1; y < crop_y2; y++) {
        for (int x = crop_x1; x < crop_x2; x++) {
            cropped[(y-crop_y1)*new_size + (x-crop_x1)] = image[y*width + x];
        }
    }
}


float bicubic_sample(const std::vector<uint8_t>& img, int width, int height, float x, float y) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;

    float col[4];
    for (int m = -1; m <= 2; m++) {
        float row[4];
        for (int n = -1; n <= 2; n++) {
            int px = clamp(ix + n, 0, width - 1);
            int py = clamp(iy + m, 0, height - 1);
            row[n + 1] = static_cast<float>(img[py * width + px]);
        }
        col[m + 1] = cubic_interpolate(row[0], row[1], row[2], row[3], fx);
    }

    return clampf(cubic_interpolate(col[0], col[1], col[2], col[3], fy), 0.0f, 255.0f);
}

void find_digit_cubic(const std::vector<uint8_t>& image, int width, int height,
                                  std::vector<uint8_t>& out, int target_width = 24, int target_height = 24) {
    // Find bounding box
    int top = height, bottom = -1, left = width, right = -1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (image[y * width + x] > 25) { // threshold ~ 0.1 * 255
                top = std::min(top, y);
                bottom = std::max(bottom, y);
                left = std::min(left, x);
                right = std::max(right, x);
            }
        }
    }

    // Initialize output to white
    out.assign(target_width * target_height, 255);

    if (top > bottom || left > right) {
        // No digit found
        return;
    }

    // Crop
    int cropped_w = right - left + 1;
    int cropped_h = bottom - top + 1;
    std::vector<uint8_t> cropped(cropped_w * cropped_h);

    for (int y = 0; y < cropped_h; ++y) {
        for (int x = 0; x < cropped_w; ++x) {
            cropped[y * cropped_w + x] = image[(top + y) * width + (left + x)];
        }
    }

    // Bicubic resize
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            float src_x = (x + 0.5f) * cropped_w / target_width - 0.5f;
            float src_y = (y + 0.5f) * cropped_h / target_height - 0.5f;
            float pixel = bicubic_sample(cropped, cropped_w, cropped_h, src_x, src_y);
            out[y * target_width + x] = static_cast<uint8_t>(roundf_to_int(pixel));
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
        centered_image[i] = image[i]/255.0 - __mean_vector[i];
    }

    double max = 0;

    //std::cerr << "After projection: \n";
    
    for (int i = 0; i < num_components; i++) { 
        for (int j = 0; j < num_features; j++) { 
            out[i] += __pca_components[i][j] * centered_image[j];
            
        }
        
        if (fabs(out[i]) > max){
            max = fabs(out[i]);
        }
        //std::cerr << out[i] << std::endl;
    }
    
    //std::cerr << "=======================================\n";
    if (max == 0){
        return;
    }
    
    //std::cerr << "After normalization: \n";
    
    for (int i = 0; i < num_components; i++){
        out[i] = 0.625*(out[i]/max);
        //std::cerr << out[i] << std::endl;
    }
    //std::cerr << "=======================================\n";
    
    int levels = 4096;
    double scale = (2.0*2.75) / ((double) levels);
    
    //std::cerr << "After quantization: \n";
    
    for (int i = 0; i < num_components; i++){
        out[i] = round(out[i]/scale) * scale;
        //std::cerr << out[i] << std::endl;
    }
    //std::cerr << "=======================================\n";
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

void crop(const std::vector<uint8_t>& image, 
          int input_width, int input_height,
          int crop_width, int crop_height, 
          std::vector<uint8_t>& out) {
    int start_x = (input_width - crop_width) / 2;
    int start_y = (input_height - crop_height) / 2;

    out.assign(crop_width*crop_height, 0);
    
    for (int y = 0; y < crop_height; ++y) {
        for (int x = 0; x < crop_width; ++x) {
            out[y * crop_width + x] = image[(start_y + y) * input_width + (start_x + x)];
            //std::cerr << (start_y + y) * input_width + (start_x + x) << std::endl;
            //exit(1);
        }
    }
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

void lighten_image(std::vector<uint8_t>& image,
                   uint8_t threshold,
                   float factor = 1.2f){
    
    for (int i = 0; i < image.size(); i++){
        if (image[i] > threshold){
            float brightened = image[i] * factor;
            image[i] = static_cast<uint8_t>(std::min(255.0f, brightened));
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
    //crop_to_square(image, width, height, cropped_centered, new_size, BLACK_THRESHOLD);
    
    rotate_image(image, cropped_centered);
    
    int quality = 100;  // JPG quality
    bool success = stbi_write_jpg("data/step_1.jpg", width, height, 1, cropped_centered.data(), quality);    


    std::vector<uint8_t> center_crop;
    
    //crop(cropped_centered, width, height, 1000, 1000, center_crop);
    add_circular_brightness(cropped_centered, width, height, center_crop, 2.5, 3.5);
    //success = stbi_write_jpg("data/step_1.5.jpg", width, height, 1, center_crop.data(), quality);    
    

    //Step 2: Contrast boost
    std::vector<uint8_t> thresholded_image;
    threshold(center_crop, BLACK_THRESHOLD, thresholded_image);
    
    //width = 1000; height = 1000;

    //quality = 100; 
    //success = stbi_write_jpg("data/step_2.jpg", width, height, 1, thresholded_image.data(), quality);    
    
    std::vector<uint8_t> digit_bound;
    crop_to_square(thresholded_image, width, height, digit_bound, new_size, BLACK_THRESHOLD);
    
    //success = stbi_write_jpg("data/step_2.5.jpg", new_size, new_size, 1, digit_bound.data(), quality);  

    //Step 3: Downsample the image
    std::vector<uint8_t> downsampled_image;
    downsampleInterArea(digit_bound, new_size, new_size, DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, downsampled_image);
    
    //quality = 100; 
    //success = stbi_write_jpg("data/step_3.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, downsampled_image.data(), quality);    

    //Step 4: Contrast boost again
    //threshold(downsampled_image, WHITE_THRESHOLD, thresholded_image);
    
    //quality = 100; 

    //success = stbi_write_jpg("data/step_4.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, thresholded_image.data(), quality);    
    
    //Step 5: Apply a gaussian blur
    //std::vector<uint8_t> blurred_image;
    //gaussian_blur(thresholded_image, DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, blurred_image);
    
    //quality = 100;  // JPG quality
    //success = stbi_write_jpg("data/step_5.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, blurred_image.data(), quality);    

    //Step 6: Darken the gaussian blur
    //darken_image(blurred_image, WHITE_THRESHOLD);
    
    //quality = 100;  // JPG quality
    //success = stbi_write_jpg("data/step_6.jpg", DOWNSAMPLE_SIZE, DOWNSAMPLE_SIZE, 1, blurred_image.data(), quality);    

    //Step 7: Invert 
    std::vector<uint8_t> inverted;
    invert(downsampled_image, inverted);
    
    quality = 100;  // JPG quality
    //success = stbi_write_jpg("data/step_7.jpg", 24, 24, 1, inverted.data(), quality);   
    
    std::vector<uint8_t> output(28*28, 0);
    
    for (int y = 0; y < 24; y++){
        for (int x = 0; x < 24; x++){
            output[(y+2)*28+(x+2)] = inverted[y*24 + x];
        }
    }
    
    std::vector<uint8_t> blurred_image;
    gaussian_blur(output, 28, 28, blurred_image);
    
    lighten_image(blurred_image, 2, 3.5f);
    
    //success = stbi_write_jpg("data/step_7.jpg", 28, 28, 1, blurred_image.data(), quality);   
    
    
    std::vector<uint8_t> output_for_pca(24*24, 0);
    find_digit_cubic(blurred_image, 28, 28, output_for_pca, 24, 24);

    success = stbi_write_jpg("data/step_8.jpg", 24, 24, 1, output_for_pca.data(), quality);   
    //Step 8: Project to PCA space
    pcaProject(output_for_pca, out);
    
}

