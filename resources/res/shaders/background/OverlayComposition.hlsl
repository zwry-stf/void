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
};

struct CompositionData {
    float4 resolution;

    uint  blur_enabled;
    float noise_scale;
    float border_size;
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
    float2 frag_coord = vec2(uv.x, 1.0 - uv.y) * g_data.resolution.xy;
#endif

    float4 base_color = float4(0.0, 0.0, 0.0, 1.0);
    
    // overlay rounded rect
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
    
    // blur + blend
    if (g_data.blur_enabled != 0u) {
        base_color.rgb = sample_texture(uv).rgb;
        
        float avarage = dot(base_color.rgb, float3(0.3, 0.58, 0.12));
        
        float background_avarage = max(
            dot(g_data.overlay.background.rgb, float3(0.3, 0.58, 0.12)), 0.0001f);
        float r_ratio = g_data.overlay.background.r / background_avarage;
        float g_ratio = g_data.overlay.background.g / background_avarage;
        float b_ratio = g_data.overlay.background.b / background_avarage;
        
        base_color.r = lerp(base_color.r, avarage * r_ratio, g_data.blend_amount);
        base_color.g = lerp(base_color.g, avarage * g_ratio, g_data.blend_amount);
        base_color.b = lerp(base_color.b, avarage * b_ratio, g_data.blend_amount);
    }
    
    // noise
    if (g_data.blur_enabled != 0u && 
        g_data.noise_scale > 0.003) {
        float noise = hash21(frag_coord);
        float scale = 1.2 - length(base_color.rgb) * 0.2;
        scale *= 0.3f + g_data.overlay.background.a * 0.7;
        base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4);
        base_color.rgb = clamp(base_color.rgb, 0.0, 1.0);
    }
    
    // apply menu mask
    if (g_data.blur_enabled == 0u)
        base_color.a = 0.0;
    
    // border
    float2 inner_half_size = half_size - g_data.border_size;
    float inner_rounding = max(radius - g_data.border_size, 0.0);
    float2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, float2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * AA_SCALE, -inner_dist), 0.0, 1.0);
    
    // mask to menu
    float4 background_color = g_data.overlay.background;
    
    float4 border_color = g_data.overlay.border;
    border_color.a *= border_mask;
    
    background_color = blend_color(background_color, border_color);
    
    base_color = blend_color(base_color, background_color);
    
    base_color.a *= mask * g_data.overlay.animation;
    if (base_color.a < 0.003f)
        discard;
    
    return_output(base_color);
}