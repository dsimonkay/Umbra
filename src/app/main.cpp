// umbra: the GUI application.
//
// Layout: a main menu on top, a status bar at the bottom, and two dockable
// panels in between: "Viewport" (the rendered image) and "Render" (settings
// and progress). Panels can be dragged around; View > Reset layout restores them.
//
// This file starts the app, runs the main loop and shuts it down. Everything else lives in:
//   app/app_state.h       AppState: everything the app remembers between frames
//   app/render_job.cpp    rendering a few rows per frame on this (UI) thread; Save PNG
//   app/panels/           one file per window: menu (+ shortcuts), status bar, dock space, panels
//   app/ui_scale.cpp      which UI scale is in effect, and changing it
//   app/settings.cpp      what survives a restart
//   app/command_line_options.cpp  --screenshot, --ui-scale
//   ui/                   the look (colors, fonts, scaling) and small widgets; no app logic
//
// Settings (the UI scale) and the panel layout are kept in the user's config directory,
// see app/settings.h. Screenshot mode ignores both, so its pictures are reproducible.

#include "app/app_state.h"
#include "app/command_line_options.h"
#include "app/panels/panels.h"
#include "app/render_job.h"
#include "app/settings.h"
#include "app/ui_scale.h"
#include "ui/theme.h"

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

#include <filesystem>
#include <string>
#include <system_error>

namespace umbra::app
{

namespace
{

/// Saves what is currently on screen (call it after the UI is drawn, before EndDrawing).
void SaveWindowScreenshot(const std::filesystem::path &path)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
    const ::Image screen = LoadImageFromScreen();
    ExportImage(screen, path.string().c_str());
    UnloadImage(screen);
}

/// The whole life of the GUI: open the window, run the main loop, clean up.
int Run(const CommandLineOptions &command_line_options)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1600, 900, "Umbra");
    SetWindowMinSize(960, 540);
    SetExitKey(KEY_NULL); // Esc should not close the app
    SetTargetFPS(60);
    if (!command_line_options.screenshot_path)
    {
        MaximizeWindow();
    }

    ui::InstallFontLoader();
    rlImGuiSetup(true);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    std::string layout_path; // ImGui keeps a pointer to it (io.IniFilename) until rlImGuiShutdown()

    /// @note AppState holds a Texture RAII instance, which will be cleaned up (=unloaded)
    ///       automatically once the `app` instance goes out of scope. However... it still requires
    ///       an existing gl context, which would not be provided in case the `app` instance is
    ///       declared in the scope of Run(). Sadly, this is an ugly workaround for the moment.
    /// @todo make the window a resource as well
    {
        AppState app{};

        // Settings and panel layout live in the config directory (e.g. ~/.config/umbra).
        // Screenshot mode uses neither, so it always starts from the defaults.
        if (!command_line_options.screenshot_path)
        {
            const std::filesystem::path config_directory = ConfigDirectory();
            std::error_code error;
            std::filesystem::create_directories(config_directory, error); // ImGui needs it to save the layout
            app.settings_path = config_directory / "settings.ini";
            app.settings = LoadSettings(app.settings_path);
            layout_path = (config_directory / "layout.ini").string();
        }
        io.IniFilename = layout_path.empty() ? nullptr : layout_path.c_str();

        // --ui-scale only applies to this run; it is saved only if you change the scale in the app.
        if (command_line_options.ui_scale)
        {
            if (const auto scale = ParseUiScale(*command_line_options.ui_scale))
            {
                app.settings.ui_scale = *scale;
            }
            else
            {
                TraceLog(LOG_WARNING, "--ui-scale: %s", scale.error().c_str());
            }
        }

        RestartRender(app);

        int finished_frames{0};
        while (!WindowShouldClose() && !app.quit)
        {
            // (Re)build the theme when the UI scale changes: a new setting, or (in automatic
            // mode) the window moved to a monitor with a different scale.
            if (const float scale = EffectiveUiScale(app); scale != app.applied_ui_scale)
            {
                ui::ApplyTheme(scale);
                app.applied_ui_scale = scale;
            }

            AdvanceRender(app);
            UploadTexture(app);

            BeginDrawing();
            ClearBackground(ui::kClearColor);

            rlImGuiBegin();
            HandleShortcuts(app);
            DrawMainMenu(app);
            DrawStatusBar(app);
            DrawDockspace(app);
            DrawViewport(app);
            DrawRenderPanel(app);
            DrawAbout(app);
            if (app.show_imgui_demo)
            {
                ImGui::ShowDemoWindow(&app.show_imgui_demo);
            }
            rlImGuiEnd();

            // Give the layout a few frames to settle after the render has finished.
            if (command_line_options.screenshot_path && !IsRendering(app) && ++finished_frames == 10)
            {
                SaveWindowScreenshot(*command_line_options.screenshot_path);
                app.quit = true;
            }

            EndDrawing();
        }
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}

} // namespace

} // namespace umbra::app

int main(int argc, char **argv)
{
    return umbra::app::Run(umbra::app::ParseCommandLineOptions(argc, argv));
}
