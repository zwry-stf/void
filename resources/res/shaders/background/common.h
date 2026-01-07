

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