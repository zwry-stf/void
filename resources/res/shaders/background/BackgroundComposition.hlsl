#include "_def.h"
#include "noise.h"
#include "common.h"


struct CompositionData {
    float4 menu_pos;
    
    float4 background_color;
    float4 accent_color;
    float4 shadow_color;
    
    float4 resolution;
    
    float rounding;
    float border_size;
    float sidebar_width;
    float top_bar_height;
    
    uint  blur_enabled;
    float noise_scale;
    float shadow_size;
    float down_scale;
    float blend_amount;
};

constant_buffer(cb, 1) {
    CompositionData g_data;
};


main_entry(main) {
    main_get_uv(uv);

    float2 frag_coord = uv * g_data.resolution.xy;
    
    float4 base_color = float4(0.0, 0.0, 0.0, 1.0);
    
    // menu rounded rect
    float2 half_size = (g_data.menu_pos.zw) * 0.5;
    float2 center    = g_data.menu_pos.xy + half_size;
    float2 center_delta = frag_coord - center;
    float2 rel = abs(center_delta) - half_size + g_data.rounding;
    float dist = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - g_data.rounding + 0.5;
    float aa = fwidth(dist);
  
    float mask = smoothstep_custom(-aa * AA_SCALE, 0.f, -dist);
    
    // blur + blend
    if (g_data.blur_enabled != 0u &&
        (frag_coord.x >= g_data.menu_pos.x && frag_coord.y >= g_data.menu_pos.y &&
        frag_coord.x <= g_data.menu_pos.x + g_data.menu_pos.z && 
            frag_coord.y <= g_data.menu_pos.y + g_data.menu_pos.w)) {
        base_color.rgb = sample_texture(uv).rgb;
        
        float avarage = dot(base_color.rgb, float3(0.3, 0.58, 0.12));
        
        float background_avarage = max(
            dot(g_data.background_color.rgb, float3(0.3, 0.58, 0.12)), 0.0001f);
        float r_ratio = g_data.background_color.r / background_avarage;
        float g_ratio = g_data.background_color.g / background_avarage;
        float b_ratio = g_data.background_color.b / background_avarage;
        
        base_color.r = lerp(base_color.r, avarage * r_ratio, g_data.blend_amount);
        base_color.g = lerp(base_color.g, avarage * g_ratio, g_data.blend_amount);
        base_color.b = lerp(base_color.b, avarage * b_ratio, g_data.blend_amount);
        
        // noise
        if (g_data.noise_scale > 0.003)
        {
            float noise = hash21(frag_coord);
            float scale = 1.2f - length(base_color.rgb) * 0.2;
            scale *= 0.3f + g_data.background_color.a * 0.7;
            base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4f);
            base_color.rgb = saturate(base_color.rgb);
        }
    }
    else {
        base_color.a = 0.0;
    }

    // shadow
    if (mask < 0.99) {
        float4 shadowColor = g_data.shadow_color;
        float radius = g_data.shadow_size * 0.5;
        float shadowDist = dist + radius;
        float shadowWeight = exp(-0.693 /*ln(2)*/ * (shadowDist * shadowDist) / (radius * radius));
        shadowColor.a *= saturate(shadowWeight) * (1.0 - mask);
    
        if (shadowWeight < 0.004 && mask < 0.004)
            discard;

        base_color = blend_color(shadowColor, base_color);
    }
    
    // border
    float2 inner_half_size = g_data.menu_pos.zw * 0.5 - g_data.border_size;
    float inner_rounding = max(g_data.rounding - g_data.border_size, 0.0);
    float2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, float2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * AA_SCALE, -inner_dist), 0.0, 1.0);
    
    // sidebar
    float _dist = frag_coord.x - g_data.menu_pos.x;
    float alpha = 1.0 - smoothstep_custom(g_data.sidebar_width - 0.5, g_data.sidebar_width + 0.5, _dist);
    float4 bgColor = g_data.background_color;
    float alpha2 = smoothstep_custom(g_data.sidebar_width - 0.5, g_data.sidebar_width + 0.5, _dist + g_data.border_size);
    float alpha3 = alpha2 * alpha;
    if (alpha3 > border_mask)
        border_mask = alpha3;
    
    // top bar
    if (g_data.top_bar_height > 0.0) {
        _dist = frag_coord.y - g_data.menu_pos.y;
        alpha = 1.0 - smoothstep_custom(g_data.top_bar_height - 0.5, g_data.top_bar_height + 0.5, _dist);
        alpha3 = smoothstep_custom(g_data.top_bar_height - 0.5,
            g_data.top_bar_height + 0.5, _dist + g_data.border_size) * alpha * alpha2; // * alpha2 so its only on the right side of the sidebar
        if (alpha3 > border_mask)
            border_mask = alpha3;
    }
    
    // add linear gaussian to border_mask
    {
        float o_ = g_data.menu_pos.w * 0.25;
        float ypos = frag_coord.y - g_data.menu_pos.y;
        ypos -= g_data.menu_pos.w * 0.5;
        float alpha = exp(-((ypos * ypos) / (2.0 * (o_ * o_))));
        alpha = saturate(alpha);
        border_mask *= (0.7 + alpha * 0.6);
    }
    
    // blend between border and background color
    bgColor = blend_color(bgColor, float4(g_data.accent_color.rgb, g_data.accent_color.a * border_mask));
    
    base_color = blend_color(base_color, bgColor);

    // mask to menu
    base_color.a *= mask;
    
    return_output(base_color);
}