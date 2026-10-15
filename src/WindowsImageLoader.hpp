#pragma once

#include "ImageProcessor.hpp"
#include <filesystem>
#include <string>

namespace r387 {
struct LoadResult {
    Raster image;
    std::string error;
    explicit operator bool() const noexcept { return error.empty() && image.valid(); }
};

LoadResult loadImageWindows(const std::filesystem::path& path);
std::filesystem::path chooseImageFile();
} // namespace r387
