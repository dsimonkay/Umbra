#ifndef UMBRA_IO_IMAGE_IO_HPP
#define UMBRA_IO_IMAGE_IO_HPP

#include "core/framebuffer.hpp"

#include <expected>
#include <filesystem>
#include <string>

namespace umbra
{

/// @brief Write the framebuffer to a PNG file. Missing parent directories are created.
/// @return nothing on success, or a human-readable error message.
[[nodiscard]] std::expected<void, std::string> write_png(const Framebuffer &framebuffer,
                                                         const std::filesystem::path &path);

/// @brief A file name like "umbra-20261011-213045.png" (local time), for snapshots.
[[nodiscard]] std::string timestamped_file_name(const std::string &prefix = "umbra");

} // namespace umbra

#endif // UMBRA_IO_IMAGE_IO_HPP
