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
//   umbra --screenshot a.png   render, save a picture of the whole window, quit
//                              (for the docs, and so Claude Code can "see" the UI)

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

    bool show_imgui_demo{false};
    bool open_about{false};
    bool reset_layout{false};
    bool quit{false};
};

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

[[nodiscard]] bool is_rendering(const AppState &app)
{
    return app.next_row < app.framebuffer.height();
}

[[nodiscard]] float render_progress(const AppState &app)
{
    const int height = app.framebuffer.height();
    return height > 0 ? static_cast<float>(app.next_row) / static_cast<float>(height) : 0.0f;
}

void set_status(AppState &app, std::string message)
{
    app.status = std::move(message);
    app.status_time = Clock::now();
}

/// Start rendering from the first row; resize the image and the texture if needed.
void restart_render(AppState &app)
{
    const Resolution &resolution = resolutions[app.resolution_index];
    if (resolution.width != app.framebuffer.width() || resolution.height != app.framebuffer.height())
    {
        app.framebuffer.resize(resolution.width, resolution.height);

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
    set_status(app, "Rendering...");
}

/// Render rows until this frame's time budget is used up, so the UI stays smooth.
void advance_render(AppState &app)
{
    if (!is_rendering(app))
    {
        return;
    }

    constexpr std::chrono::milliseconds frame_budget{12};
    const int width = app.framebuffer.width();
    const int height = app.framebuffer.height();
    const auto frame_start = Clock::now();

    while (is_rendering(app))
    {
        const int y = app.next_row;
        for (int x{0}; x < width; ++x)
        {
            app.framebuffer.put_pixel(x, y, umbra::render_pixel(x, y, width, height));
        }
        ++app.next_row;

        if (app.slow_motion || Clock::now() - frame_start > frame_budget)
        {
            break;
        }
    }

    app.texture_dirty = true;
    app.compute_time += Clock::now() - frame_start;

    if (!is_rendering(app))
    {
        set_status(app, std::format("Rendered {} x {} in {:.3f} s", width, height, app.compute_time.count()));
    }
}

void upload_texture(AppState &app)
{
    if (app.texture_dirty)
    {
        UpdateTexture(app.texture, app.framebuffer.data());
        app.texture_dirty = false;
    }
}

void save_snapshot(AppState &app)
{
    const std::filesystem::path path = std::filesystem::path{"renders"} / umbra::timestamped_file_name();
    if (const auto written = umbra::write_png(app.framebuffer, path); written)
    {
        set_status(app, std::format("Saved {}", path.string()));
    }
    else
    {
        set_status(app, std::format("Save failed: {}", written.error()));
    }
}

// ---------------------------------------------------------------------------
// Small UI helpers: a two-column "label | widget" property grid
// ---------------------------------------------------------------------------

bool begin_properties(const char *id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp))
    {
        return false;
    }
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 110.0f * ImGui::GetStyle().FontScaleDpi);
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

/// Starts a row: writes the label, then leaves the cursor in the value column.
void property(const char *label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void property_text(const char *label, const std::string &value)
{
    property(label);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(value.c_str());
}

// ---------------------------------------------------------------------------
// UI pieces
// ---------------------------------------------------------------------------

void handle_shortcuts(AppState &app)
{
    if (ImGui::Shortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal))
    {
        restart_render(app);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
    {
        save_snapshot(app);
    }
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Q, ImGuiInputFlags_RouteGlobal))
    {
        app.quit = true;
    }
}

void draw_main_menu(AppState &app)
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save PNG", "Ctrl+S"))
        {
            save_snapshot(app);
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
            restart_render(app);
        }
        ImGui::MenuItem("Slow motion", nullptr, &app.slow_motion);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
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

void draw_status_bar(const AppState &app)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

    if (ImGui::BeginViewportSideBar("##StatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), flags))
    {
        if (ImGui::BeginMenuBar())
        {
            if (is_rendering(app))
            {
                ImGui::Text(ICON_FA_SPINNER "  Rendering... %d%%", static_cast<int>(render_progress(app) * 100.0f));
            }
            else
            {
                ImGui::TextUnformatted(app.status.c_str());
            }

            const std::string right = std::format("{}   |   {} x {}   |   {} FPS", umbra::milestone,
                                                  app.framebuffer.width(), app.framebuffer.height(), GetFPS());
            const float right_width = ImGui::CalcTextSize(right.c_str()).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - right_width - 2.0f * ImGui::GetStyle().WindowPadding.x);
            ImGui::TextDisabled("%s", right.c_str());
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

/// The docking area between the menu and the status bar, with a default layout.
void draw_dockspace(AppState &app)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    const ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    // First start (nothing in imgui.ini yet) or View > Reset layout.
    if (app.reset_layout || ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        ImGuiID right_id{0};
        ImGuiID left_id{0};
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.24f, &right_id, &left_id);
        ImGui::DockBuilderDockWindow("###Viewport", left_id);
        ImGui::DockBuilderDockWindow("###Render", right_id);
        ImGui::DockBuilderFinish(dockspace_id);
        app.reset_layout = false;
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport);
}

void draw_viewport(const AppState &app)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    const bool visible =
        ImGui::Begin(viewport_window_title, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const auto image_width = static_cast<float>(app.framebuffer.width());
    const auto image_height = static_cast<float>(app.framebuffer.height());

    if (visible && available.x > 1.0f && available.y > 1.0f && image_width > 0.0f)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(origin, ImVec2{origin.x + available.x, origin.y + available.y},
                                 umbra::ui::viewport_background);

        // Fit the image into the panel (with a margin), keep the aspect ratio, center it.
        constexpr float margin{24.0f};
        const float zoom = std::max(
            0.01f, std::min((available.x - 2.0f * margin) / image_width, (available.y - 2.0f * margin) / image_height));
        const ImVec2 size{std::floor(image_width * zoom), std::floor(image_height * zoom)};
        const ImVec2 position{std::floor(origin.x + (available.x - size.x) * 0.5f),
                              std::floor(origin.y + (available.y - size.y) * 0.5f)};

        // A soft shadow and a thin outline make the image "sit" on the background.
        for (int layer{1}; layer <= 6; ++layer)
        {
            const float spread = 3.0f * static_cast<float>(layer);
            draw_list->AddRectFilled(ImVec2{position.x - spread, position.y - spread + 4.0f},
                                     ImVec2{position.x + size.x + spread, position.y + size.y + spread + 4.0f},
                                     umbra::ui::image_shadow_layer, spread);
        }
        ImGui::SetCursorScreenPos(position);
        rlImGuiImageSizeV(&app.texture, Vector2{size.x, size.y});
        draw_list->AddRect(ImVec2{position.x - 1.0f, position.y - 1.0f},
                           ImVec2{position.x + size.x + 1.0f, position.y + size.y + 1.0f}, umbra::ui::image_outline);

        // A thin progress line along the bottom edge of the image.
        if (is_rendering(app))
        {
            const float bottom = position.y + size.y;
            draw_list->AddRectFilled(ImVec2{position.x, bottom - 3.0f},
                                     ImVec2{position.x + size.x * render_progress(app), bottom}, umbra::ui::accent);
        }

        // Small overlay in the top-left corner, like in 3D apps.
        const std::string hud =
            std::format("{} x {}  ·  {:.0f}%", app.framebuffer.width(), app.framebuffer.height(), zoom * 100.0f);
        const ImVec2 text_size = ImGui::CalcTextSize(hud.c_str());
        const ImVec2 hud_min{origin.x + 10.0f, origin.y + 10.0f};
        const ImVec2 hud_max{hud_min.x + text_size.x + 16.0f, hud_min.y + text_size.y + 8.0f};
        draw_list->AddRectFilled(hud_min, hud_max, umbra::ui::hud_background, 4.0f);
        draw_list->AddText(ImVec2{hud_min.x + 8.0f, hud_min.y + 4.0f}, umbra::ui::hud_text, hud.c_str());
    }
    ImGui::End();
}

void draw_render_panel(AppState &app)
{
    if (!ImGui::Begin(render_window_title))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Output");
    if (begin_properties("##output"))
    {
        property("Resolution");
        if (ImGui::BeginCombo("##resolution", resolutions[app.resolution_index].label))
        {
            for (std::size_t i{0}; i < resolutions.size(); ++i)
            {
                const bool selected = (i == app.resolution_index);
                if (ImGui::Selectable(resolutions[i].label, selected) && !selected)
                {
                    app.resolution_index = i;
                    restart_render(app);
                }
            }
            ImGui::EndCombo();
        }

        property("Slow motion");
        ImGui::Checkbox("1 row per frame", &app.slow_motion);
        ImGui::SetItemTooltip("Watch the image being drawn row by row.");
        ImGui::EndTable();
    }

    ImGui::Spacing();
    const float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "  Re-render", ImVec2{button_width, 0.0f}))
    {
        restart_render(app);
    }
    ImGui::SetItemTooltip("F5");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_DISK "  Save PNG", ImVec2{button_width, 0.0f}))
    {
        save_snapshot(app);
    }
    ImGui::SetItemTooltip("Ctrl+S  -  saved into ./renders/");

    ImGui::SeparatorText("Progress");
    const float progress = render_progress(app);
    const std::string progress_label = is_rendering(app) ? std::format("{:.0f}%", progress * 100.0f) : "Done";
    ImGui::ProgressBar(progress, ImVec2{-FLT_MIN, 0.0f}, progress_label.c_str());

    if (begin_properties("##progress"))
    {
        const double seconds = app.compute_time.count();
        const double pixels = static_cast<double>(app.next_row) * app.framebuffer.width();
        property_text("Rows", std::format("{} / {}", app.next_row, app.framebuffer.height()));
        property_text("Render time", std::format("{:.3f} s", seconds));
        property_text("Speed", seconds > 0.0 ? std::format("{:.1f} Mpixel/s", pixels / seconds / 1e6) : "-");
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Renderer");
    if (begin_properties("##renderer"))
    {
        property_text("Backend", "CPU, 1 thread");
        property_text("Milestone", std::string{umbra::milestone});
        ImGui::EndTable();
    }

    ImGui::End();
}

void draw_about(AppState &app)
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
        if (ImGui::Button("Close", ImVec2{120.0f, 0.0f}) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/// Saves what is currently on screen (call it after the UI is drawn, before EndDrawing).
void save_window_screenshot(const std::filesystem::path &path)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
    const Image screen = LoadImageFromScreen();
    ExportImage(screen, path.string().c_str());
    UnloadImage(screen);
}

std::optional<std::filesystem::path> screenshot_argument(const int argc, char **argv)
{
    for (int i{1}; i + 1 < argc; ++i)
    {
        if (std::string_view{argv[i]} == "--screenshot")
        {
            return std::filesystem::path{argv[i + 1]};
        }
    }
    return std::nullopt;
}

} // namespace

int main(int argc, char **argv)
{
    const auto screenshot_path = screenshot_argument(argc, argv);

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

    umbra::ui::install_font_loader();
    rlImGuiSetup(true);
    umbra::ui::apply_theme();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Remember the panel layout in imgui.ini next to the executable (not in the
    // current directory). Screenshot mode always starts from the default layout.
    const std::string ini_path = (std::filesystem::path{GetApplicationDirectory()} / "imgui.ini").string();
    io.IniFilename = screenshot_path ? nullptr : ini_path.c_str();

    AppState app{};
    restart_render(app);

    int finished_frames{0};
    while (!WindowShouldClose() && !app.quit)
    {
        advance_render(app);
        upload_texture(app);

        BeginDrawing();
        ClearBackground(umbra::ui::clear_color);

        rlImGuiBegin();
        handle_shortcuts(app);
        draw_main_menu(app);
        draw_status_bar(app);
        draw_dockspace(app);
        draw_viewport(app);
        draw_render_panel(app);
        draw_about(app);
        if (app.show_imgui_demo)
        {
            ImGui::ShowDemoWindow(&app.show_imgui_demo);
        }
        rlImGuiEnd();

        // Give the layout a few frames to settle after the render has finished.
        if (screenshot_path && !is_rendering(app) && ++finished_frames == 10)
        {
            save_window_screenshot(*screenshot_path);
            app.quit = true;
        }

        EndDrawing();
    }

    UnloadTexture(app.texture);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
