# MangoHud Font Customization - Implementation Notes

## Remaining Manual Steps

Due to the extensive nature of changes to `src/hud_elements.cpp` (500+ lines), 
you need to add the following implementations:

### 1. Add FontScope Implementation (after line 30)

```cpp
FontScope::FontScope(const FontProperties& props, const overlay_params* params)
    : current_color(1.0f, 1.0f, 1.0f, 1.0f)
    , pushed_font(false)
{
    if (props.has_custom_color) {
        current_color = props.color;
    } else {
        current_color = HUDElements.colors.text;
    }

    if (props.has_custom_size && props.font_size > 0) {
        float scale = props.font_size / params->font_size;
        ImGui::GetFont()->Scale = scale;
        ImGui::PushFont(ImGui::GetFont());
        pushed_font = true;
    }
}

FontScope::~FontScope() {
    if (pushed_font) {
        ImGui::PopFont();
        ImGui::GetFont()->Scale = 1.0f;
    }
}

static const FontProperties& get_component_font(const std::string& component) {
    static FontProperties default_props;
    auto it = HUDElements.component_fonts.find(component);
    if (it != HUDElements.component_fonts.end()) {
        return it->second;
    }
    return default_props;
}

static void init_component_font(const std::string& component,
                                const overlay_params* params,
                                unsigned color_param,
                                float size_param,
                                bool has_color,
                                bool has_size) {
    FontProperties props;

    if (has_color) {
        ImVec4 color = ImGui::ColorConvertU32ToFloat4(color_param);
        color.w = params->alpha;
        props.color = color;
        props.has_custom_color = true;
    }

    if (has_size && size_param > 0) {
        props.font_size = size_param;
        props.has_custom_size = true;
    }

    if (props.has_custom_color || props.has_custom_size) {
        HUDElements.component_fonts[component] = props;
    }
}
```

### 2. Add Color Initialization in convert_colors() function

Add after existing color conversions (around line 190):

```cpp
    HUDElements.colors.cpu_label = convert(params.cpu_label_color);
    HUDElements.colors.gpu_label = convert(params.gpu_label_color);
    HUDElements.colors.vram_label = convert(params.vram_label_color);
    HUDElements.colors.ram_label = convert(params.ram_label_color);
    HUDElements.colors.wine_label = convert(params.wine_label_color);
    HUDElements.colors.engine_label = convert(params.engine_label_color);
    HUDElements.colors.time_label = convert(params.time_label_color);
    HUDElements.colors.fps_label = convert(params.fps_label_color);
    HUDElements.colors.resolution_label = convert(params.resolution_label_color);
    HUDElements.colors.custom_text_label = convert(params.custom_text_color);
    HUDElements.colors.title_label = convert(params.title_color);

    // Initialize component fonts
    HUDElements.component_fonts.clear();

    auto setup_font = [&params](const std::string& name,
                                 unsigned color_param,
                                 float size_param) {
        init_component_font(name, &params, color_param, size_param,
                           true, size_param > 0);
    };

    setup_font("cpu_label", params.cpu_label_color, params.cpu_label_font_size);
    setup_font("gpu_label", params.gpu_label_color, params.gpu_label_font_size);
    setup_font("vram_label", params.vram_label_color, params.vram_label_font_size);
    setup_font("ram_label", params.ram_label_color, params.ram_label_font_size);
    setup_font("wine_label", params.wine_label_color, params.wine_label_font_size);
    setup_font("engine_label", params.engine_label_color, params.engine_label_font_size);
    setup_font("time_label", params.time_label_color, params.time_label_font_size);
    setup_font("fps_label", params.fps_label_color, params.fps_label_font_size);
    setup_font("resolution_label", params.resolution_label_color, params.resolution_label_font_size);
    setup_font("custom_text", params.custom_text_color, params.custom_text_font_size);
    setup_font("title", params.title_color, params.title_font_size);

    if (params.core_p_font_size > 0 || params.core_p_color != params.cpu_color) {
        init_component_font("core_p", &params, params.core_p_color,
                           params.core_p_font_size,
                           params.core_p_color != params.cpu_color,
                           params.core_p_font_size > 0);
    }

    if (params.core_e_font_size > 0 || params.core_e_color != params.cpu_color) {
        init_component_font("core_e", &params, params.core_e_color,
                           params.core_e_font_size,
                           params.core_e_color != params.cpu_color,
                           params.core_e_font_size > 0);
    }
```

### 3. Update Component Rendering Functions

Replace unit strings with proper case:
- Find: `"MHZ"` Replace: `"MHz"`
- Find: `"GIB"` Replace: `"GiB"`
- Find: `"MIB/s"` Replace: `"MiB/s"`
- Find: `"RPM"` Replace: `"rpm"`

### 4. Add title() Function (after version() function)

```cpp
void HudElements::title(){
    if (HUDElements.params->title_text.empty())
        return;

    ImguiNextColumnFirstItem();

    FontScope font_scope(get_component_font("title"), HUDElements.params.get());

    float window_width = ImGui::GetWindowSize().x;
    ImVec2 text_size = ImGui::CalcTextSize(HUDElements.params->title_text.c_str());
    float text_x = (window_width - text_size.x) * 0.5f;

    if (text_x > 0) {
        ImGui::SetCursorPosX(text_x);
    }

    HUDElements.TextColored(font_scope.get_color(), "%s", HUDElements.params->title_text.c_str());

    if (!HUDElements.params->enabled[OVERLAY_PARAM_ENABLED_hud_compact]) {
        ImGui::Dummy(ImVec2(0.0f, HUDElements.params->font_size * 0.5f));
    }
}
```

### 5. Update core_load() to use GetFilteredCores()

In the `core_load()` function, replace the loop over `cpuStats.GetCPUData()` with:

```cpp
        auto core_groups = cpuStats.GetFilteredCores();

        for (const auto& group : core_groups)
        {
            ImguiNextColumnFirstItem();

            std::string component_name = group.is_performance_core ? "core_p" : "core_e";
            FontScope font_scope(get_component_font(component_name), HUDElements.params.get());

            const char* cpu_label = HUDElements.params->cpu_label_text.empty()
                ? "CPU"
                : HUDElements.params->cpu_label_text.c_str();

            HUDElements.TextColored(font_scope.get_color(), "%s", cpu_label);
            ImGui::SameLine(0, 1.0f);
            ImGui::PushFont(HUDElements.sw_stats->font_small);

            HUDElements.TextColored(font_scope.get_color(), "%s", group.display_name.c_str());

            ImGui::PopFont();
            ImguiNextColumnOrNewRow();
            auto text_color = HUDElements.colors.text;

            if (HUDElements.params->enabled[OVERLAY_PARAM_ENABLED_core_load_change]){
                int cpu_load_percent = int(group.combined_load);
                struct LOAD_DATA cpu_data = {
                    HUDElements.colors.cpu_load_low,
                    HUDElements.colors.cpu_load_med,
                    HUDElements.colors.cpu_load_high,
                    HUDElements.params->cpu_load_value[0],
                    HUDElements.params->cpu_load_value[1]
                };
                auto load_color = change_on_load_temp(cpu_data, cpu_load_percent);
                right_aligned_text(load_color, HUDElements.ralign_width, "%d", cpu_load_percent);
                ImGui::SameLine(0, 1.0f);
                HUDElements.TextColored(load_color, "%%");
                ImguiNextColumnOrNewRow();
            }
            else {
                right_aligned_text(text_color, HUDElements.ralign_width, "%i", int(group.combined_load));
                ImGui::SameLine(0, 1.0f);
                HUDElements.TextColored(HUDElements.colors.text, "%%");
                ImguiNextColumnOrNewRow();
            }

            right_aligned_text(HUDElements.colors.text, HUDElements.ralign_width, "%i", group.combined_freq);
            ImGui::SameLine(0, 1.0f);
            ImGui::PushFont(HUDElements.sw_stats->font_small);
            HUDElements.TextColored(HUDElements.colors.text, "MHz");
            ImGui::PopFont();
        }
```

### 6. Add title to sort_elements

In `sort_elements()` function, add to the display_params map:

```cpp
        {"title", {title}},
```

### 7. Add title to legacy_elements

Add at the beginning of `legacy_elements()`:

```cpp
    if (!HUDElements.params->title_text.empty() &&
        HUDElements.params->title_position == "top") {
        ordered_functions.push_back({title, "title", value});
    }
```

And at the end:

```cpp
    if (!HUDElements.params->title_text.empty() &&
        HUDElements.params->title_position == "bottom") {
        ordered_functions.push_back({title, "title", value});
    }
```

## Building

After making these changes, build with:

```bash
./build.sh clean
./build.sh build
./build.sh install
```

## Testing Configuration

Create `~/.config/MangoHud/MangoHud.conf`:

```ini
title_text=Gaming Performance
title_font_size=32
title_color=00FFFF
title_position=top

cpu_label_font_size=28
cpu_label_color=FF0000
cpu_cores=0-7

gpu_label_font_size=28
gpu_label_color=00FF00

vram_label_font_size=24
vram_label_color=0000FF
```

Test with:
```bash
mangohud glxgears
```

## See Full Documentation

Refer to the patch artifacts for complete implementation details.
