#include "app/theme.hpp"

#include <rlImGui.h>

#include <filesystem>
#include <string>

namespace umbra::ui
{

namespace
{

constexpr float base_font_size{16.0f};

/// UI scale for high-DPI screens (1.0 on a "normal" monitor).
float ui_scale()
{
    const Vector2 dpi = GetWindowScaleDPI();
    return dpi.y > 0.0f ? dpi.y : 1.0f;
}

/// Called by rlImGui during setup, instead of loading ImGui's built-in pixel font.
void load_fonts()
{
    ImGuiIO &io = ImGui::GetIO();
    const float size = base_font_size * ui_scale();

    // The font is copied next to the executable by CMake (see CMakeLists.txt).
    const std::filesystem::path font_path =
        std::filesystem::path{GetApplicationDirectory()} / "assets" / "Roboto-Medium.ttf";

    if (std::filesystem::exists(font_path))
    {
        io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), size);
    }
    else
    {
        TraceLog(LOG_WARNING, "UI font not found at '%s', using the built-in one", font_path.string().c_str());
        ImFontConfig config{};
        config.SizePixels = 13.0f * ui_scale();
        io.Fonts->AddFontDefault(&config);
    }
    // rlImGui merges the Font Awesome icons into this font afterwards.
}

constexpr ImVec4 rgb(const int r, const int g, const int b, const float alpha = 1.0f)
{
    return ImVec4{static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f,
                  alpha};
}

} // namespace

void install_font_loader()
{
    rlImGuiSetLoadFontsCallback(load_fonts);
}

void apply_theme()
{
    ImGuiStyle &style = ImGui::GetStyle();

    // ---- shapes and spacing -------------------------------------------------
    style.WindowPadding = ImVec2{10.0f, 10.0f};
    style.FramePadding = ImVec2{8.0f, 5.0f};
    style.ItemSpacing = ImVec2{8.0f, 6.0f};
    style.ItemInnerSpacing = ImVec2{6.0f, 4.0f};
    style.CellPadding = ImVec2{6.0f, 4.0f};
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabBarBorderSize = 1.0f;
    style.DockingSeparatorSize = 2.0f;
    style.SeparatorTextBorderSize = 1.0f;
    style.SeparatorTextPadding = ImVec2{0.0f, 6.0f};

    style.WindowMenuButtonPosition = ImGuiDir_None; // no collapse arrow on panels
    style.WindowTitleAlign = ImVec2{0.0f, 0.5f};

    // ---- colors: neutral greys + one blue accent ----------------------------
    const ImVec4 bg_deep = rgb(19, 19, 21);
    const ImVec4 bg_window = rgb(33, 33, 36);
    const ImVec4 bg_title = rgb(26, 26, 28);
    const ImVec4 bg_frame = rgb(45, 45, 49);
    const ImVec4 bg_hover = rgb(56, 56, 61);
    const ImVec4 bg_active = rgb(68, 68, 74);
    const ImVec4 border = rgb(48, 48, 52);
    const ImVec4 text = rgb(221, 221, 225);
    const ImVec4 text_dim = rgb(128, 128, 136);
    const ImVec4 accent_color = rgb(74, 139, 223);
    const ImVec4 accent_hover = rgb(98, 157, 232);
    const ImVec4 accent_dim = rgb(74, 139, 223, 0.35f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = text_dim;
    colors[ImGuiCol_WindowBg] = bg_window;
    colors[ImGuiCol_ChildBg] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_PopupBg] = rgb(38, 38, 42, 0.98f);
    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = rgb(0, 0, 0, 0.0f);

    colors[ImGuiCol_FrameBg] = bg_frame;
    colors[ImGuiCol_FrameBgHovered] = bg_hover;
    colors[ImGuiCol_FrameBgActive] = bg_active;

    colors[ImGuiCol_TitleBg] = bg_title;
    colors[ImGuiCol_TitleBgActive] = bg_title;
    colors[ImGuiCol_TitleBgCollapsed] = bg_title;
    colors[ImGuiCol_MenuBarBg] = bg_title;

    colors[ImGuiCol_ScrollbarBg] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_ScrollbarGrab] = bg_hover;
    colors[ImGuiCol_ScrollbarGrabHovered] = bg_active;
    colors[ImGuiCol_ScrollbarGrabActive] = rgb(84, 84, 92);

    colors[ImGuiCol_CheckMark] = accent_color;
    colors[ImGuiCol_SliderGrab] = accent_color;
    colors[ImGuiCol_SliderGrabActive] = accent_hover;

    colors[ImGuiCol_Button] = bg_frame;
    colors[ImGuiCol_ButtonHovered] = bg_hover;
    colors[ImGuiCol_ButtonActive] = bg_active;

    colors[ImGuiCol_Header] = bg_frame;
    colors[ImGuiCol_HeaderHovered] = bg_hover;
    colors[ImGuiCol_HeaderActive] = bg_active;

    colors[ImGuiCol_Separator] = border;
    colors[ImGuiCol_SeparatorHovered] = accent_dim;
    colors[ImGuiCol_SeparatorActive] = accent_color;

    colors[ImGuiCol_ResizeGrip] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_ResizeGripHovered] = accent_dim;
    colors[ImGuiCol_ResizeGripActive] = accent_color;

    colors[ImGuiCol_Tab] = bg_title;
    colors[ImGuiCol_TabHovered] = bg_hover;
    colors[ImGuiCol_TabSelected] = bg_window;
    colors[ImGuiCol_TabSelectedOverline] = accent_color;
    colors[ImGuiCol_TabDimmed] = bg_title;
    colors[ImGuiCol_TabDimmedSelected] = bg_window;
    colors[ImGuiCol_TabDimmedSelectedOverline] = rgb(0, 0, 0, 0.0f);

    colors[ImGuiCol_DockingPreview] = accent_dim;
    colors[ImGuiCol_DockingEmptyBg] = bg_deep;

    colors[ImGuiCol_PlotHistogram] = accent_color; // also the progress bar
    colors[ImGuiCol_PlotHistogramHovered] = accent_hover;

    colors[ImGuiCol_TableHeaderBg] = bg_title;
    colors[ImGuiCol_TableBorderStrong] = border;
    colors[ImGuiCol_TableBorderLight] = rgb(42, 42, 46);
    colors[ImGuiCol_TableRowBg] = rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = rgb(255, 255, 255, 0.02f);

    colors[ImGuiCol_TextSelectedBg] = accent_dim;
    colors[ImGuiCol_NavCursor] = accent_color;
    colors[ImGuiCol_ModalWindowDimBg] = rgb(0, 0, 0, 0.55f);

    const float scale = ui_scale();
    if (scale != 1.0f)
    {
        style.ScaleAllSizes(scale);
    }
}

} // namespace umbra::ui
