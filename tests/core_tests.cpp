#include "ImageProcessor.hpp"

#include <cassert>
#include <iostream>

using namespace r387;

static Raster solidImage(int w, int h, Pixel color = {240, 10, 30, 255}) {
    Raster img;
    img.width = w;
    img.height = h;
    img.pixels.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), color);
    return img;
}

int main() {
    {
        auto img = solidImage(64, 64);
        Settings s; s.objectLimit = 100;
        auto p = makePlan(img, s);
        assert(p.valid());
        assert(p.cells.size() <= 100);
        assert(!p.cells.empty());
    }
    {
        auto img = solidImage(8, 8);
        Settings s; s.objectLimit = 1;
        auto p = makePlan(img, s);
        assert(p.valid());
        assert(p.cells.size() == 1);
    }
    {
        auto img = solidImage(10, 10, {0, 0, 0, 0});
        Settings s; s.objectLimit = 750;
        auto p = makePlan(img, s);
        assert(p.valid());
        assert(p.cells.empty()); // transparent cells should not become objects
    }
    {
        auto img = solidImage(100, 30);
        Settings s; s.unlimited = true; s.hardSafetyCap = 9000;
        auto p = makePlan(img, s);
        assert(p.valid());
        assert(p.budget <= kAbsoluteSafetyCap);
        assert(p.cells.size() <= kAbsoluteSafetyCap);
    }
    {
        Raster broken;
        auto p = makePlan(broken, Settings{});
        assert(!p.valid());
        assert(!p.error.empty());
    }
    std::cout << "R387 image processor tests passed.\n";
}
