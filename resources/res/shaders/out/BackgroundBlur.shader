


















struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

























Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);







































struct BlurSample {
    float weight;
    float offset;
    float2 _pad;
};

struct BlurData {
    uint samples;
    uint parts;
    float2 dir;
    
    BlurSample gaussianp[51];
};

cbuffer cb : register(b1) {
    BlurData g_data;
};

float4 main(PSInput input) : SV_TARGET {
    float2 uv = input.uv;
    
    float3 color = g_data.gaussianp[0].weight * inputTexture.Sample(inputSampler, uv).rgb;
    
    [unroll]
    for (uint i = 1u; i < g_data.parts; ++i) {
        float w = g_data.gaussianp[i].weight;
        float o = g_data.gaussianp[i].offset;
        float2 du = g_data.dir * o;

        float3 c_pos = inputTexture.Sample(inputSampler, uv + du).rgb;
        float3 c_neg = inputTexture.Sample(inputSampler, uv - du).rgb;

        color += w * c_pos;
        color += w * c_neg;
    }

    return float4(color, 1.0);
}
