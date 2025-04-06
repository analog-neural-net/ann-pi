#pragma once

#include <alsa/asoundlib.h>
#include <vector>
#include <iostream>

struct MicrophoneContext {
    snd_pcm_t *handle = nullptr;
    int sample_rate = 44100;
    int channels = 1;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
};

bool init_microphone(MicrophoneContext &ctx, const char *device = "default") {
    snd_pcm_hw_params_t *hw_params;
    int err;

    if ((err = snd_pcm_open(&ctx.handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "Cannot open audio device " << device << ": " << snd_strerror(err) << "\n";
        return false;
    }

    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(ctx.handle, hw_params);
    snd_pcm_hw_params_set_access(ctx.handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(ctx.handle, hw_params, ctx.format);
    snd_pcm_hw_params_set_rate(ctx.handle, hw_params, ctx.sample_rate, 0);
    snd_pcm_hw_params_set_channels(ctx.handle, hw_params, ctx.channels);

    if ((err = snd_pcm_hw_params(ctx.handle, hw_params)) < 0) {
        std::cerr << "Cannot set parameters: " << snd_strerror(err) << "\n";
        snd_pcm_hw_params_free(hw_params);
        return false;
    }

    snd_pcm_hw_params_free(hw_params);
    snd_pcm_prepare(ctx.handle);
    return true;
}

bool capture_audio(MicrophoneContext &ctx, std::vector<float> &audio_out, int duration_seconds = 1) {
    int total_frames = duration_seconds * ctx.sample_rate;
    std::vector<int16_t> buffer(total_frames);

    int frames_read = snd_pcm_readi(ctx.handle, buffer.data(), total_frames);
    if (frames_read < 0) {
        frames_read = snd_pcm_recover(ctx.handle, frames_read, 0);
        if (frames_read < 0) {
            std::cerr << "Failed to recover from audio capture error: " << snd_strerror(frames_read) << "\n";
            return false;
        }
    }

    audio_out.resize(frames_read);
    for (int i = 0; i < frames_read; ++i) {
        audio_out[i] = static_cast<float>(buffer[i]) / 32768.0f; // normalize to [-1, 1]
    }

    return true;
}

void cleanup_microphone(MicrophoneContext &ctx) {
    if (ctx.handle) {
        snd_pcm_close(ctx.handle);
        ctx.handle = nullptr;
    }
}
