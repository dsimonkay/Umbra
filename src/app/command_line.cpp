#include "app/command_line.h"

#include <string_view>

namespace umbra::app
{

namespace
{

/// The value after a command-line option, e.g. "a.png" for "--screenshot a.png".
std::optional<std::string_view> OptionValue(const int argc, char **argv, const std::string_view option)
{
    for (int i{1}; i + 1 < argc; ++i)
    {
        if (argv[i] == option)
        {
            return std::string_view{argv[i + 1]};
        }
    }
    return std::nullopt;
}

} // namespace

CommandLine ParseCommandLine(const int argc, char **argv)
{
    CommandLine command_line{};
    if (const auto value = OptionValue(argc, argv, "--screenshot"))
    {
        command_line.screenshot_path = std::filesystem::path{*value};
    }
    if (const auto value = OptionValue(argc, argv, "--ui-scale"))
    {
        command_line.ui_scale = std::string{*value};
    }
    return command_line;
}

} // namespace umbra::app
