#include "ui/theme.h"

#include <rlImGui.h>

#define GLFW_INCLUDE_NONE // we only need the window functions, not the OpenGL headers
#include <GLFW/glfw3.h>

#include <filesystem>
#include <string>

namespace umbra::ui
{

namespace
{

constexpr float base_font_size{16.0f};

/// Called by rlImGui during setup, instead of loading ImGui's built-in pixel font.
/// The font is loaded at its base size: scaling happens later through the style's
/// FontScaleDpi / FontScaleMain factors, and ImGui (1.92+) re-rasterizes the glyphs
/// at the final size, so the text stays sharp at any scale.
void LoadFonts()
{
    ImGuiIO &io = ImGui::GetIO();
    const float size = base_font_size;

    // The font is copied next to the executable by CMake (see src/app/CMakeLists.txt).
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
        config.SizePixels = 13.0f;
        io.Fonts->AddFontDefault(&config);
    }
    // rlImGui merges the Font Awesome icons into this font afterwards.
}

constexpr ImVec4 Rgb(const int r, const int g, const int b, const float alpha = 1.0f)
{
    return ImVec4{static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f,
                  alpha};
}

} // namespace

void InstallFontLoader()
{
    rlImGuiSetLoadFontsCallback(LoadFonts);
}

float SystemScale()
{
    // raylib's GetWindowScaleDPI() returns 1.0 unless FLAG_WINDOW_HIGHDPI is set, and that flag
    // also changes how raylib sizes the window and maps the mouse. We only want the number,
    // so we ask GLFW (the window library inside raylib) directly. raylib's window is the
    // current OpenGL context, so glfwGetCurrentContext() gives us its GLFW handle.
    float x_scale{1.0f};
    float y_scale{1.0f};
    if (GLFWwindow *window = glfwGetCurrentContext(); window != nullptr)
    {
        glfwGetWindowContentScale(window, &x_scale, &y_scale);
    }
    return y_scale > 0.0f ? y_scale : 1.0f;
}

float UiScale()
{
    const ImGuiStyle &style = ImGui::GetStyle();
    return style.FontScaleDpi * style.FontScaleMain;
}

void ApplyTheme(const float scale)
{
    ImGuiStyle &style = ImGui::GetStyle();

    // Start from ImGui's defaults every time, so applying the theme again (after a
    // UI scale change) does not scale the already scaled sizes a second time.
    const float font_size_base = style.FontSizeBase;
    style = ImGuiStyle{};
    style.FontSizeBase = font_size_base;

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
    const ImVec4 bg_deep = Rgb(19, 19, 21);
    const ImVec4 bg_window = Rgb(33, 33, 36);
    const ImVec4 bg_title = Rgb(26, 26, 28);
    const ImVec4 bg_frame = Rgb(45, 45, 49);
    const ImVec4 bg_hover = Rgb(56, 56, 61);
    const ImVec4 bg_active = Rgb(68, 68, 74);
    const ImVec4 border = Rgb(48, 48, 52);
    const ImVec4 text = Rgb(221, 221, 225);
    const ImVec4 text_dim = Rgb(128, 128, 136);
    const ImVec4 accent_color = Rgb(74, 139, 223);
    const ImVec4 accent_hover = Rgb(98, 157, 232);
    const ImVec4 accent_dim = Rgb(74, 139, 223, 0.35f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = text_dim;
    colors[ImGuiCol_WindowBg] = bg_window;
    colors[ImGuiCol_ChildBg] = Rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_PopupBg] = Rgb(38, 38, 42, 0.98f);
    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = Rgb(0, 0, 0, 0.0f);

    colors[ImGuiCol_FrameBg] = bg_frame;
    colors[ImGuiCol_FrameBgHovered] = bg_hover;
    colors[ImGuiCol_FrameBgActive] = bg_active;

    colors[ImGuiCol_TitleBg] = bg_title;
    colors[ImGuiCol_TitleBgActive] = bg_title;
    colors[ImGuiCol_TitleBgCollapsed] = bg_title;
    colors[ImGuiCol_MenuBarBg] = bg_title;

    colors[ImGuiCol_ScrollbarBg] = Rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_ScrollbarGrab] = bg_hover;
    colors[ImGuiCol_ScrollbarGrabHovered] = bg_active;
    colors[ImGuiCol_ScrollbarGrabActive] = Rgb(84, 84, 92);

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

    colors[ImGuiCol_ResizeGrip] = Rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_ResizeGripHovered] = accent_dim;
    colors[ImGuiCol_ResizeGripActive] = accent_color;

    colors[ImGuiCol_Tab] = bg_title;
    colors[ImGuiCol_TabHovered] = bg_hover;
    colors[ImGuiCol_TabSelected] = bg_window;
    colors[ImGuiCol_TabSelectedOverline] = accent_color;
    colors[ImGuiCol_TabDimmed] = bg_title;
    colors[ImGuiCol_TabDimmedSelected] = bg_window;
    colors[ImGuiCol_TabDimmedSelectedOverline] = Rgb(0, 0, 0, 0.0f);

    colors[ImGuiCol_DockingPreview] = accent_dim;
    colors[ImGuiCol_DockingEmptyBg] = bg_deep;

    colors[ImGuiCol_PlotHistogram] = accent_color; // also the progress bar
    colors[ImGuiCol_PlotHistogramHovered] = accent_hover;

    colors[ImGuiCol_TableHeaderBg] = bg_title;
    colors[ImGuiCol_TableBorderStrong] = border;
    colors[ImGuiCol_TableBorderLight] = Rgb(42, 42, 46);
    colors[ImGuiCol_TableRowBg] = Rgb(0, 0, 0, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = Rgb(255, 255, 255, 0.02f);

    colors[ImGuiCol_TextSelectedBg] = accent_dim;
    colors[ImGuiCol_NavCursor] = accent_color;
    colors[ImGuiCol_ModalWindowDimBg] = Rgb(0, 0, 0, 0.55f);

    // ---- scale ----------------------------------------------------------------
    // Sizes are scaled once here; the font is scaled by ImGui itself, through FontScaleDpi.
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    style.FontScaleMain = 1.0f;
}

} // namespace umbra::ui
