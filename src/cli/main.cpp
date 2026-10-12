// umbra-cli: render the current scene straight into a PNG file, without a window.
//
//   umbra-cli [--width N] [--height N] [--out path.png]
//
// Useful for quick checks and benchmarks, and it lets Claude Code look at the
// result of a change (it can open PNG files, but it cannot see your window).

#include "core/framebuffer.hpp"
#include "core/render.hpp"
#include "io/image_io.hpp"

#include <raylib.h>

#include <charconv>
#include <chrono>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

namespace
{

/// @brief  Command-line options for the umbra-cli application.
struct Options
{
    int width{960};                                  //< The width of the rendered image.
    int height{540};                                 //< The height of the rendered image.
    std::filesystem::path output{"renders/cli.png"}; //< The output file path for the rendered image.
};

constexpr std::string_view usage{"usage: umbra-cli [--width N] [--height N] [--out path.png]\n"};

std::optional<int> ParsePositiveInt(const std::string_view text)
{
    int value{0};
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() || value <= 0)
    {
        return std::nullopt;
    }
    return value;
}

/// @return the parsed options, or the exit code to quit with
///         (EXIT_SUCCESS after --help, EXIT_FAILURE after an error).
std::expected<Options, int> ParseOptions(const std::vector<std::string_view> &args)
{
    Options options{};
    for (std::size_t i{0}; i < args.size(); ++i)
    {
        const std::string_view arg = args[i];
        if (arg == "--help" || arg == "-h")
        {
            std::cout << usage;
            return std::unexpected{EXIT_SUCCESS};
        }
        if (arg != "--width" && arg != "--height" && arg != "--out")
        {
            std::cerr << "error: unknown option '" << arg << "'\n" << usage;
            return std::unexpected{EXIT_FAILURE};
        }
        if (i + 1 >= args.size())
        {
            std::cerr << "error: '" << arg << "' needs a value\n" << usage;
            return std::unexpected{EXIT_FAILURE};
        }

        const std::string_view value = args[++i];
        if (arg == "--out")
        {
            options.output = std::filesystem::path{value};
            continue;
        }

        const auto number = ParsePositiveInt(value);
        if (!number)
        {
            std::cerr << "error: '" << value << "' is not a positive integer\n";
            return std::unexpected{EXIT_FAILURE};
        }
        else if (*number < 2)
        {
            std::cerr << "error: " << (arg == "--width" ? "width" : "height") << " must be at least 2\n";
            return std::unexpected{EXIT_FAILURE};
        }

        (arg == "--width" ? options.width : options.height) = *number;
    }

    return options;
}

} // namespace

int main(int argc, char **argv)
{
    const auto options = ParseOptions(std::vector<std::string_view>(argv + 1, argv + argc));
    if (!options)
    {
        return options.error();
    }

    SetTraceLogLevel(LOG_WARNING); // keep raylib quiet

    umbra::Framebuffer framebuffer{options->width, options->height};

    const auto start = std::chrono::steady_clock::now();
    for (int y{0}; y < framebuffer.Height(); ++y)
    {
        for (int x{0}; x < framebuffer.Width(); ++x)
        {
            framebuffer.PutPixel(x, y, umbra::RenderPixel(x, y, framebuffer.Width(), framebuffer.Height()));
        }
    }
    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;

    if (const auto written = umbra::WritePng(framebuffer, options->output); !written)
    {
        std::cerr << "error: " << written.error() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "rendered " << framebuffer.Width() << "x" << framebuffer.Height() << " in " << elapsed.count()
              << " s -> " << options->output.string() << '\n';
    return EXIT_SUCCESS;
}
