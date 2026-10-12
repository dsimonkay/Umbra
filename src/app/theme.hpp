#ifndef UMBRA_APP_THEME_HPP
#define UMBRA_APP_THEME_HPP

#include <imgui.h>
#include <raylib.h>

namespace umbra::ui
{

// A few colors that the drawing code needs directly (the rest lives in the ImGui style).
inline constexpr Color clear_color{23, 23, 25, 255};                    // behind everything
inline constexpr ImU32 viewport_background = IM_COL32(19, 19, 21, 255); // around the rendered image
inline constexpr ImU32 accent = IM_COL32(74, 139, 223, 255);            // progress line, highlights
inline constexpr ImU32 image_shadow_layer = IM_COL32(0, 0, 0, 22);      // drawn 6x, stacked
inline constexpr ImU32 image_outline = IM_COL32(255, 255, 255, 18);
inline constexpr ImU32 hud_background = IM_COL32(0, 0, 0, 150);
inline constexpr ImU32 hud_text = IM_COL32(220, 220, 225, 255);

/// @brief Make rlImGui load our UI font (Roboto + Font Awesome icons) instead of
///        the default one. Call it before rlImGuiSetup().
void InstallFontLoader();

/// @brief The "pro grey" look: colors, roundings, spacing. Call it after rlImGuiSetup().
void ApplyTheme();

} // namespace umbra::ui

#endif // UMBRA_APP_THEME_HPP
