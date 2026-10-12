#include "io/image_io.hpp"

#include <raylib.h>

#include <chrono>
#include <format>
#include <system_error>

namespace umbra
{

std::expected<void, std::string> WritePng(const Framebuffer &framebuffer, const std::filesystem::path &path)
{
    if (framebuffer.width() == 0 || framebuffer.height() == 0)
    {
        return std::unexpected{std::string{"the framebuffer is empty"}};
    }

    if (path.has_parent_path())
    {
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error)
        {
            return std::unexpected{std::format("cannot create '{}': {}", path.parent_path().string(), error.message())};
        }
    }

    // raylib's Image is a plain C struct that does not own anything here:
    // it just points at our pixels for the duration of the call.
    const Image image{
        .data = const_cast<Pixel *>(framebuffer.data()), // ExportImage only reads it
        .width = framebuffer.width(),
        .height = framebuffer.height(),
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };

    if (!ExportImage(image, path.string().c_str()))
    {
        return std::unexpected{std::format("raylib could not write '{}'", path.string())};
    }

    return {};
}

std::string GetTimestampedFileName(const std::string &prefix)
{
    using namespace std::chrono;

    const auto now = floor<seconds>(system_clock::now());
    try
    {
        return std::format("{}-{:%Y%m%d-%H%M%S}.png", prefix, zoned_time{current_zone(), now}.get_local_time());
    }
    catch (const std::exception &)
    {
        // No time zone database available (rare): fall back to UTC.
        return std::format("{}-{:%Y%m%d-%H%M%S}.png", prefix, now);
    }
}

} // namespace umbra
