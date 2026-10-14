#ifndef UMBRA_APP_COMMAND_LINE_OPTIONS_H
#define UMBRA_APP_COMMAND_LINE_OPTIONS_H

#include <filesystem>
#include <optional>
#include <string>

namespace umbra::app
{

/// @brief What the command line asks for:
///
///     umbra                      normal start
///     umbra --ui-scale 1.5       use this UI scale for this run (also "150%" or "auto")
///     umbra --screenshot a.png   render, save a picture of the whole window, quit
///                                (for the docs, and so Claude Code can "see" the UI)
///
/// Anything else on the command line is ignored.
struct CommandLineOptions
{
    std::optional<std::filesystem::path> screenshot_path;

    /// As typed. It is parsed (ParseUiScale(), app/settings.h) after the settings are loaded,
    /// because it overrides them for this run.
    std::optional<std::string> ui_scale;
};

[[nodiscard]] CommandLineOptions ParseCommandLineOptions(int argc, char **argv);

} // namespace umbra::app

#endif // UMBRA_APP_COMMAND_LINE_OPTIONS_H
