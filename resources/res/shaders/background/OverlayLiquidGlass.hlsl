#include "_def.h"
#include "noise.h"
#include "common.h"


struct OverlayData {
    float4 pos;
    float4 background;
    float4 border;
    float rounding_top;
    float rounding_bottom;
    float animation;
    float liquid_curve_value;
};

struct CompositionData {
    float4 resolution;
    
    uint blur_enabled;
    float noise_scale;
    float warp_size;
    float blend_amount;
    
    OverlayData overlay;
};

constant_buffer(cb, 1) {
    CompositionData g_data;
};


main_entry(main) {
    main_get_uv(uv);

#ifdef R2_BACKEND_D3D11
    float2 frag_coord = uv * g_data.resolution.xy;
#elif R2_BACKEND_OPENGL
    float2 frag_coord = float2(uv.x, 1.0 - uv.y) * g_data.resolution.xy;
#endif

    float4 base_color = float4(0.0, 0.0, 0.0, 1.0);
    
    // -- Menu Rounded Corner
    float2 half_size = g_data.overlay.pos.zw * 0.5;
    float2 center    = g_data.overlay.pos.xy + half_size;
    float2 center_delta = frag_coord - center;
    float radius;
    if (frag_coord.y > center.y)
        radius = g_data.overlay.rounding_bottom;
    else
        radius = g_data.overlay.rounding_top;
    float2 rel = abs(center_delta) - half_size + radius;
    float d = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - radius + 0.5;
    float aa = fwidth(d);
  
    float mask = smoothstep_custom(-aa * AA_SCALE, 0.f, -d);

    // distort uv
    float max_border_dist = max(1.0, g_data.warp_size);

    float box_disp = 0.f;
    {
        float2 half_size_inner = half_size - float2(max_border_dist, max_border_dist);
        float2 center_delta = frag_coord - center;
        float  radius_inner = radius - max_border_dist;
        float2 rel = abs(center_delta) - half_size_inner + radius_inner;
        float  d = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - radius_inner + 0.5;

        float box_disp_smooth = smoothstep_custom(max_border_dist * 1.3f, 0.0, d);

        float box_disp_linear = sqrt(saturate(1.0 - d / max_border_dist));

        box_disp = lerp(box_disp_smooth, box_disp_linear, g_data.overlay.liquid_curve_value);
    }

    float2 center_uv = center / g_data.resolution.xy;
#if R2_BACKEND_OPENGL
    center_uv.y = 1.f - center_uv.y;
#endif
    
    float factor = 0.5 + 0.5 * smoothstep_custom(0.1, 1.0, box_disp);
    float2 warped_uv = uv - center_uv;
    warped_uv *= factor;
    warped_uv += center_uv;
    
    // 
    base_color.rgb = sample_texture(warped_uv).rgb;
    base_color.rgb = lerp(base_color.rgb, g_data.overlay.border.rgb, 
        g_data.overlay.border.a * (1.f - box_disp));
    
    // blend blurred color
    float avarage = dot(base_color.rgb, float3(0.3, 0.58, 0.12));
    
    float background_avarage = max(
        dot(g_data.overlay.background.rgb, float3(0.3, 0.58, 0.12)), 0.0001f);
    float r_ratio = g_data.overlay.background.r / background_avarage;
    float g_ratio = g_data.overlay.background.g / background_avarage;
    float b_ratio = g_data.overlay.background.b / background_avarage;
    
    base_color.r = lerp(base_color.r, avarage * r_ratio, g_data.blend_amount);
    base_color.g = lerp(base_color.g, avarage * g_ratio, g_data.blend_amount);
    base_color.b = lerp(base_color.b, avarage * b_ratio, g_data.blend_amount);
    
    // noise
    if (g_data.noise_scale > 0.003) {
        float noise = hash21(frag_coord);
        float scale = 1.2 - length(base_color.rgb) * 0.2;
        scale *= 0.3f + g_data.overlay.background.a * 0.7;
        base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4);
        base_color.rgb = clamp(base_color.rgb, 0.0, 1.0);
    }
    
    // background
    base_color = blend_color(base_color, g_data.overlay.background);

    // blend
    base_color.a *= mask * g_data.overlay.animation;
    if (base_color.a < 0.004)
        discard;

    return_output(base_color);
}