#ifndef UMBRA_UI_THEME_H
#define UMBRA_UI_THEME_H

#include <imgui.h>
#include <raylib.h>

namespace umbra::ui
{

// A few colors that the drawing code needs directly (the rest lives in the ImGui style).
// "::Color" is raylib's 8-bit color; a plain "Color" could pick up umbra::Color (core/color.h) in here.
inline constexpr ::Color kClearColor{23, 23, 25, 255};                  // behind everything
inline constexpr ImU32 kViewportBackground = IM_COL32(19, 19, 21, 255); // around the rendered image
inline constexpr ImU32 kAccent = IM_COL32(74, 139, 223, 255);           // progress line, highlights
inline constexpr ImU32 kImageShadowLayer = IM_COL32(0, 0, 0, 22);       // drawn 6x, stacked
inline constexpr ImU32 kImageOutline = IM_COL32(255, 255, 255, 18);
inline constexpr ImU32 kHudBackground = IM_COL32(0, 0, 0, 150);
inline constexpr ImU32 kHudText = IM_COL32(220, 220, 225, 255);

/// @brief Make rlImGui load our UI font (Roboto + Font Awesome icons) instead of
///        the default one. Call it before rlImGuiSetup().
void InstallFontLoader();

/// @brief The "pro grey" look: colors, roundings, spacing, font size.
/// @param scale the UI scale to build it for (1.0 = 100 %, 1.5 = 150 %).
/// @note Call it after rlImGuiSetup(), and again (between frames) whenever the UI scale changes.
void ApplyTheme(float scale);

/// @brief The UI scale the system asks for (1.0 on a "normal" monitor).
/// @note On Windows this is the "Make everything bigger" setting of the window's monitor.
///       On Linux/X11 (and so under WSLg) it comes from the Xft.dpi X resource: 96 dpi = 1.0.
///       Desktop environments set Xft.dpi from their own scaling setting; WSLg does not, so
///       there it is 1.0 unless you set Xft.dpi yourself (see the README).
[[nodiscard]] float SystemScale();

/// @brief The UI scale the theme was last built for (see ApplyTheme()).
[[nodiscard]] float UiScale();

/// @brief Scale a hard-coded pixel size (margins, offsets, line widths) by the UI scale.
[[nodiscard]] inline float Px(const float pixels)
{
    return pixels * UiScale();
}

} // namespace umbra::ui

#endif // UMBRA_UI_THEME_H
