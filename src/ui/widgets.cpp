#include "ui/widgets.h"

#include "ui/theme.h"

#include <imgui.h>

#include <cfloat>

namespace umbra::ui
{

bool BeginProperties(const char *id)
{
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp))
    {
        return false;
    }
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, Px(110.0f));
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

void Property(const char *label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void PropertyText(const char *label, const std::string &value)
{
    Property(label);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(value.c_str());
}

} // namespace umbra::ui
