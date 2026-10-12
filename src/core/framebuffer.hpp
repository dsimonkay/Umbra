#ifndef UMBRA_CORE_FRAMEBUFFER_HPP
#define UMBRA_CORE_FRAMEBUFFER_HPP

#include "core/pixel.hpp"

#include <cassert>
#include <cstddef>
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

    Framebuffer(const int width, const int height) { resize(width, height); }

    /// @brief Change the size. The content is reset to black.
    void resize(const int width, const int height)
    {
        assert(width >= 0 && height >= 0);
        width_ = width;
        height_ = height;
        pixels_.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Pixel{});
    }

    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }

    void put_pixel(const int x, const int y, const Pixel pixel) { pixels_[index(x, y)] = pixel; }
    [[nodiscard]] Pixel get_pixel(const int x, const int y) const { return pixels_[index(x, y)]; }

    /// @brief Raw access, e.g. for uploading to a texture or writing a file.
    [[nodiscard]] const Pixel *data() const noexcept { return pixels_.data(); }

private:
    [[nodiscard]] std::size_t index(const int x, const int y) const
    {
        assert((x >= 0) && (x < width_) && (y >= 0) && (y < height_));
        return (static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)) + static_cast<std::size_t>(x);
    }

    int width_{0};
    int height_{0};
    std::vector<Pixel> pixels_;
};

} // namespace umbra

#endif // UMBRA_CORE_FRAMEBUFFER_HPP
