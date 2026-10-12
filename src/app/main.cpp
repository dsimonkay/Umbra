// umbra: the GUI application.
//
// Layout: a main menu on top, a status bar at the bottom, and two dockable
// panels in between: "Viewport" (the rendered image) and "Render" (settings
// and progress). Panels can be dragged around; View > Reset layout restores them.
//
// Rendering happens on this (UI) thread for now: every frame we render as many
// rows as fit into a small time budget, so the window stays responsive and you
// can watch the image appear. Worker threads arrive in milestone M8.
//
// Command line:
//   umbra                      normal start
//   umbra --ui-scale 1.5       use this UI scale for this run (also "150%" or "auto")
//   umbra --screenshot a.png   render, save a picture of the whole window, quit
//                              (for the docs, and so Claude Code can "see" the UI)
//
// Settings (the UI scale) and the panel layout are kept in the user's config directory,
// see app/settings.hpp. Screenshot mode ignores both, so its pictures are reproducible.

#include "app/settings.hpp"
#include "app/theme.hpp"
#include "core/framebuffer.hpp"
#include "core/render.hpp"
#include "io/image_io.hpp"

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder*, BeginViewportSideBar
#include <raylib.h>
#include <rlImGui.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>

#ifndef UMBRA_VERSION
#define UMBRA_VERSION "dev"
#endif

namespace
{

using Clock = std::chrono::steady_clock;

struct Resolution
{
    const char *label;
    int width;
    int height;
};

constexpr std::array resolutions{
    Resolution{"640 x 360", 640, 360},
    Resolution{"960 x 540", 960, 540},
    Resolution{"1280 x 720 (HD)", 1280, 720},
    Resolution{"1920 x 1080 (Full HD)", 1920, 1080},
};

// "visible title###stable id": the part after ### identifies the window,
// so the icon or the text can change without breaking the saved layout.
constexpr const char *viewport_window_title{ICON_FA_IMAGE "  Viewport###Viewport"};
constexpr const char *render_window_title{ICON_FA_SLIDERS "  Render###Render"};

// UI scale: by default the system's (see ui::SystemScale()), or a fixed value chosen
// under View > UI scale. Useful under WSLg, where Windows' display scaling does not
// reach Linux apps.
constexpr std::array ui_scale_presets{1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
constexpr float min_ui_scale{0.75f};
constexpr float max_ui_scale{3.0f};
constexpr float ui_scale_step{0.25f};

/// Everything the application remembers between frames.
struct AppState
{
    umbra::Framebuffer framebuffer;

    // A raw raylib texture handle: whoever owns it must call UnloadTexture().
    // M1 side quest: wrap it into a small RAII class (rule of five, move-only).
    Texture2D texture{};
    bool texture_dirty{false};

    std::size_t resolution_index{1};
    bool slow_motion{false};

    int next_row{0};                              // first row that is not rendered yet
    std::chrono::duration<double> compute_time{}; // time spent rendering (not drawing the UI)

    std::string status{"Ready"};
    Clock::time_point status_time{};

    umbra::app::Settings settings;       // what the user chose (see app/settings.hpp)
    std::filesystem::path settings_path; // where to save it; empty: don't save (screenshot mode)
    float applied_ui_scale{0.0f};        // the scale the theme is currently built for

    bool show_imgui_demo{false};
    bool open_about{false};
    bool reset_layout{false};
    bool quit{false};
};

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

[[nodiscard]] bool IsRendering(const AppState &app)
{
    return app.next_row < app.framebuffer.Height();
}

[[nodiscard]] float RenderProgress(const AppState &app)
{
    const int height = app.framebuffer.Height();
    return height > 0 ? static_cast<float>(app.next_row) / static_cast<float>(height) : 0.0f;
}

void SetStatus(AppState &app, std::string message)
{
    app.status = std::move(message);
    app.status_time = Clock::now();
}

/// Start rendering from the first row; resize the image and the texture if needed.
void RestartRender(AppState &app)
{
    const Resolution &resolution = resolutions[app.resolution_index];
    if (resolution.width != app.framebuffer.Width() || resolution.height != app.framebuffer.Height())
    {
        app.framebuffer.Resize(resolution.width, resolution.height);

        if (IsTextureValid(app.texture))
        {
            UnloadTexture(app.texture);
        }
        const Image blank = GenImageColor(resolution.width, resolution.height, BLACK); // RGBA8, like Pixel
        app.texture = LoadTextureFromImage(blank);
        UnloadImage(blank);
        SetTextureFilter(app.texture, TEXTURE_FILTER_BILINEAR);
    }

    app.next_row = 0;
    app.compute_time = {};
    SetStatus(app, "Rendering...");
}

/// Render rows until this frame's time budget is used up, so the UI stays smooth.
void AdvanceRender(AppState &app)
{
    if (!IsRendering(app))
    {
        return;
    }

    constexpr std::chrono::milliseconds frame_budget{12};
    const int width = app.framebuffer.Width();
    const int height = app.framebuffer.Height();
    const auto frame_start = Clock::now();

    while (IsRendering(app))
    {
        const int y = app.next_row;
        for (int x{0}; x < width; ++x)
        {
            app.framebuffer.PutPixel(x, y, umbra::RenderPixel(x, y, width, height));
        }
        ++app.next_row;

        if (app.slow_motion || Clock::now() - frame_start > frame_budget)
        {
            break;
        }
    }

    app.texture_dirty = true;
    app.compute_time += Clock::now() - frame_start;

    if (!IsRendering(app))
    {
        SetStatus(app, std::format("Rendered {} x {} in {:.3f} s", width, height, app.compute_time.count()));
    }
}

void UploadTexture(AppState &app)
{
    if (app.texture_dirty)
    {
        UpdateTexture(app.texture, app.framebuffer.Data());
        app.texture_dirty = false;
    }
}

void SaveSnapshot(AppState &app)
{
    const std::filesystem::path path = std::filesystem::path{"renders"} / umbra::GetTimestampedFileName();
    if (const auto written = umbra::WritePng(app.framebuffer, path); written)
    {
        SetStatus(app, std::format("Saved {}", path.string()));
    }
    else
    {
        SetStatus(app, std::format("Save failed: {}", written.error()));
    }
}

// ---------------------------------------------------------------------------
// UI scale
// ---------------------------------------------------------------------------

/// The UI scale in effect: the fixed one from the settings, or else the system's.
[[nodiscard]] float EffectiveUiScale(const AppState &app)
{
    return std::clamp(app.settings.ui_scale.value_or(umbra::ui::SystemScale()), min_ui_scale, max_ui_scale);
}

/// Change the UI scale setting (empty: automatic) and save it right away.
/// The theme picks up the new scale at the start of the next frame.
void SetUiScale(AppState &app, std::optional<float> scale)
{
    if (scale)
    {
        scale = std::clamp(*scale, min_ui_scale, max_ui_scale);
    }
    if (scale == app.settings.ui_scale)
    {
        return;
    }
    app.settings.ui_scale = scale;

    const std::string description =
        scale ? std::format("UI scale: {:.0f}%", *scale * 100.0f)
              : std::format("UI scale: automatic ({:.0f}%)", umbra::ui::SystemScale() * 100.0f);
    if (app.settings_path.empty())
    {
        SetStatus(app, description);
    }
    else if (const auto saved = umbra::app::SaveSettings(app.settings, app.settings_path); saved)
    {
        SetStatus(app, description + " (saved)");
    }
    else
    {
        SetStatus(app, std::format("{} (not saved: {})", description, saved.error()));
    }
}

// ---------------------------------------------------------------------------
// Small UI helpers: a two-column "label | widget" property grid
// ---------------------------------------------------------------------------

bool BeginProperties(const char *id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp))
    {
        return false;
    }
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, umbra::ui::Px(110.0f));
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

/// Starts a row: writes the label, then leaves the cursor in the value column.
void Property(const char *label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void PropertyText(const char *label, const std::string &value)
{
    Property(label);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(value.c_str());
}

// ---------------------------------------------------------------------------
// UI pieces
// ---------------------------------------------------------------------------

void HandleShortcuts(AppState &app)
{
    if (ImGui::Shortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal))
    {
        RestartRender(app);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
    {
        SaveSnapshot(app);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Q, ImGuiInputFlags_RouteGlobal))
    {
        app.quit = true;
    }

    // UI scale: Ctrl + numpad +/- (or the "=" / "-" keys of a US layout), Ctrl + numpad 0 = automatic.
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadAdd, ImGuiInputFlags_RouteGlobal) ||
        ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Equal, ImGuiInputFlags_RouteGlobal))
    {
        SetUiScale(app, EffectiveUiScale(app) + ui_scale_step);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadSubtract, ImGuiInputFlags_RouteGlobal) ||
        ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Minus, ImGuiInputFlags_RouteGlobal))
    {
        SetUiScale(app, EffectiveUiScale(app) - ui_scale_step);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Keypad0, ImGuiInputFlags_RouteGlobal))
    {
        SetUiScale(app, std::nullopt);
    }
}

void DrawMainMenu(AppState &app)
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save PNG", "Ctrl+S"))
        {
            SaveSnapshot(app);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Ctrl+Q"))
        {
            app.quit = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Render"))
    {
        if (ImGui::MenuItem("Re-render", "F5"))
        {
            RestartRender(app);
        }
        ImGui::MenuItem("Slow motion", nullptr, &app.slow_motion);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
        // "###UiScale" keeps the menu's ID stable while the number in its label changes.
        const std::string scale_menu = std::format("UI scale ({:.0f}%)###UiScale", EffectiveUiScale(app) * 100.0f);
        if (ImGui::BeginMenu(scale_menu.c_str()))
        {
            const std::string automatic = std::format("Automatic ({:.0f}%)", umbra::ui::SystemScale() * 100.0f);
            if (ImGui::MenuItem(automatic.c_str(), "Ctrl+Num 0", !app.settings.ui_scale.has_value()))
            {
                SetUiScale(app, std::nullopt);
            }
            ImGui::SetItemTooltip("Follow the system: Xft.dpi on Linux and WSLg (96 dpi = 100%%),\n"
                                  "the display settings on Windows.");
            ImGui::Separator();
            for (const float preset : ui_scale_presets)
            {
                const std::string label = std::format("{:.0f}%", preset * 100.0f);
                const bool current = std::abs(EffectiveUiScale(app) - preset) < 0.01f;
                if (ImGui::MenuItem(label.c_str(), nullptr, current))
                {
                    SetUiScale(app, preset);
                }
            }
            ImGui::Separator();
            ImGui::TextDisabled("Ctrl + numpad +/-");
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Reset layout"))
        {
            app.reset_layout = true;
        }
        ImGui::MenuItem("ImGui demo window", nullptr, &app.show_imgui_demo);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("About Umbra"))
        {
            app.open_about = true;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void DrawStatusBar(const AppState &app)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

    if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), flags))
    {
        if (ImGui::BeginMenuBar())
        {
            if (IsRendering(app))
            {
                ImGui::Text(ICON_FA_SPINNER "  Rendering... %d%%", static_cast<int>(RenderProgress(app) * 100.0f));
            }
            else
            {
                ImGui::TextUnformatted(app.status.c_str());
            }

            const std::string right = std::format("{}   |   {} x {}   |   {} FPS", umbra::milestone,
                                                  app.framebuffer.Width(), app.framebuffer.Height(), GetFPS());
            const float right_width = ImGui::CalcTextSize(right.c_str()).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - right_width - 2.0f * ImGui::GetStyle().WindowPadding.x);
            ImGui::TextDisabled("%s", right.c_str());
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

/// The docking area between the menu and the status bar, with a default layout.
void DrawDockspace(AppState &app)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    const ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    // First start (nothing in imgui.ini yet) or View > Reset layout.
    if (app.reset_layout || ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        // The side panel gets about 400 (scaled) pixels, but never less than 20% or more than half.
        const float side_ratio = std::clamp(umbra::ui::Px(400.0f) / viewport->WorkSize.x, 0.2f, 0.5f);
        ImGuiID right_id{0};
        ImGuiID left_id{0};
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, side_ratio, &right_id, &left_id);
        ImGui::DockBuilderDockWindow("###Viewport", left_id);
        ImGui::DockBuilderDockWindow("###Render", right_id);
        ImGui::DockBuilderFinish(dockspace_id);
        app.reset_layout = false;
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport);
}

void DrawViewport(const AppState &app)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    const bool visible =
        ImGui::Begin(viewport_window_title, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const auto image_width = static_cast<float>(app.framebuffer.Width());
    const auto image_height = static_cast<float>(app.framebuffer.Height());

    if (visible && available.x > 1.0f && available.y > 1.0f && image_width > 0.0f)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(origin, ImVec2{origin.x + available.x, origin.y + available.y},
                                 umbra::ui::viewport_background);

        using umbra::ui::Px;

        // Fit the image into the panel (with a margin), keep the aspect ratio, center it.
        const float margin = Px(24.0f);
        const float zoom = std::max(
            0.01f, std::min((available.x - 2.0f * margin) / image_width, (available.y - 2.0f * margin) / image_height));
        const ImVec2 size{std::floor(image_width * zoom), std::floor(image_height * zoom)};
        const ImVec2 position{std::floor(origin.x + (available.x - size.x) * 0.5f),
                              std::floor(origin.y + (available.y - size.y) * 0.5f)};

        // A soft shadow and a thin outline make the image "sit" on the background.
        for (int layer{1}; layer <= 6; ++layer)
        {
            const float spread = Px(3.0f) * static_cast<float>(layer);
            draw_list->AddRectFilled(ImVec2{position.x - spread, position.y - spread + Px(4.0f)},
                                     ImVec2{position.x + size.x + spread, position.y + size.y + spread + Px(4.0f)},
                                     umbra::ui::image_shadow_layer, spread);
        }
        ImGui::SetCursorScreenPos(position);
        rlImGuiImageSizeV(&app.texture, Vector2{size.x, size.y});
        draw_list->AddRect(ImVec2{position.x - 1.0f, position.y - 1.0f},
                           ImVec2{position.x + size.x + 1.0f, position.y + size.y + 1.0f}, umbra::ui::image_outline);

        // A thin progress line along the bottom edge of the image.
        if (IsRendering(app))
        {
            const float bottom = position.y + size.y;
            draw_list->AddRectFilled(ImVec2{position.x, bottom - Px(3.0f)},
                                     ImVec2{position.x + size.x * RenderProgress(app), bottom}, umbra::ui::accent);
        }

        // Small overlay in the top-left corner, like in 3D apps: image size, and how much
        // the image is magnified to fit the panel.
        const std::string hud =
            std::format("{} x {}  ·  zoom {:.0f}%", app.framebuffer.Width(), app.framebuffer.Height(), zoom * 100.0f);
        const ImVec2 text_size = ImGui::CalcTextSize(hud.c_str());
        const ImVec2 hud_min{origin.x + Px(10.0f), origin.y + Px(10.0f)};
        const ImVec2 hud_max{hud_min.x + text_size.x + Px(16.0f), hud_min.y + text_size.y + Px(8.0f)};
        draw_list->AddRectFilled(hud_min, hud_max, umbra::ui::hud_background, Px(4.0f));
        draw_list->AddText(ImVec2{hud_min.x + Px(8.0f), hud_min.y + Px(4.0f)}, umbra::ui::hud_text, hud.c_str());
    }
    ImGui::End();
}

void DrawRenderPanel(AppState &app)
{
    if (!ImGui::Begin(render_window_title))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Output");
    if (BeginProperties("##output"))
    {
        Property("Resolution");
        if (ImGui::BeginCombo("##resolution", resolutions[app.resolution_index].label))
        {
            for (std::size_t i{0}; i < resolutions.size(); ++i)
            {
                const bool selected = (i == app.resolution_index);
                if (ImGui::Selectable(resolutions[i].label, selected) && !selected)
                {
                    app.resolution_index = i;
                    RestartRender(app);
                }
            }
            ImGui::EndCombo();
        }

        Property("Slow motion");
        ImGui::Checkbox("1 row per frame", &app.slow_motion);
        ImGui::SetItemTooltip("Watch the image being drawn row by row.");
        ImGui::EndTable();
    }

    ImGui::Spacing();
    const float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "  Re-render", ImVec2{button_width, 0.0f}))
    {
        RestartRender(app);
    }
    ImGui::SetItemTooltip("F5");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_DISK "  Save PNG", ImVec2{button_width, 0.0f}))
    {
        SaveSnapshot(app);
    }
    ImGui::SetItemTooltip("Ctrl+S  -  saved into ./renders/");

    ImGui::SeparatorText("Progress");
    const float progress = RenderProgress(app);
    const std::string progress_label = IsRendering(app) ? std::format("{:.0f}%", progress * 100.0f) : "Done";
    ImGui::ProgressBar(progress, ImVec2{-FLT_MIN, 0.0f}, progress_label.c_str());

    if (BeginProperties("##progress"))
    {
        const double seconds = app.compute_time.count();
        const double pixels = static_cast<double>(app.next_row) * app.framebuffer.Width();
        PropertyText("Rows", std::format("{} / {}", app.next_row, app.framebuffer.Height()));
        PropertyText("Render time", std::format("{:.3f} s", seconds));
        PropertyText("Speed", seconds > 0.0 ? std::format("{:.1f} Mpixel/s", pixels / seconds / 1e6) : "-");
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Renderer");
    if (BeginProperties("##renderer"))
    {
        PropertyText("Backend", "CPU, 1 thread");
        PropertyText("Milestone", std::string{umbra::milestone});
        ImGui::EndTable();
    }

    ImGui::End();
}

void DrawAbout(AppState &app)
{
    if (app.open_about)
    {
        ImGui::OpenPopup("About Umbra");
        app.open_about = false;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
    if (ImGui::BeginPopupModal("About Umbra", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Umbra %s", UMBRA_VERSION);
        ImGui::TextUnformatted("A ray tracer with a GUI, written in C++23.");
        ImGui::Separator();
        ImGui::TextDisabled("raylib %s   |   Dear ImGui %s", RAYLIB_VERSION, IMGUI_VERSION);
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2{umbra::ui::Px(120.0f), 0.0f}) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/// Saves what is currently on screen (call it after the UI is drawn, before EndDrawing).
void SaveWindowScreenshot(const std::filesystem::path &path)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
    const Image screen = LoadImageFromScreen();
    ExportImage(screen, path.string().c_str());
    UnloadImage(screen);
}

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

int main(int argc, char **argv)
{
    std::optional<std::filesystem::path> screenshot_path;
    if (const auto value = OptionValue(argc, argv, "--screenshot"))
    {
        screenshot_path = std::filesystem::path{*value};
    }
    const auto ui_scale_argument = OptionValue(argc, argv, "--ui-scale");

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1600, 900, "Umbra");
    SetWindowMinSize(960, 540);
    SetExitKey(KEY_NULL); // Esc should not close the app
    SetTargetFPS(60);
    if (!screenshot_path)
    {
        MaximizeWindow();
    }

    umbra::ui::InstallFontLoader();
    rlImGuiSetup(true);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    AppState app{};

    // Settings and panel layout live in the config directory (e.g. ~/.config/umbra).
    // Screenshot mode uses neither, so it always starts from the defaults.
    std::string layout_path;
    if (!screenshot_path)
    {
        const std::filesystem::path config_directory = umbra::app::ConfigDirectory();
        std::error_code error;
        std::filesystem::create_directories(config_directory, error); // ImGui needs it to save the layout
        app.settings_path = config_directory / "settings.ini";
        app.settings = umbra::app::LoadSettings(app.settings_path);
        layout_path = (config_directory / "layout.ini").string();
    }
    io.IniFilename = layout_path.empty() ? nullptr : layout_path.c_str();

    // --ui-scale only applies to this run; it is saved only if you change the scale in the app.
    if (ui_scale_argument)
    {
        if (const auto scale = umbra::app::ParseUiScale(*ui_scale_argument))
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
            umbra::ui::ApplyTheme(scale);
            app.applied_ui_scale = scale;
        }

        AdvanceRender(app);
        UploadTexture(app);

        BeginDrawing();
        ClearBackground(umbra::ui::clear_color);

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
        if (screenshot_path && !IsRendering(app) && ++finished_frames == 10)
        {
            SaveWindowScreenshot(*screenshot_path);
            app.quit = true;
        }

        EndDrawing();
    }

    UnloadTexture(app.texture);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
