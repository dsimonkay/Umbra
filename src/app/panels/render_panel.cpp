#include "app/panels/panels.h"

#include "app/render_job.h"
#include "core/render.h"
#include "ui/widgets.h"

#include <extras/IconsFontAwesome6.h> // ICON_FA_*, shipped with rlImGui
#include <imgui.h>

#include <cfloat>
#include <cstddef>
#include <format>
#include <string>

namespace umbra::app
{

namespace
{

// "visible title###stable id": the part after ### identifies the window,
// so the icon or the text can change without breaking the saved layout.
constexpr const char *render_window_title{ICON_FA_SLIDERS "  Render###Render"};

} // namespace

void DrawRenderPanel(AppState &app)
{
    if (!ImGui::Begin(render_window_title))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Output");
    if (ui::BeginProperties("##output"))
    {
        ui::Property("Resolution");
        if (ImGui::BeginCombo("##resolution", resolutions[app.resolution_index].label))
        {
            for (std::size_t i{0}; i < resolutions.size(); ++i)
            {
                const bool selected = (i == app.resolution_index);
                if (ImGui::Selectable(resolutions[i].label, selected) && !selected)
                {
                    app.resolution_index = i;
                    RestartRender(app);
                }
            }
            ImGui::EndCombo();
        }

        ui::Property("Slow motion");
        ImGui::Checkbox("1 row per frame", &app.slow_motion);
        ImGui::SetItemTooltip("Watch the image being drawn row by row.");
        ImGui::EndTable();
    }

    ImGui::Spacing();
    const float button_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "  Re-render", ImVec2{button_width, 0.0f}))
    {
        RestartRender(app);
    }
    ImGui::SetItemTooltip("F5");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_DISK "  Save PNG", ImVec2{button_width, 0.0f}))
    {
        SaveSnapshot(app);
    }
    ImGui::SetItemTooltip("Ctrl+S  -  saved into ./renders/");

    ImGui::SeparatorText("Progress");
    const float progress = RenderProgress(app);
    const std::string progress_label = IsRendering(app) ? std::format("{:.0f}%", progress * 100.0f) : "Done";
    ImGui::ProgressBar(progress, ImVec2{-FLT_MIN, 0.0f}, progress_label.c_str());

    if (ui::BeginProperties("##progress"))
    {
        const double seconds = app.compute_time.count();
        const double pixels = static_cast<double>(app.next_row) * app.framebuffer.Width();
        ui::PropertyText("Rows", std::format("{} / {}", app.next_row, app.framebuffer.Height()));
        ui::PropertyText("Render time", std::format("{:.3f} s", seconds));
        ui::PropertyText("Speed", seconds > 0.0 ? std::format("{:.1f} Mpixel/s", pixels / seconds / 1e6) : "-");
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Renderer");
    if (ui::BeginProperties("##renderer"))
    {
        ui::PropertyText("Backend", "CPU, 1 thread");
        ui::PropertyText("Milestone", std::string{milestone});
        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace umbra::app
