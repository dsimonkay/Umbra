#ifndef UMBRA_APP_RENDER_JOB_H
#define UMBRA_APP_RENDER_JOB_H

// The render job: the image is rendered on the UI thread, a few rows per frame within a small
// time budget, so the window stays responsive and you can watch the image appear.
// M3 replaces the rows with progressive accumulation; worker threads arrive in M8.

#include "app/app_state.h"

#include <array>

namespace umbra::app
{

/// @brief An output size offered in the Render panel.
struct Resolution
{
    const char *label{""};
    int width{};
    int height{};
};

inline constexpr std::array kResolutions{
    Resolution{"640 x 360", 640, 360},
    Resolution{"960 x 540", 960, 540},
    Resolution{"1280 x 720 (HD)", 1280, 720},
    Resolution{"1920 x 1080 (Full HD)", 1920, 1080},
};

/// @brief True while there are rows left to render.
[[nodiscard]] bool IsRendering(const AppState &app);

/// @brief How much of the image is done, from 0 to 1.
[[nodiscard]] float RenderProgress(const AppState &app);

/// @brief Start rendering from the first row; resize the image and the texture if needed.
void RestartRender(AppState &app);

/// @brief Render rows until this frame's time budget is used up, so the UI stays smooth.
void AdvanceRender(AppState &app);

/// @brief Copy the framebuffer into the texture, if it changed since the last upload.
void UploadTexture(AppState &app);

/// @brief Save the image as ./renders/umbra-<date>-<time>.png, and say so in the status bar.
void SaveSnapshot(AppState &app);

} // namespace umbra::app

#endif // UMBRA_APP_RENDER_JOB_H
