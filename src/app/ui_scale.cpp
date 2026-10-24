#include "app/ui_scale.h"

#include "app/settings.h"
#include "ui/theme.h"

#include <algorithm>
#include <format>
#include <string>

namespace umbra::app
{

namespace
{

constexpr float kMinUiScale{0.75f};
constexpr float kMaxUiScale{3.0f};

} // namespace

float EffectiveUiScale(const AppState &app)
{
    return std::clamp(app.settings.ui_scale.value_or(ui::SystemScale()), kMinUiScale, kMaxUiScale);
}

void SetUiScale(AppState &app, std::optional<float> scale)
{
    if (scale)
    {
        scale = std::clamp(*scale, kMinUiScale, kMaxUiScale);
    }
    if (scale == app.settings.ui_scale)
    {
        return;
    }
    app.settings.ui_scale = scale;

    const std::string description = scale ? std::format("UI scale: {:.0f}%", *scale * 100.0f)
                                          : std::format("UI scale: automatic ({:.0f}%)", ui::SystemScale() * 100.0f);
    if (app.settings_path.empty())
    {
        SetStatus(app, description);
    }
    else if (const auto saved = SaveSettings(app.settings, app.settings_path); saved)
    {
        SetStatus(app, description + " (saved)");
    }
    else
    {
        SetStatus(app, std::format("{} (not saved: {})", description, saved.error()));
    }
}

} // namespace umbra::app
