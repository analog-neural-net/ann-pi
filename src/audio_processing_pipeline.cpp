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

float compute_rms(const std::vector<float>& segment) {
    float sum = 0.0f;
    for (float s : segment) {
        sum += s * s;
    }
    return std::sqrt(sum / segment.size());
}

void process_audio(const std::vector<float>& audio_data,
                   int sample_rate,
                   std::vector<double>& out_features) {
    // Step 1: Use first 0.25s as noise sample
    int noise_samples = sample_rate / 4;
    std::vector<float> noise_clip(audio_data.begin(), audio_data.begin() + std::min((int)audio_data.size(), noise_samples));

    // Step 2: Simple noise reduction (subtract average noise)
    float noise_mean = std::accumulate(noise_clip.begin(), noise_clip.end(), 0.0f) / noise_clip.size();
    std::vector<float> reduced_audio(audio_data.size());
    for (size_t i = 0; i < audio_data.size(); ++i) {
        reduced_audio[i] = audio_data[i] - noise_mean;
    }

    // Step 3: Select most significant segment based on RMS * duration
    int frame_len = AUDIO_WINDOW_SIZE;
    int hop_len = AUDIO_HOP_SIZE;

    int best_start = 0;
    float max_score = -1.0f;

    for (int i = 0; i + frame_len < reduced_audio.size(); i += hop_len) {
        std::vector<float> frame(reduced_audio.begin() + i, reduced_audio.begin() + i + frame_len);
        float rms = compute_rms(frame);
        float score = rms * frame.size();
        if (score > max_score) {
            max_score = score;
            best_start = i;
        }
    }

    std::vector<float> selected_audio(reduced_audio.begin() + best_start, reduced_audio.begin() + best_start + frame_len);

    // Step 4: Compute 12 MFCCs (placeholder: use DCT of log energies across 12 bins)
    std::vector<double> mfcc(AUDIO_NUM_MFCC, 0.0);
    for (int i = 0; i < AUDIO_NUM_MFCC; ++i) {
        mfcc[i] = std::log(1 + std::abs(selected_audio[i % selected_audio.size()]));
    }

    // Step 5: Apply PCA
    std::vector<double> centered(AUDIO_NUM_MFCC);
    for (int i = 0; i < AUDIO_NUM_MFCC; ++i) {
        centered[i] = mfcc[i] - __mean_vector[i];
    }

    out_features.assign(__pca_components.size(), 0.0);
    for (size_t i = 0; i < __pca_components.size(); ++i) {
        for (size_t j = 0; j < __pca_components[i].size(); ++j) {
            out_features[i] += __pca_components[i][j] * centered[j];
        }
    }

    // Step 6: Normalize to [-0.625, 0.625]
    double max_abs = 0.0;
    for (double val : out_features) {
        if (std::abs(val) > max_abs) max_abs = std::abs(val);
    }
    if (max_abs > 0.0) {
        for (double& val : out_features) {
            val = 0.625 * (val / max_abs);
        }
    }

    // Step 7: Quantize to 12-bit resolution
    double scale = (2.0 * 2.75) / 4096.0;
    for (double& val : out_features) {
        val = std::round(val / scale) * scale;
    }
}
