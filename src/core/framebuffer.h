#ifndef UMBRA_CORE_FRAMEBUFFER_H
#define UMBRA_CORE_FRAMEBUFFER_H

#include "core/pixel.h"

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

namespace umbra
{

/// @brief A width x height grid of pixels, stored row by row, top row first.
/// @note No user-declared copy/move/destructor: std::vector already does the
///       right thing, so the compiler-generated ones are correct ("rule of zero").
class Framebuffer
{
public:
    Framebuffer() = default;

    Framebuffer(const int width, const int height) { Resize(width, height); }

    /// @brief Change the size. The content is reset to black.
    void Resize(const int width, const int height)
    {
        assert(width >= 0 && height >= 0);
        width_ = width;
        height_ = height;
        pixels_.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Pixel{});
    }

    [[nodiscard]] int Width() const noexcept { return width_; }
    [[nodiscard]] int Height() const noexcept { return height_; }

    void PutPixel(const int x, const int y, const Pixel pixel) { pixels_[Index(x, y)] = pixel; }
    [[nodiscard]] Pixel GetPixel(const int x, const int y) const { return pixels_[Index(x, y)]; }

    /// @brief All pixels, row by row, e.g. for uploading to a texture.
    [[nodiscard]] std::span<const Pixel> Pixels() const noexcept { return pixels_; }

    /// @brief Raw access for C APIs, e.g. writing a file.
    [[nodiscard]] const Pixel *Data() const noexcept { return pixels_.data(); }

private:
    [[nodiscard]] std::size_t Index(const int x, const int y) const
    {
        assert((x >= 0) && (x < width_) && (y >= 0) && (y < height_));
        return (static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)) + static_cast<std::size_t>(x);
    }

    int width_{0};
    int height_{0};
    std::vector<Pixel> pixels_;
};

} // namespace umbra

#endif // UMBRA_CORE_FRAMEBUFFER_H
