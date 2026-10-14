#ifndef UMBRA_UI_WIDGETS_H
#define UMBRA_UI_WIDGETS_H

#include <string>

namespace umbra::ui
{

// A two-column "label | widget" property grid, like the property panels of 3D apps:
//
//     if (ui::BeginProperties("##output"))
//     {
//         ui::Property("Resolution");
//         ImGui::Combo(...);                    // fills the value column
//         ui::PropertyText("Rows", "540 / 540");
//         ImGui::EndTable();
//     }

/// @brief Start a property grid. Like ImGui::BeginTable(): call ImGui::EndTable() only if it returned true.
[[nodiscard]] bool BeginProperties(const char *id);

/// @brief Start a row: write the label, then leave the cursor in the value column.
void Property(const char *label);

/// @brief A row with a read-only text value.
void PropertyText(const char *label, const std::string &value);

} // namespace umbra::ui

#endif // UMBRA_UI_WIDGETS_H
