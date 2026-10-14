#ifndef UMBRA_APP_TEXTURE_H
#define UMBRA_APP_TEXTURE_H

#include "core/pixel.h"

#include <raylib.h>

#include <span>
#include <utility>

namespace umbra
{

/// @brief RAII raylib Texture wrapper.
class Texture
{
public:
    Texture() = default;
    explicit Texture(::Texture2D texture) : texture_{texture} {}
    ~Texture() { Unload(); }

    Texture(const Texture &other) = delete;
    Texture &operator=(const Texture &other) = delete;

    Texture(Texture &&other) noexcept : texture_{std::exchange(other.texture_, ::Texture2D{})} {}

    Texture &operator=(Texture &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        Unload();
        texture_ = std::exchange(other.texture_, ::Texture2D{});

        return *this;
    }

    const ::Texture2D &Get() const;

    /// @brief Copy the pixels into the texture.
    /// @pre The texture is RGBA8 (like Pixel), and pixels.size() is its width * height.
    void Upload(std::span<const Pixel> pixels);

private:
    void Unload() noexcept;

    ::Texture2D texture_{};
};

} // namespace umbra

#endif // UMBRA_APP_TEXTURE_H
