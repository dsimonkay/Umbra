#include "app/panels/panels.h"

#include "app/render_job.h"
#include "ui/theme.h"

#include <extras/IconsFontAwesome6.h> // ICON_FA_*, shipped with rlImGui
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <string>

namespace umbra::app
{

namespace
{

// "visible title###stable id": the part after ### identifies the window,
// so the icon or the text can change without breaking the saved layout.
constexpr const char *viewport_window_title{ICON_FA_IMAGE "  Viewport###Viewport"};

} // namespace

void DrawViewport(const AppState &app)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
    const bool visible =
        ImGui::Begin(viewport_window_title, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const auto image_width = static_cast<float>(app.framebuffer.Width());
    const auto image_height = static_cast<float>(app.framebuffer.Height());

    if (visible && available.x > 1.0f && available.y > 1.0f && image_width > 0.0f)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(origin, ImVec2{origin.x + available.x, origin.y + available.y},
                                 ui::viewport_background);

        using ui::Px;

        // Fit the image into the panel (with a margin), keep the aspect ratio, center it.
        const float margin = Px(24.0f);
        const float zoom = std::max(
            0.01f, std::min((available.x - 2.0f * margin) / image_width, (available.y - 2.0f * margin) / image_height));
        const ImVec2 size{std::floor(image_width * zoom), std::floor(image_height * zoom)};
        const ImVec2 position{std::floor(origin.x + (available.x - size.x) * 0.5f),
                              std::floor(origin.y + (available.y - size.y) * 0.5f)};

        // A soft shadow and a thin outline make the image "sit" on the background.
        for (int layer{1}; layer <= 6; ++layer)
        {
            const float spread = Px(3.0f) * static_cast<float>(layer);
            draw_list->AddRectFilled(ImVec2{position.x - spread, position.y - spread + Px(4.0f)},
                                     ImVec2{position.x + size.x + spread, position.y + size.y + spread + Px(4.0f)},
                                     ui::image_shadow_layer, spread);
        }
        ImGui::SetCursorScreenPos(position);
        rlImGuiImageSizeV(&app.texture, ::Vector2{size.x, size.y});
        draw_list->AddRect(ImVec2{position.x - 1.0f, position.y - 1.0f},
                           ImVec2{position.x + size.x + 1.0f, position.y + size.y + 1.0f}, ui::image_outline);

        // A thin progress line along the bottom edge of the image.
        if (IsRendering(app))
        {
            const float bottom = position.y + size.y;
            draw_list->AddRectFilled(ImVec2{position.x, bottom - Px(3.0f)},
                                     ImVec2{position.x + size.x * RenderProgress(app), bottom}, ui::accent);
        }

        // Small overlay in the top-left corner, like in 3D apps: image size, and how much
        // the image is magnified to fit the panel.
        const std::string hud =
            std::format("{} x {}  ·  zoom {:.0f}%", app.framebuffer.Width(), app.framebuffer.Height(), zoom * 100.0f);
        const ImVec2 text_size = ImGui::CalcTextSize(hud.c_str());
        const ImVec2 hud_min{origin.x + Px(10.0f), origin.y + Px(10.0f)};
        const ImVec2 hud_max{hud_min.x + text_size.x + Px(16.0f), hud_min.y + text_size.y + Px(8.0f)};
        draw_list->AddRectFilled(hud_min, hud_max, ui::hud_background, Px(4.0f));
        draw_list->AddText(ImVec2{hud_min.x + Px(8.0f), hud_min.y + Px(4.0f)}, ui::hud_text, hud.c_str());
    }
    ImGui::End();
}

} // namespace umbra::app
