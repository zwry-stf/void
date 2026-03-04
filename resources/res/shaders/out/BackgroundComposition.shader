


















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



struct CompositionData {
    float4 menu_pos;
    
    float4 background_color;
    float4 accent_color;
    
    float4 resolution;
    
    float rounding;
    float border_size;
    float sidebar_width;
    float top_bar_height;
    
    uint  blur_enabled;
    float noise_scale;
    float down_scale;
    float blend_amount;
};

cbuffer cb : register(b1) {
    CompositionData g_data;
};


float4 main(PSInput input) : SV_TARGET {
    float2 uv = input.uv;


    float2 frag_coord = uv * g_data.resolution.xy;



    
    float4 base_color = float4(0.0, 0.0, 0.0, 1.0);
    
    
    float2 half_size = (g_data.menu_pos.zw) * 0.5;
    float2 center    = g_data.menu_pos.xy + half_size;
    float2 center_delta = frag_coord - center;
    float2 rel = abs(center_delta) - half_size + g_data.rounding;
    float dist = length(max(rel, 0.0)) + min(max(rel.x, rel.y), 0.0) - g_data.rounding + 0.5;
    float aa = fwidth(dist);
  
    float mask = smoothstep_custom(-aa * 0.7, 0.f, -dist);
    
    
    if (g_data.blur_enabled != 0u &&
        (frag_coord.x >= g_data.menu_pos.x && frag_coord.y >= g_data.menu_pos.y &&
        frag_coord.x <= g_data.menu_pos.x + g_data.menu_pos.z && 
            frag_coord.y <= g_data.menu_pos.y + g_data.menu_pos.w)) {
        base_color.rgb = inputTexture.Sample(inputSampler, uv).rgb;
        
        float avarage = dot(base_color.rgb, float3(0.3, 0.58, 0.12));
        
        float background_avarage = max(
            dot(g_data.background_color.rgb, float3(0.3, 0.58, 0.12)), 0.0001f);
        float r_ratio = g_data.background_color.r / background_avarage;
        float g_ratio = g_data.background_color.g / background_avarage;
        float b_ratio = g_data.background_color.b / background_avarage;
        
        float blend_amount = g_data.blend_amount * sqrt(background_avarage);
        base_color.r = lerp(base_color.r, avarage * r_ratio, blend_amount);
        base_color.g = lerp(base_color.g, avarage * g_ratio, blend_amount);
        base_color.b = lerp(base_color.b, avarage * b_ratio, blend_amount);
        
        
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
    
    
    float2 inner_half_size = g_data.menu_pos.zw * 0.5 - g_data.border_size;
    float inner_rounding = max(g_data.rounding - g_data.border_size, 0.0);
    float2 inner_q = abs(frag_coord - center) - (inner_half_size - inner_rounding);
    float inner_dist = length(max(inner_q, float2(0.0, 0.0))) + min(max(inner_q.x, inner_q.y), 0.0) - inner_rounding - 0.5;
    float inner_aa = fwidth(inner_dist);
    float border_mask = 1.0 - clamp(smoothstep_custom(0.0, inner_aa * 0.7, -inner_dist), 0.0, 1.0);
    
    
    float _dist = frag_coord.x - g_data.menu_pos.x;
    float alpha = 1.0 - smoothstep_custom(g_data.sidebar_width - 0.5, g_data.sidebar_width + 0.5, _dist);
    float4 bg_color = g_data.background_color;
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
        alpha = saturate(alpha);
        border_mask *= (0.7 + alpha * 0.6);
    }
    
    
    bg_color = blend_color(bg_color, float4(g_data.accent_color.rgb, g_data.accent_color.a * border_mask));
    
    base_color = blend_color(base_color, bg_color);

    
    base_color.a *= mask;
    
    return base_color;
}
