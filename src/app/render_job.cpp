#include "app/render_job.h"

#include "core/render.h"
#include "io/image_io.h"

#include <raylib.h>

#include <chrono>
#include <filesystem>
#include <format>

namespace umbra::app
{

bool IsRendering(const AppState &app)
{
    return app.next_row < app.framebuffer.Height();
}

float RenderProgress(const AppState &app)
{
    const int height = app.framebuffer.Height();
    return height > 0 ? static_cast<float>(app.next_row) / static_cast<float>(height) : 0.0f;
}

void RestartRender(AppState &app)
{
    const Resolution &resolution = resolutions[app.resolution_index];
    if (resolution.width != app.framebuffer.Width() || resolution.height != app.framebuffer.Height())
    {
        app.framebuffer.Resize(resolution.width, resolution.height);

        if (IsTextureValid(app.texture))
        {
            UnloadTexture(app.texture);
        }
        // Black, RGBA8 like Pixel. Not raylib's BLACK: that macro expands to "Color{ 0, 0, 0, 255 }",
        // and inside namespace umbra "Color" means umbra::Color as soon as core/color.h is included.
        const ::Image blank = GenImageColor(resolution.width, resolution.height, ::Color{0, 0, 0, 255});
        app.texture = LoadTextureFromImage(blank);
        UnloadImage(blank);
        SetTextureFilter(app.texture, TEXTURE_FILTER_BILINEAR);
    }

    app.next_row = 0;
    app.compute_time = {};
    SetStatus(app, "Rendering...");
}

void AdvanceRender(AppState &app)
{
    if (!IsRendering(app))
    {
        return;
    }

    constexpr std::chrono::milliseconds frame_budget{12};
    const int width = app.framebuffer.Width();
    const int height = app.framebuffer.Height();
    const auto frame_start = Clock::now();

    while (IsRendering(app))
    {
        const int y = app.next_row;
        for (int x{0}; x < width; ++x)
        {
            app.framebuffer.PutPixel(x, y, RenderPixel(x, y, width, height));
        }
        ++app.next_row;

        if (app.slow_motion || Clock::now() - frame_start > frame_budget)
        {
            break;
        }
    }

    app.texture_dirty = true;
    app.compute_time += Clock::now() - frame_start;

    if (!IsRendering(app))
    {
        SetStatus(app, std::format("Rendered {} x {} in {:.3f} s", width, height, app.compute_time.count()));
    }
}

void UploadTexture(AppState &app)
{
    if (app.texture_dirty)
    {
        UpdateTexture(app.texture, app.framebuffer.Data());
        app.texture_dirty = false;
    }
}

void SaveSnapshot(AppState &app)
{
    const std::filesystem::path path = std::filesystem::path{"renders"} / GetTimestampedFileName();
    if (const auto written = WritePng(app.framebuffer, path); written)
    {
        SetStatus(app, std::format("Saved {}", path.string()));
    }
    else
    {
        SetStatus(app, std::format("Save failed: {}", written.error()));
    }
}

} // namespace umbra::app
