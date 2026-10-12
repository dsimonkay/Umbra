#include "app/settings.h"

#include <charconv>
#include <cstdlib>
#include <format>
#include <fstream>
#include <memory>
#include <system_error>

namespace umbra::app
{

namespace
{

std::optional<std::string> ReadEnvironmentVariable(const char *name)
{
#if defined(_MSC_VER)
    // MSVC warns about std::getenv (C4996); _dupenv_s is its safe replacement.
    char *buffer{nullptr};
    std::size_t size{0};
    if (_dupenv_s(&buffer, &size, name) != 0 || buffer == nullptr)
    {
        return std::nullopt;
    }
    const std::unique_ptr<char, decltype(&std::free)> owner{buffer, &std::free};
    return std::string{buffer};
#else
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return std::nullopt;
    }
    return std::string{value};
#endif
}

std::string_view Trim(const std::string_view text)
{
    constexpr std::string_view whitespace{" \t\r\n"};
    const auto first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos)
    {
        return {};
    }
    const auto last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

} // namespace

std::filesystem::path ConfigDirectory()
{
#if defined(_WIN32)
    if (const auto app_data = ReadEnvironmentVariable("APPDATA"))
    {
        return std::filesystem::path{*app_data} / "Umbra";
    }
#else
    if (const auto xdg_config_home = ReadEnvironmentVariable("XDG_CONFIG_HOME"))
    {
        return std::filesystem::path{*xdg_config_home} / "umbra";
    }
    if (const auto home = ReadEnvironmentVariable("HOME"))
    {
        return std::filesystem::path{*home} / ".config" / "umbra";
    }
#endif
    return std::filesystem::current_path(); // last resort
}

Settings LoadSettings(const std::filesystem::path &path)
{
    Settings settings{};

    std::ifstream file{path};
    std::string line;
    while (std::getline(file, line))
    {
        const std::string_view text = Trim(line);
        const auto equals = text.find('=');
        if (text.empty() || text.starts_with('#') || equals == std::string_view::npos)
        {
            continue;
        }

        const std::string_view key = Trim(text.substr(0, equals));
        const std::string_view value = Trim(text.substr(equals + 1));
        if (key == "ui_scale")
        {
            if (const auto scale = ParseUiScale(value))
            {
                settings.ui_scale = *scale;
            }
        }
    }
    return settings;
}

std::expected<void, std::string> SaveSettings(const Settings &settings, const std::filesystem::path &path)
{
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error)
    {
        return std::unexpected{std::format("cannot create '{}': {}", path.parent_path().string(), error.message())};
    }

    std::ofstream file{path};
    file << "# Umbra settings. The app rewrites this file when you change a setting in it.\n"
         << "#\n"
         << "# ui_scale: auto  = follow the system (Xft.dpi on Linux and WSLg, display settings on Windows)\n"
         << "#           1.5   = always 150 %\n"
         << "ui_scale = " << (settings.ui_scale ? std::format("{:.2f}", *settings.ui_scale) : "auto") << '\n';

    if (!file)
    {
        return std::unexpected{std::format("cannot write '{}'", path.string())};
    }
    return {};
}

std::expected<std::optional<float>, std::string> ParseUiScale(std::string_view text)
{
    text = Trim(text);
    if (text == "auto")
    {
        return std::optional<float>{};
    }

    const bool percent = text.ends_with('%');
    const std::string_view number = percent ? text.substr(0, text.size() - 1) : text;

    float value{0.0f};
    const auto [end, error] = std::from_chars(number.data(), number.data() + number.size(), value);
    if (error != std::errc{} || end != number.data() + number.size() || !(value > 0.0f))
    {
        return std::unexpected{std::format("'{}' is not a UI scale (try 1.5, 150% or auto)", text)};
    }
    return percent ? value / 100.0f : value;
}

} // namespace umbra::app
