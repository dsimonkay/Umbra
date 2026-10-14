#ifndef UMBRA_APP_PANELS_PANELS_H
#define UMBRA_APP_PANELS_PANELS_H

// The windows of the GUI, one .cpp file each (the shortcuts live with the menu).
// Call them once per frame, between rlImGuiBegin() and rlImGuiEnd(), in the order of this
// file: the menu and the status bar take their space first, the dock space fills the rest,
// and the panels dock into it.

#include "app/app_state.h"

namespace umbra::app
{

/// @brief The global keyboard shortcuts: F5, Ctrl+S, Ctrl+Q and the UI scale keys.
void HandleShortcuts(AppState &app);

/// @brief File / Render / View / Help.
void DrawMainMenu(AppState &app);

/// @brief The bar at the bottom: the status (or the progress) on the left; milestone, image size and FPS
///        on the right.
void DrawStatusBar(const AppState &app);

/// @brief The docking area between the menu and the status bar. Builds the default layout on the
///        first start and after View > Reset layout.
void DrawDockspace(AppState &app);

/// @brief "Viewport": the rendered image, fitted into the panel.
void DrawViewport(const AppState &app);

/// @brief "Render": output settings, progress, renderer info.
void DrawRenderPanel(AppState &app);

/// @brief Help > About Umbra.
void DrawAbout(AppState &app);

} // namespace umbra::app

#endif // UMBRA_APP_PANELS_PANELS_H
