#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "stb_image_write.h"

#include <GLFW/glfw3.h>

class VisualTestHarness {
public:
    using CaptureCallback =
        std::function<void(int width, int height, void* pixelBuffer)>;

    bool init(int width, int height) {
        m_width = width;
        m_height = height;

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        m_window = glfwCreateWindow(width, height, "Visual Tests", nullptr, nullptr);
        if (!m_window) {
            std::cerr << "[harness] Failed to create GLFW window" << std::endl;
            return false;
        }

        glfwMakeContextCurrent(m_window);
        return true;
    }

    void shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void setCaptureCallback(CaptureCallback callback) {
        m_captureCallback = std::move(callback);
    }

    GLFWwindow* getWindow() const { return m_window; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    bool captureToFile(const std::string& path) {
        const int channels = 3;
        const int rowBytes = m_width * channels;
        std::vector<uint8_t> pixels(static_cast<size_t>(rowBytes) * m_height);

        // TODO: hook your backend's readback here
        // The default implementation calls the user-provided callback.
        // For a real GL backend, you would set a callback like:
        //   harness.setCaptureCallback([](int w, int h, void* buf) {
        //       glFinish();
        //       glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf);
        //   });
        if (m_captureCallback) {
            m_captureCallback(m_width, m_height, pixels.data());
        } else {
            std::cerr << "[harness] No capture callback set — output will be blank"
                      << std::endl;
            std::memset(pixels.data(), 0, pixels.size());
        }

        // Flip rows: GL gives bottom-left origin, PNG expects top-left
        std::vector<uint8_t> row(rowBytes);
        for (int y = 0; y < m_height / 2; ++y) {
            uint8_t* top = pixels.data() + y * rowBytes;
            uint8_t* bot = pixels.data() + (m_height - 1 - y) * rowBytes;
            std::memcpy(row.data(), top, rowBytes);
            std::memcpy(top, bot, rowBytes);
            std::memcpy(bot, row.data(), rowBytes);
        }

        // Ensure output directory exists
        auto parentDir = std::filesystem::path(path).parent_path();
        if (!parentDir.empty()) {
            std::filesystem::create_directories(parentDir);
        }

        if (!stbi_write_png(path.c_str(), m_width, m_height, channels,
                            pixels.data(), rowBytes)) {
            std::cerr << "[harness] Failed to write PNG: " << path << std::endl;
            return false;
        }

        return true;
    }

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
    CaptureCallback m_captureCallback;
};
