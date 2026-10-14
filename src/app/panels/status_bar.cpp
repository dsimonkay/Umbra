#include "app/panels/panels.h"

#include "app/render_job.h"
#include "core/render.h"

#include <extras/IconsFontAwesome6.h> // ICON_FA_*, shipped with rlImGui
#include <imgui.h>
#include <imgui_internal.h> // BeginViewportSideBar
#include <raylib.h>         // GetFPS

#include <format>
#include <string>

namespace umbra::app
{

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

            const std::string right = std::format("{}   |   {} x {}   |   {} FPS", milestone, app.framebuffer.Width(),
                                                  app.framebuffer.Height(), GetFPS());
            const float right_width = ImGui::CalcTextSize(right.c_str()).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - right_width - 2.0f * ImGui::GetStyle().WindowPadding.x);
            ImGui::TextDisabled("%s", right.c_str());
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

} // namespace umbra::app
