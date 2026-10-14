#include "app/panels/panels.h"

#include "ui/theme.h"

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder*

#include <algorithm>

namespace umbra::app
{

void DrawDockspace(AppState &app)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    const ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    // First start (nothing in layout.ini yet) or View > Reset layout.
    if (app.reset_layout || ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        // The side panel gets about 400 (scaled) pixels, but never less than 20% or more than half.
        const float side_ratio = std::clamp(ui::Px(400.0f) / viewport->WorkSize.x, 0.2f, 0.5f);
        ImGuiID right_id{0};
        ImGuiID left_id{0};
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, side_ratio, &right_id, &left_id);
        // The stable IDs of the panels: the part after "###" in their titles.
        ImGui::DockBuilderDockWindow("###Viewport", left_id);
        ImGui::DockBuilderDockWindow("###Render", right_id);
        ImGui::DockBuilderFinish(dockspace_id);
        app.reset_layout = false;
    }

    ImGui::DockSpaceOverViewport(dockspace_id, viewport);
}

} // namespace umbra::app
