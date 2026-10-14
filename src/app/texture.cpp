#include "app/texture.h"

#include <raylib.h>

#include <cassert>
#include <cstddef>

namespace umbra
{

const ::Texture2D &Texture::Get() const
{
    return texture_;
}

void Texture::Upload(const std::span<const Pixel> pixels)
{
    // UpdateTexture() reads width * height pixels in the texture's format, whatever the buffer holds.
    assert(texture_.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    assert(pixels.size() == static_cast<std::size_t>(texture_.width) * static_cast<std::size_t>(texture_.height));

    UpdateTexture(texture_, pixels.data());
}

void Texture::Unload() noexcept
{
    if (IsTextureValid(texture_))
    {
        UnloadTexture(texture_);
    }

    texture_ = {};
}

} // namespace umbra
