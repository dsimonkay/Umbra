#ifndef UMBRA_APP_SETTINGS_HPP
#define UMBRA_APP_SETTINGS_HPP

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace umbra::app
{

/// @brief User preferences that survive a restart. Stored as a small text file:
///
///     # comment
///     ui_scale = auto
///
/// Unknown keys and broken lines are ignored, so an older or newer version of the
/// app can read the same file.
struct Settings
{
    /// A fixed UI scale (1.5 = 150 %), or empty: automatic, i.e. follow the system
    /// (Xft.dpi on Linux and under WSLg, the display settings on Windows).
    std::optional<float> ui_scale;
};

/// @brief Where the settings and the panel layout are kept:
///        $XDG_CONFIG_HOME/umbra or ~/.config/umbra on Linux, %APPDATA%\Umbra on Windows.
[[nodiscard]] std::filesystem::path ConfigDirectory();

/// @brief Read the settings file. A missing file or a bad line simply means "use the default".
[[nodiscard]] Settings LoadSettings(const std::filesystem::path &path);

/// @brief Write the settings file, creating its directory if needed.
[[nodiscard]] std::expected<void, std::string> SaveSettings(const Settings &settings,
                                                            const std::filesystem::path &path);

/// @brief Parse a UI scale: "auto" -> automatic (empty optional), "1.5" or "150%" -> 1.5.
/// @return the value, or an error message for anything else.
[[nodiscard]] std::expected<std::optional<float>, std::string> ParseUiScale(std::string_view text);

} // namespace umbra::app

#endif // UMBRA_APP_SETTINGS_HPP
