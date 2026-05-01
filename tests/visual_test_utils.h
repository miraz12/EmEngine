#pragma once

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "stb_image.h"

struct DiffResult {
    bool passed;
    float diffPercent;
    float maxDiff;
};

inline DiffResult compareImages(const std::string& refPath,
                                const std::string& actualPath,
                                float threshold = 5.0f,
                                float maxFailPercent = 0.01f) {
    DiffResult result{false, 100.0f, 255.0f};

    if (!std::filesystem::exists(refPath)) {
        std::cerr << "[visual_test_utils] Reference image not found: " << refPath
                  << std::endl;
        return result;
    }
    if (!std::filesystem::exists(actualPath)) {
        std::cerr << "[visual_test_utils] Actual image not found: " << actualPath
                  << std::endl;
        return result;
    }

    int refW, refH, refCh;
    uint8_t* refData = stbi_load(refPath.c_str(), &refW, &refH, &refCh, 3);
    if (!refData) {
        std::cerr << "[visual_test_utils] Failed to load reference: " << refPath
                  << std::endl;
        return result;
    }

    int actW, actH, actCh;
    uint8_t* actData = stbi_load(actualPath.c_str(), &actW, &actH, &actCh, 3);
    if (!actData) {
        std::cerr << "[visual_test_utils] Failed to load actual: " << actualPath
                  << std::endl;
        stbi_image_free(refData);
        return result;
    }

    if (refW != actW || refH != actH) {
        std::cerr << "[visual_test_utils] Size mismatch: reference " << refW << "x"
                  << refH << " vs actual " << actW << "x" << actH << std::endl;
        stbi_image_free(refData);
        stbi_image_free(actData);
        return result;
    }

    const int totalChannels = refW * refH * 3;
    int failedChannels = 0;
    float maxChannelDiff = 0.0f;

    for (int i = 0; i < totalChannels; ++i) {
        float diff = std::abs(static_cast<float>(refData[i]) -
                              static_cast<float>(actData[i]));
        if (diff > threshold) {
            ++failedChannels;
        }
        if (diff > maxChannelDiff) {
            maxChannelDiff = diff;
        }
    }

    stbi_image_free(refData);
    stbi_image_free(actData);

    result.diffPercent =
        (static_cast<float>(failedChannels) / static_cast<float>(totalChannels)) *
        100.0f;
    result.maxDiff = maxChannelDiff;
    result.passed = (result.diffPercent <= (maxFailPercent * 100.0f));

    return result;
}
