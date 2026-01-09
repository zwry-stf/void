




#version 420 core





















in vec2 g_uv;

out vec4 o_frag_color;
























layout(binding = 0) uniform sampler2D inputTexture;



































float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}




float smoothstep_custom(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

vec4 blend_color(vec4 dst, vec4 src) {
    float outA = src.a + dst.a * (1.0 - src.a);
    vec3 prem = src.rgb * src.a
                  + dst.rgb * dst.a * (1.0 - src.a);
    
    vec3 outRGB = (outA > 0.0)
                 ? (prem / outA)
                 : vec3(0.0, 0.0, 0.0);

    return vec4(outRGB, outA);
}



struct CompositionData {
    vec4 menu_pos;
    
    vec4 background_color;
    vec4 accent_color;
    vec4 shadow_color;
    
    vec4 resolution;
    
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

layout(std140, binding = 1) uniform cb {
    CompositionData g_data;
};


void main() {
    vec2 uv = g_uv;




    vec2 frag_coord = vec2(uv.x, 1.f - uv.y) * g_data.resolution.xy;

    
    vec4 base_color = vec4(0.0, 0.0, 0.0, 1.0);
    
    
    vec2 half_size = (g_data.menu_pos.zw) * 0.5;
    vec2 center    = g_data.menu_pos.xy + half_size;
    vec2 center_delta = frag_coord - center;
    vec2 rel = abs(center_delta) - half_size + g_data.rounding;
    float dist = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - g_data.rounding + 0.5;
    float aa = fwidth(dist);
  
    float mask = smoothstep_custom(-aa * 0.7, 0.f, -dist);
    
    
    if (g_data.blur_enabled != 0u &&
        (frag_coord.x >= g_data.menu_pos.x && frag_coord.y >= g_data.menu_pos.y &&
        frag_coord.x <= g_data.menu_pos.x + g_data.menu_pos.z && 
            frag_coord.y <= g_data.menu_pos.y + g_data.menu_pos.w)) {
        base_color.rgb = texture(inputTexture, uv).rgb;
        
        float avarage = dot(base_color.rgb, vec3(0.3, 0.58, 0.12));
        
        float background_avarage = max(
            dot(g_data.background_color.rgb, vec3(0.3, 0.58, 0.12)), 0.0001f);
        float r_ratio = g_data.background_color.r / background_avarage;
        float g_ratio = g_data.background_color.g / background_avarage;
        float b_ratio = g_data.background_color.b / background_avarage;
        
        base_color.r = mix(base_color.r, avarage * r_ratio, g_data.blend_amount);
        base_color.g = mix(base_color.g, avarage * g_ratio, g_data.blend_amount);
        base_color.b = mix(base_color.b, avarage * b_ratio, g_data.blend_amount);
        
        
        if (g_data.noise_scale > 0.003)
        {
            float noise = hash21(frag_coord);
            float scale = 1.2f - length(base_color.rgb) * 0.2;
            scale *= 0.3f + g_data.background_color.a * 0.7;
            base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4f);
            base_color.rgb = clamp(base_color.rgb, 0.0, 1.0);
        }
    }
    else {
        base_color.a = 0.0;
    }

    
    if (mask < 0.99) {
        vec4 shadowColor = g_data.shadow_color;
        float radius = g_data.shadow_size * 0.5;
        float shadowDist = dist + radius;
        float shadowWeight = exp(-0.693  * (shadowDist * shadowDist) / (radius * radius));
        shadowColor.a *= clamp(shadowWeight, 0.0, 1.0) * (1.0 - mask);
    
        if (shadowWeight < 0.004 && mask < 0.004)
            discard;

        base_color = blend_color(shadowColor, base_color);
    }
    
    
    vec2 inner_half_size = g_data.menu_pos.zw * 0.5 - g_data.border_size;
    float inner_rounding = max(g_data.rounding - g_data.border_size, 0.0);
    vec2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, vec2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * 0.7, -inner_dist), 0.0, 1.0);
    
    
    float _dist = frag_coord.x - g_data.menu_pos.x;
    float alpha = 1.0 - smoothstep_custom(g_data.sidebar_width - 0.5, g_data.sidebar_width + 0.5, _dist);
    vec4 bgColor = g_data.background_color;
    float alpha2 = smoothstep_custom(g_data.sidebar_width - 0.5, g_data.sidebar_width + 0.5, _dist + g_data.border_size);
    float alpha3 = alpha2 * alpha;
    if (alpha3 > border_mask)
        border_mask = alpha3;
    
    
    if (g_data.top_bar_height > 0.0) {
        _dist = frag_coord.y - g_data.menu_pos.y;
        alpha = 1.0 - smoothstep_custom(g_data.top_bar_height - 0.5, g_data.top_bar_height + 0.5, _dist);
        alpha3 = smoothstep_custom(g_data.top_bar_height - 0.5,
            g_data.top_bar_height + 0.5, _dist + g_data.border_size) * alpha * alpha2; 
        if (alpha3 > border_mask)
            border_mask = alpha3;
    }
    
    
    {
        float o_ = g_data.menu_pos.w * 0.25;
        float ypos = frag_coord.y - g_data.menu_pos.y;
        ypos -= g_data.menu_pos.w * 0.5;
        float alpha = exp(-((ypos * ypos) / (2.0 * (o_ * o_))));
        alpha = clamp(alpha, 0.0, 1.0);
        border_mask *= (0.7 + alpha * 0.6);
    }
    
    
    bgColor = blend_color(bgColor, vec4(g_data.accent_color.rgb, g_data.accent_color.a * border_mask));
    
    base_color = blend_color(base_color, bgColor);

    
    base_color.a *= mask;
    
    o_frag_color = base_color; return;
}
