




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
    float liquid_curve_value;
};

struct CompositionData {
    vec4 resolution;
    
    uint blur_enabled;
    float noise_scale;
    float warp_size;
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

    
    float max_border_dist = max(1.0, g_data.warp_size);

    float box_disp = 0.f;
    {
        vec2 half_size_inner = half_size - vec2(max_border_dist, max_border_dist);
        vec2 center_delta = frag_coord - center;
        float  radius_inner = radius - max_border_dist;
        vec2 rel = abs(center_delta) - half_size_inner + radius_inner;
        float  d = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - radius_inner + 0.5;

        float box_disp_smooth = smoothstep_custom(max_border_dist * 1.3f, 0.0, d);

        float box_disp_linear = sqrt(clamp(1.0 - d / max_border_dist, 0.0, 1.0));

        box_disp = mix(box_disp_smooth, box_disp_linear, g_data.overlay.liquid_curve_value);
    }

    vec2 center_uv = center / g_data.resolution.xy;



    
    float factor = 0.5 + 0.5 * smoothstep_custom(0.1, 1.0, box_disp);
    vec2 warped_uv = uv - center_uv;
    warped_uv *= factor;
    warped_uv += center_uv;
    
    
    base_color.rgb = texture(inputTexture, warped_uv).rgb;
    base_color.rgb = mix(base_color.rgb, g_data.overlay.border.rgb, 
        g_data.overlay.border.a * (1.f - box_disp));
    
    
    float avarage = dot(base_color.rgb, vec3(0.3, 0.58, 0.12));
    
    float background_avarage = max(
        dot(g_data.overlay.background.rgb, vec3(0.3, 0.58, 0.12)), 0.0001f);
    float r_ratio = g_data.overlay.background.r / background_avarage;
    float g_ratio = g_data.overlay.background.g / background_avarage;
    float b_ratio = g_data.overlay.background.b / background_avarage;
    
    base_color.r = mix(base_color.r, avarage * r_ratio, g_data.blend_amount);
    base_color.g = mix(base_color.g, avarage * g_ratio, g_data.blend_amount);
    base_color.b = mix(base_color.b, avarage * b_ratio, g_data.blend_amount);
    
    
    if (g_data.noise_scale > 0.003) {
        float noise = hash21(frag_coord);
        float scale = 1.2 - length(base_color.rgb) * 0.2;
        scale *= 0.3f + g_data.overlay.background.a * 0.7;
        base_color.rgb *= 1.0 + ((noise - 0.5) * g_data.noise_scale * scale * 0.4);
        base_color.rgb = clamp(base_color.rgb, 0.0, 1.0);
    }
    
    
    base_color = blend_color(base_color, g_data.overlay.background);

    
    base_color.a *= mask * g_data.overlay.animation;
    if (base_color.a < 0.004)
        discard;

    o_frag_color = base_color; return;
}
