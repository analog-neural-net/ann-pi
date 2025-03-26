#pragma once

#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <libcamera/control_ids.h>

#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sys/mman.h>

struct CameraContext {
    std::shared_ptr<libcamera::Camera> camera;
    std::unique_ptr<libcamera::CameraConfiguration> config_ptr;
    libcamera::CameraConfiguration *config;
    libcamera::Stream *stream;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> *buffers;
    int width;
    int height;
};

bool init_camera(CameraContext &ctx, int width = 640, int height = 480) {
    using namespace libcamera;

    static CameraManager *manager = new CameraManager();
    if (manager->start()) {
        std::cerr << "Failed to start CameraManager\n";
        return false;
    }

    if (manager->cameras().empty()) {
        std::cerr << "No cameras found.\n";
        return false;
    }

    ctx.camera = manager->get(manager->cameras()[0]->id());
    if (!ctx.camera || ctx.camera->acquire()) {
        std::cerr << "Failed to acquire camera.\n";
        return false;
    }

    ctx.config_ptr = ctx.camera->generateConfiguration({ StreamRole::StillCapture });
    ctx.config = ctx.config_ptr.get();
    auto &cfg = ctx.config->at(0);
    cfg.pixelFormat = formats::YUV420; // Arducam IMX519 does not support R8
    cfg.size.width = width;
    cfg.size.height = height;
    ctx.width = width;
    ctx.height = height;
    ctx.config->validate();

    if (ctx.camera->configure(ctx.config) < 0) {
        std::cerr << "Failed to configure camera.\n";
        return false;
    }

    ctx.stream = cfg.stream();
    ctx.allocator = std::make_unique<FrameBufferAllocator>(ctx.camera);

    if (ctx.allocator->allocate(ctx.stream) < 0) {
        std::cerr << "Failed to allocate buffers.\n";
        return false;
    }

    ctx.buffers = &ctx.allocator->buffers(ctx.stream);
    return true;
}

void *map_framebuffer(libcamera::FrameBuffer *fb) {
    const libcamera::FrameBuffer::Plane &plane = fb->planes()[0];
    int fd = plane.fd.get();
    void *memory = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED)
        return nullptr;
    return memory;
}

bool capture_grayscale_image(CameraContext &ctx, std::vector<uint8_t> &image_out) {
    using namespace libcamera;

    std::unique_ptr<Request> request = ctx.camera->createRequest();
    if (!request) {
        std::cerr << "Failed to create request\n";
        return false;
    }

    FrameBuffer *fb = ctx.buffers->at(0).get();
    if (request->addBuffer(ctx.stream, fb) < 0) {
        std::cerr << "Failed to add buffer to request\n";
        return false;
    }

    if (ctx.camera->start() < 0) {
        std::cerr << "Failed to start camera.\n";
        return false;
    }

    if (ctx.camera->queueRequest(request.get()) < 0) {
        std::cerr << "Failed to queue request.\n";
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ctx.camera->stop();

    const FrameBuffer::Plane &plane = fb->planes()[0];
    int fd = plane.fd.get();
    void *raw = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, fd, 0);
    if (raw == MAP_FAILED) {
        std::cerr << "Failed to map framebuffer.\n";
        return false;
    }

    size_t stride = ctx.config->at(0).stride;
    size_t height = ctx.height;

    image_out.resize(ctx.width * height);

    uint8_t *src = static_cast<uint8_t *>(raw);
    uint8_t *dst = image_out.data();

    for (int y = 0; y < height; ++y) {
        std::memcpy(dst + y * ctx.width, src + y * stride, ctx.width);
    }

    munmap(raw, plane.length);
    return true;
}

void cleanup_camera(CameraContext &ctx) {
    if (ctx.camera)
        ctx.camera->release();
    ctx.allocator.reset();
    ctx.buffers = nullptr;
}
