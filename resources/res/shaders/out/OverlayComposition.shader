




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



struct OverlayData {
    vec4 pos;
    vec4 background;
    vec4 border;
    float rounding_top;
    float rounding_bottom;
    float animation;
};

struct CompositionData {
    vec4 resolution;

    uint  blur_enabled;
    float noise_scale;
    float border_size;
    float blend_amount;
    
    OverlayData overlay;
};

layout(std140, binding = 1) uniform cb {
    CompositionData g_data;
};


void main()
{
    vec2 uv = g_uv;




    vec2 frag_coord = vec2(uv.x, uv.y) * g_data.resolution.xy;


    vec4 base_color = vec4(0.0, 0.0, 0.0, 1.0);
    
    
    vec2 half_size = g_data.overlay.pos.zw * 0.5;
    vec2 center    = g_data.overlay.pos.xy + half_size;
    vec2 center_delta = frag_coord - center;
    float radius;
    if (frag_coord.y > center.y)
        radius = g_data.overlay.rounding_bottom;
    else
        radius = g_data.overlay.rounding_top;
    vec2 rel = abs(center_delta) - half_size + radius;
    float d = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - radius + 0.5;
    float aa = fwidth(d);
  
    float mask = smoothstep_custom(-aa * 0.7, 0.f, -d);
    
    
    if (g_data.blur_enabled != 0u) {
        base_color.rgb = texture(inputTexture, uv).rgb;
        
        float avarage = dot(base_color.rgb, vec3(0.3, 0.58, 0.12));
        
        float background_avarage = max(
            dot(g_data.overlay.background.rgb, vec3(0.3, 0.58, 0.12)), 0.0001f);
        float r_ratio = g_data.overlay.background.r / background_avarage;
        float g_ratio = g_data.overlay.background.g / background_avarage;
        float b_ratio = g_data.overlay.background.b / background_avarage;
        
        base_color.r = mix(base_color.r, avarage * r_ratio, g_data.blend_amount);
        base_color.g = mix(base_color.g, avarage * g_ratio, g_data.blend_amount);
        base_color.b = mix(base_color.b, avarage * b_ratio, g_data.blend_amount);
    }
    
    
    if (g_data.blur_enabled != 0u && 
        g_data.noise_scale > 0.003) {
        float noise = hash21(frag_coord);
        float scale = 1.2 - length(base_color.rgb) * 0.2;
        scale *= 0.3f + g_data.overlay.background.a * 0.7;
        base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4);
        base_color.rgb = clamp(base_color.rgb, 0.0, 1.0);
    }
    
    
    if (g_data.blur_enabled == 0u)
        base_color.a = 0.0;
    
    
    vec2 inner_half_size = half_size - g_data.border_size;
    float inner_rounding = max(radius - g_data.border_size, 0.0);
    vec2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, vec2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * 0.7, -inner_dist), 0.0, 1.0);
    
    
    vec4 background_color = g_data.overlay.background;
    
    vec4 border_color = g_data.overlay.border;
    border_color.a *= border_mask;
    
    background_color = blend_color(background_color, border_color);
    
    base_color = blend_color(base_color, background_color);
    
    base_color.a *= mask * g_data.overlay.animation;
    if (base_color.a < 0.003f)
        discard;
    
    o_frag_color = base_color; return;
}
