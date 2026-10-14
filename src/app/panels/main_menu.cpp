// The main menu and the global keyboard shortcuts. They trigger the same actions, so they live
// together: a new shortcut goes into HandleShortcuts() and next to its menu item.

#include "app/panels/panels.h"

#include "app/render_job.h"
#include "app/ui_scale.h"
#include "ui/theme.h"

#include <imgui.h>

#include <cmath>
#include <format>
#include <optional>
#include <string>

namespace umbra::app
{

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
            const std::string automatic = std::format("Automatic ({:.0f}%)", ui::SystemScale() * 100.0f);
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

} // namespace umbra::app
