#include "app/panels/panels.h"

#include "ui/theme.h"

#include <imgui.h>
#include <raylib.h> // RAYLIB_VERSION

// Set from the project version in src/app/CMakeLists.txt.
#ifndef UMBRA_VERSION
#define UMBRA_VERSION "dev"
#endif

namespace umbra::app
{

void DrawAbout(AppState &app)
{
    if (app.open_about)
    {
        ImGui::OpenPopup("About Umbra");
        app.open_about = false;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
    if (ImGui::BeginPopupModal("About Umbra", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Umbra %s", UMBRA_VERSION);
        ImGui::TextUnformatted("A ray tracer with a GUI, written in C++23.");
        ImGui::Separator();
        ImGui::TextDisabled("raylib %s   |   Dear ImGui %s", RAYLIB_VERSION, IMGUI_VERSION);
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2{ui::Px(120.0f), 0.0f}) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace umbra::app
