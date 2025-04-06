#include "audio_process_pipeline.h"
#include "utilities.h"
#include "math.h"

std::vector<std::vector<double>> __pca_components;
std::vector<double> __mean_vector;

void audio_processing_init() {
    __pca_components = loadMatrixCSV("./data/pca_components_audio.csv", 12, 12);
    __mean_vector = loadVectorCSV("./data/mean_audio.csv", 12);
}

void hann_window(std::vector<float>& frame) {
    int N = frame.size();
    for (int i = 0; i < N; ++i) {
        frame[i] *= 0.5f * (1 - std::cos(2 * M_PI * i / (N - 1)));
    }
}

void fft(const std::vector<float>& in, std::vector<std::complex<float>>& out) {
    int N = in.size();
    out.resize(N);
    for (int i = 0; i < N; ++i) out[i] = std::complex<float>(in[i], 0);

    // Bit reversal
    int j = 0;
    for (int i = 0; i < N; ++i) {
        if (i < j) std::swap(out[i], out[j]);
        int m = N >> 1;
        while (m >= 1 && j >= m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // Cooley-Tukey FFT
    for (int s = 1; s <= std::log2(N); ++s) {
        int m = 1 << s;
        std::complex<float> wm = std::exp(std::complex<float>(0, -2.0f * M_PI / m));
        for (int k = 0; k < N; k += m) {
            std::complex<float> w = 1;
            for (int j = 0; j < m / 2; ++j) {
                std::complex<float> t = w * out[k + j + m / 2];
                std::complex<float> u = out[k + j];
                out[k + j] = u + t;
                out[k + j + m / 2] = u - t;
                w *= wm;
            }
        }
    }
}

std::vector<float> mel_filterbank(int n_filters, int fft_size, int sample_rate, float fmin = 0, float fmax = 8000.0f) {
    auto hz_to_mel = [](float hz) {
        return 2595.0f * std::log10(1.0f + hz / 700.0f);
    };
    auto mel_to_hz = [](float mel) {
        return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
    };

    float mel_min = hz_to_mel(fmin);
    float mel_max = hz_to_mel(fmax);

    std::vector<float> mel_points(n_filters + 2);
    for (int i = 0; i < mel_points.size(); ++i)
        mel_points[i] = mel_to_hz(mel_min + (mel_max - mel_min) * i / (n_filters + 1));

    std::vector<int> bin(n_filters + 2);
    for (int i = 0; i < bin.size(); ++i)
        bin[i] = static_cast<int>(std::floor((fft_size + 1) * mel_points[i] / sample_rate));

    std::vector<float> filters(fft_size / 2 * n_filters, 0.0f);
    for (int m = 1; m <= n_filters; ++m) {
        for (int k = bin[m - 1]; k < bin[m]; ++k)
            filters[(m - 1) * (fft_size / 2) + k] = (k - bin[m - 1]) / float(bin[m] - bin[m - 1]);
        for (int k = bin[m]; k < bin[m + 1]; ++k)
            filters[(m - 1) * (fft_size / 2) + k] = (bin[m + 1] - k) / float(bin[m + 1] - bin[m]);
    }
    return filters;
}

std::vector<double> dct(const std::vector<float>& input, int num_coeffs) {
    int N = input.size();
    std::vector<double> out(num_coeffs);
    for (int k = 0; k < num_coeffs; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += input[n] * std::cos(M_PI * k * (2 * n + 1) / (2.0 * N));
        }
        out[k] = sum * std::sqrt(2.0 / N);
    }
    return out;
}

std::vector<double> compute_mfcc(const std::vector<float>& audio, int sample_rate, int fft_size = 512, int hop_size = 256, int num_filters = 26, int num_coeffs = 12) {
    std::vector<double> mfcc_avg(num_coeffs, 0.0);
    auto filters = mel_filterbank(num_filters, fft_size, sample_rate);

    int num_frames = (audio.size() - fft_size) / hop_size + 1;
    for (int f = 0; f < num_frames; ++f) {
        std::vector<float> frame(audio.begin() + f * hop_size, audio.begin() + f * hop_size + fft_size);
        hann_window(frame);

        std::vector<std::complex<float>> spectrum;
        fft(frame, spectrum);

        std::vector<float> power(fft_size / 2, 0.0f);
        for (int i = 0; i < fft_size / 2; ++i) {
            power[i] = std::norm(spectrum[i]);
        }

        std::vector<float> mel_energies(num_filters, 0.0f);
        for (int m = 0; m < num_filters; ++m) {
            for (int i = 0; i < fft_size / 2; ++i) {
                mel_energies[m] += power[i] * filters[m * (fft_size / 2) + i];
            }
            mel_energies[m] = std::log(1 + mel_energies[m]);
        }

        auto mfcc = dct(mel_energies, num_coeffs);
        for (int i = 0; i < num_coeffs; ++i) {
            mfcc_avg[i] += mfcc[i];
        }
    }

    for (int i = 0; i < num_coeffs; ++i) {
        mfcc_avg[i] /= std::max(num_frames, 1);
    }
    return mfcc_avg;
}

std::vector<float> polyphase_resample(const std::vector<float>& input, unsigned int input_rate, unsigned int output_rate) {
    if (input_rate == output_rate) return input;

    double ratio = static_cast<double>(output_rate) / input_rate;
    size_t output_len = static_cast<size_t>(input.size() * ratio);
    std::vector<float> output(output_len);

    for (size_t i = 0; i < output_len; ++i) {
        double src_idx = i / ratio;
        size_t idx = static_cast<size_t>(src_idx);
        double frac = src_idx - idx;

        float s0 = (idx > 0) ? input[idx - 1] : input[0];
        float s1 = input[std::min(idx, input.size() - 1)];
        float s2 = input[std::min(idx + 1, input.size() - 1)];
        float s3 = input[std::min(idx + 2, input.size() - 1)];

        float a = (-0.5f * s0) + (1.5f * s1) - (1.5f * s2) + (0.5f * s3);
        float b = s0 - (2.5f * s1) + (2.0f * s2) - (0.5f * s3);
        float c = (-0.5f * s0) + (0.5f * s2);
        float d = s1;

        output[i] = ((a * frac + b) * frac + c) * frac + d;
    }

    return output;
}

// === PCA Globals ===

std::vector<std::vector<double>> loadMatrixCSV(const std::string& path, int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        exit(1);
    }
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file >> matrix[i][j];
            if (file.peek() == ',') file.ignore();
        }
    }
    return matrix;
}

std::vector<double> loadVectorCSV(const std::string& path, int size) {
    std::vector<double> vec(size);
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        exit(1);
    }
    for (int i = 0; i < size; ++i) {
        file >> vec[i];
        if (file.peek() == ',') file.ignore();
    }
    return vec;
}

float compute_rms(const std::vector<float>& segment) {
    float sum = 0.0f;
    for (float s : segment) {
        sum += s * s;
    }
    return std::sqrt(sum / segment.size());
}

void audio_processing_init() {
    __pca_components = loadMatrixCSV("./data/pca_components_audio.csv", 12, 12);
    __mean_vector = loadVectorCSV("./data/mean_audio.csv", 12);
}

void process_audio(const std::vector<float>& audio_data,
                   int sample_rate,
                   std::vector<double>& out_features) {
    int noise_samples = sample_rate / 4;
    std::vector<float> noise_clip(audio_data.begin(), audio_data.begin() + std::min((int)audio_data.size(), noise_samples));

    float noise_mean = std::accumulate(noise_clip.begin(), noise_clip.end(), 0.0f) / noise_clip.size();
    std::vector<float> reduced_audio(audio_data.size());
    for (size_t i = 0; i < audio_data.size(); ++i) {
        reduced_audio[i] = audio_data[i] - noise_mean;
    }


    std::vector<std::pair<int, int>> intervals;
    float threshold_db = -25.0f;
    int frame_size = 2048;
    int stride = 512;

    for (int i = 0; i + frame_size <= (int)reduced_audio.size(); i += stride) {
        std::vector<float> frame(reduced_audio.begin() + i, reduced_audio.begin() + i + frame_size);
        float rms = compute_rms(frame);
        float db = 20.0f * std::log10(rms + 1e-6f);

        if (db > threshold_db) {
            if (!intervals.empty() && i <= intervals.back().second + stride) {
                intervals.back().second = i + frame_size;
            } else {
                intervals.push_back(std::make_pair(i, i + frame_size));
            }
        }
    }

    std::vector<float> selected_audio;
    float max_score = -1.0f;
    for (size_t idx = 0; idx < intervals.size(); ++idx) {
        int start = intervals[idx].first;
        int end = intervals[idx].second;
        std::vector<float> segment(reduced_audio.begin() + start, reduced_audio.begin() + std::min(end, (int)reduced_audio.size()));
        float rms = compute_rms(segment);
        float score = rms * segment.size();
        if (score > max_score) {
            max_score = score;
            selected_audio = std::move(segment);
        }
    }

    if (selected_audio.empty()) {
        std::cerr << "[WARN] No significant audio segment found. Using entire audio." << std::endl;
        selected_audio = reduced_audio;
    }

    std::cout << "[INFO] Selected segment length: " << selected_audio.size() << " samples" << std::endl;

    float max_amp = 0.0f;
    for (float s : selected_audio) {
        if (std::abs(s) > max_amp) max_amp = std::abs(s);
    }
    if (max_amp > 0.0f) {
        for (float& s : selected_audio) {
            s /= max_amp;  // normalize to [-1, 1]
        }
    }

    if (!save_wav("test_segment.wav", selected_audio, sample_rate, 1)) {
        std::cerr << "[ERROR] Failed to save test_segment.wav" << std::endl;
    }

    std::vector<double> mfcc = compute_mfcc(selected_audio, sample_rate);

    std::vector<double> centered(12);
    for (int i = 0; i < 12; ++i) {
        std::cerr << mfcc[i] << std::endl;
        centered[i] = mfcc[i] - __mean_vector[i];
    }

    out_features.assign(__pca_components.size(), 0.0);
    for (size_t i = 0; i < __pca_components.size(); ++i) {
        for (size_t j = 0; j < __pca_components[i].size(); ++j) {
            out_features[i] += __pca_components[i][j] * centered[j];
        }
    }

    double max_abs = 0.0;
    for (double val : out_features) {
        if (std::abs(val) > max_abs) max_abs = std::abs(val);
    }
    if (max_abs > 0.0) {
        for (double& val : out_features) {
            val = 0.625 * (val / max_abs);
        }
    }

    double scale = (2.0 * 2.75) / 4096.0;
    for (double& val : out_features) {
        val = std::round(val / scale) * scale;
    }
}
