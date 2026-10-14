#ifndef UMBRA_APP_UI_SCALE_H
#define UMBRA_APP_UI_SCALE_H

// The UI scale: by default the system's (see ui::SystemScale()), or a fixed value chosen
// under View > UI scale. Useful under WSLg, where Windows' display scaling does not
// reach Linux apps. The theme (ui/theme.h) is rebuilt whenever it changes.

#include "app/app_state.h"

#include <array>
#include <optional>

namespace umbra::app
{

/// @brief The fixed scales offered under View > UI scale.
inline constexpr std::array ui_scale_presets{1.0f, 1.25f, 1.5f, 1.75f, 2.0f};

/// @brief How much Ctrl + numpad +/- changes the scale.
inline constexpr float ui_scale_step{0.25f};

/// @brief The UI scale in effect: the fixed one from the settings, or else the system's.
[[nodiscard]] float EffectiveUiScale(const AppState &app);

/// @brief Change the UI scale setting (empty: automatic) and save it right away.
///        The theme picks up the new scale at the start of the next frame.
void SetUiScale(AppState &app, std::optional<float> scale);

} // namespace umbra::app

#endif // UMBRA_APP_UI_SCALE_H
