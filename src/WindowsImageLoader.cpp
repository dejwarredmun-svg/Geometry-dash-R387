#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <gdiplus.h>
#include <objidl.h>

#include "WindowsImageLoader.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comdlg32.lib")

namespace r387 {
namespace {
class GdiPlusRuntime {
public:
    GdiPlusRuntime() {
        Gdiplus::GdiplusStartupInput input;
        ok = Gdiplus::GdiplusStartup(&token, &input, nullptr) == Gdiplus::Ok;
    }
    ~GdiPlusRuntime() { if (ok) Gdiplus::GdiplusShutdown(token); }
    bool ok = false;
private:
    ULONG_PTR token = 0;
};

GdiPlusRuntime& gdiRuntime() {
    static GdiPlusRuntime runtime;
    return runtime;
}
}

std::filesystem::path chooseImageFile() {
    std::vector<wchar_t> filename(32768, L'\0');
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename.data();
    ofn.nMaxFile = static_cast<DWORD>(filename.size());
    ofn.lpstrFilter = L"Image files\0*.png;*.jpg;*.jpeg;*.bmp\0PNG files\0*.png\0JPEG files\0*.jpg;*.jpeg\0Bitmap files\0*.bmp\0All files\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
    ofn.lpstrTitle = L"Choose an image for R387 Agent";
    if (!GetOpenFileNameW(&ofn)) return {};
    return std::filesystem::path(filename.data());
}

LoadResult loadImageWindows(const std::filesystem::path& path) {
    LoadResult result;
    if (path.empty()) {
        result.error = "No image was selected.";
        return result;
    }
    if (!gdiRuntime().ok) {
        result.error = "Windows image decoding could not initialize.";
        return result;
    }

    Gdiplus::Bitmap bitmap(path.c_str(), FALSE);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        result.error = "Could not decode that image. Try a valid PNG, JPG, JPEG, or BMP.";
        return result;
    }
    const UINT width = bitmap.GetWidth();
    const UINT height = bitmap.GetHeight();
    if (width == 0 || height == 0 || width > kMaxImageDimension || height > kMaxImageDimension) {
        result.error = "Image dimensions must be between 1 and 4096 pixels per side.";
        return result;
    }
    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (count > static_cast<std::size_t>(kMaxImageDimension) * kMaxImageDimension) {
        result.error = "Image is too large to process safely.";
        return result;
    }

    Gdiplus::Rect rect(0, 0, static_cast<INT>(width), static_cast<INT>(height));
    Gdiplus::BitmapData data{};
    const auto status = bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
    if (status != Gdiplus::Ok) {
        result.error = "The image could not be read into a pixel buffer.";
        return result;
    }

    result.image.width = static_cast<int>(width);
    result.image.height = static_cast<int>(height);
    result.image.pixels.resize(count);
    const auto* base = static_cast<const std::uint8_t*>(data.Scan0);
    for (UINT y = 0; y < height; ++y) {
        const auto* row = base + static_cast<std::ptrdiff_t>(y) * data.Stride;
        for (UINT x = 0; x < width; ++x) {
            // GDI+ PixelFormat32bppARGB memory order on Windows is BGRA.
            const auto* p = row + static_cast<std::ptrdiff_t>(x) * 4;
            result.image.pixels[static_cast<std::size_t>(y) * width + x] = Pixel { p[2], p[1], p[0], p[3] };
        }
    }
    bitmap.UnlockBits(&data);
    return result;
}
} // namespace r387
