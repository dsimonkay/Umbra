#ifndef UMBRA_APP_APP_STATE_H
#define UMBRA_APP_APP_STATE_H

#include "app/settings.h"
#include "core/framebuffer.h"

#include <raylib.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>

namespace umbra::app
{

using Clock = std::chrono::steady_clock;

/// @brief Everything the application remembers between frames.
///
/// raylib is a C library, so its types (Texture2D, Image, Color, Ray, ...) live in the global
/// namespace. Inside namespace umbra we write them with a leading "::", so that an umbra type
/// with the same name (umbra::Color already exists) can never be picked up by accident.
struct AppState
{
    Framebuffer framebuffer;

    // A raw raylib texture handle: whoever owns it must call UnloadTexture().
    // M1 side quest: wrap it into a small RAII class (rule of five, move-only).
    ::Texture2D texture{};
    bool texture_dirty{false};

    std::size_t resolution_index{1}; // into resolutions (app/render_job.h)
    bool slow_motion{false};

    int next_row{0};                              // first row that is not rendered yet
    std::chrono::duration<double> compute_time{}; // time spent rendering (not drawing the UI)

    std::string status{"Ready"};
    Clock::time_point status_time{};

    Settings settings;                   // what the user chose (see app/settings.h)
    std::filesystem::path settings_path; // where to save it; empty: don't save (screenshot mode)
    float applied_ui_scale{0.0f};        // the scale the theme is currently built for

    bool show_imgui_demo{false};
    bool open_about{false};
    bool reset_layout{false};
    bool quit{false};
};

/// @brief Show a message in the status bar. It stays there until the next one.
inline void SetStatus(AppState &app, std::string message)
{
    app.status = std::move(message);
    app.status_time = Clock::now();
}

} // namespace umbra::app

#endif // UMBRA_APP_APP_STATE_H
