#include "ImageProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace r387 {

bool Raster::valid() const noexcept {
    if (width <= 0 || height <= 0 || width > kMaxImageDimension || height > kMaxImageDimension) return false;
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    return h <= std::numeric_limits<std::size_t>::max() / w && pixels.size() == w * h;
}

const Pixel& Raster::at(int x, int y) const {
    return pixels.at(static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x));
}

static std::uint8_t quantize(std::uint8_t value, std::uint8_t step) {
    if (step <= 1) return value;
    const unsigned rounded = (static_cast<unsigned>(value) + step / 2u) / step * step;
    return static_cast<std::uint8_t>(std::min(rounded, 255u));
}

static std::pair<int, int> chooseGrid(int width, int height, std::size_t budget) {
    if (budget <= 1) return {1, 1};
    const double aspect = static_cast<double>(width) / static_cast<double>(height);

    int bestW = 1;
    int bestH = 1;
    double bestScore = -std::numeric_limits<double>::infinity();
    const auto maxW = static_cast<int>(std::min<std::size_t>(budget, static_cast<std::size_t>(kMaxImageDimension)));

    for (int w = 1; w <= maxW; ++w) {
        int h = static_cast<int>(std::min<std::size_t>(budget / static_cast<std::size_t>(w), kMaxImageDimension));
        if (h < 1) continue;
        const auto used = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
        const double gridAspect = static_cast<double>(w) / static_cast<double>(h);
        const double aspectError = std::abs(std::log(gridAspect / aspect));
        // Prefer using most of the budget, while penalizing grid shapes that distort the source ratio.
        const double score = static_cast<double>(used) / static_cast<double>(budget) - aspectError * 0.20;
        if (score > bestScore) {
            bestScore = score;
            bestW = w;
            bestH = h;
        }
    }
    return {bestW, bestH};
}

Plan makePlan(const Raster& image, const Settings& settings) {
    Plan out;
    if (!image.valid()) {
        out.error = "Image is invalid or exceeds the 4096 × 4096 safety limit.";
        return out;
    }

    std::size_t budget = settings.objectLimit;
    if (settings.unlimited) {
        budget = std::min(settings.hardSafetyCap, kAbsoluteSafetyCap);
    } else {
        budget = std::max<std::size_t>(1, budget);
        budget = std::min(budget, kAbsoluteSafetyCap);
    }
    if (budget == 0) {
        out.error = "Object budget must be at least 1.";
        return out;
    }
    out.budget = budget;

    const auto [cols, rows] = chooseGrid(image.width, image.height, budget);
    out.columns = cols;
    out.rows = rows;
    out.cells.reserve(std::min(budget, static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows)));

    const std::uint8_t step = std::max<std::uint8_t>(1, settings.quantizationStep);
    for (int gy = 0; gy < rows; ++gy) {
        const int sy = std::min(image.height - 1,
            static_cast<int>((static_cast<std::int64_t>(2 * gy + 1) * image.height) / (2LL * rows)));
        for (int gx = 0; gx < cols; ++gx) {
            // This is a center sample per output cell; no output cell can create more than one object.
            const int sx = std::min(image.width - 1,
                static_cast<int>((static_cast<std::int64_t>(2 * gx + 1) * image.width) / (2LL * cols)));
            const auto& source = image.at(sx, sy);
            if (source.a < settings.alphaThreshold) continue;
            Pixel color {
                quantize(source.r, step),
                quantize(source.g, step),
                quantize(source.b, step),
                255
            };
            out.cells.push_back(Cell {gx, gy, color});
            if (out.cells.size() >= budget) return out;
        }
    }
    return out;
}

} // namespace r387
