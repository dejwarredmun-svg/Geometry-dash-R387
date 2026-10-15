#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace r387 {

struct Pixel {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 0;
};

struct Raster {
    int width = 0;
    int height = 0;
    std::vector<Pixel> pixels; // row-major, top-to-bottom RGBA

    bool valid() const noexcept;
    const Pixel& at(int x, int y) const;
};

struct Settings {
    std::size_t objectLimit = 750;
    bool unlimited = false;
    std::size_t hardSafetyCap = 2500;
    std::uint8_t alphaThreshold = 24;
    std::uint8_t quantizationStep = 32;
};

struct Cell {
    int x = 0;
    int y = 0;
    Pixel color;
};

struct Plan {
    int columns = 0;
    int rows = 0;
    std::size_t budget = 0;
    std::vector<Cell> cells;
    std::string error;

    bool valid() const noexcept { return error.empty() && columns > 0 && rows > 0; }
};

inline constexpr std::size_t kAbsoluteSafetyCap = 2500;
inline constexpr int kMaxImageDimension = 4096;

Plan makePlan(const Raster& image, const Settings& settings);

} // namespace r387
