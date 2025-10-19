#pragma once
#include "overlay_params.h"
#include <imgui.h>
#include <string>
#include <algorithm>

inline ImFont* get_font_for(const std::string& name, overlay_params* p)
{
    if (!p) return nullptr;
    if (name == "cpu" && p->cpu_font) return p->cpu_font;
    if (name == "gpu" && p->gpu_font) return p->gpu_font;
    if (name == "vram" && p->vram_font) return p->vram_font;
    if (name == "ram" && p->ram_font) return p->ram_font;
    if (name == "fps" && p->fps_font) return p->fps_font;
    if (name == "frametime" && p->frametime_font) return p->frametime_font;
    if (name == "custom" && p->custom_font) return p->custom_font;
    if (name == "title" && p->title_font) return p->title_font;
    return p->font; // fallback global
}

inline ImVec4 get_color_for(const std::string& name, overlay_params* p)
{
    if (!p) return ImVec4(1,1,1,1);
    auto to_col = [](unsigned c){return ImGui::ColorConvertU32ToFloat4(c);};
    if (name == "cpu") return to_col(p->cpu_color);
    if (name == "gpu") return to_col(p->gpu_color);
    if (name == "vram") return to_col(p->vram_color);
    if (name == "ram") return to_col(p->ram_color);
    if (name == "fps") return to_col(p->fps_color);
    if (name == "frametime") return to_col(p->frametime_color);
    if (name == "custom") return to_col(p->custom_text_color);
    if (name == "title") return to_col(p->title_color);
    return to_col(p->text_color);
}
