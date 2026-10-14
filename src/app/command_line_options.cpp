#include "app/command_line_options.h"

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

CommandLineOptions ParseCommandLineOptions(const int argc, char **argv)
{
    CommandLineOptions command_line_options{};
    if (const auto value = OptionValue(argc, argv, "--screenshot"))
    {
        command_line_options.screenshot_path = std::filesystem::path{*value};
    }
    if (const auto value = OptionValue(argc, argv, "--ui-scale"))
    {
        command_line_options.ui_scale = std::string{*value};
    }
    return command_line_options;
}

} // namespace umbra::app
