


















struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

























Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);








































float hash21(float2 p) {
    p = frac(p * float2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return frac(p.x * p.y);
}




float smoothstep_custom(float edge0, float edge1, float x) {
    float t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float4 blend_color(float4 dst, float4 src) {
    float outA = src.a + dst.a * (1.0 - src.a);
    float3 prem = src.rgb * src.a
                  + dst.rgb * dst.a * (1.0 - src.a);
    
    float3 outRGB = (outA > 0.0)
                 ? (prem / outA)
                 : float3(0.0, 0.0, 0.0);

    return float4(outRGB, outA);
}



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

cbuffer cb : register(b1) {
    CompositionData g_data;
};


float4 main(PSInput input) : SV_TARGET {
    float2 uv = input.uv;


    float2 frag_coord = uv * g_data.resolution.xy;




    float4 base_color = float4(0.0, 0.0, 0.0, 1.0);
    
    
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
  
    float mask = smoothstep_custom(-aa * 0.7, 0.f, -d);
    
    
    if (g_data.blur_enabled != 0u) {
        base_color.rgb = inputTexture.Sample(inputSampler, uv).rgb;
        
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
    
    
    float2 inner_half_size = half_size - g_data.border_size;
    float inner_rounding = max(radius - g_data.border_size, 0.0);
    float2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, float2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * 0.7, -inner_dist), 0.0, 1.0);
    
    
    float4 background_color = g_data.overlay.background;
    
    float4 border_color = g_data.overlay.border;
    border_color.a *= border_mask;
    
    background_color = blend_color(background_color, border_color);
    
    base_color = blend_color(base_color, background_color);
    
    base_color.a *= mask * g_data.overlay.animation;
    if (base_color.a < 0.003f)
        discard;
    
    return base_color;
}
