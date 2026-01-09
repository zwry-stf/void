


















struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

























Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);







































cbuffer cb_down : register(b1) {
    float2 g_inv_src_size;
    float2 _pad;
};

float4 main(PSInput input) : SV_TARGET {
    float2 uv = input.uv;

    float2 t = g_inv_src_size;
    float3 c = float3(0.0, 0.0, 0.0);

    c += inputTexture.Sample(inputSampler, uv).rgb * 0.25;

    c += inputTexture.Sample(inputSampler, uv + t * float2(-0.5, -0.5)).rgb * 0.125;
    c += inputTexture.Sample(inputSampler, uv + t * float2( 0.5, -0.5)).rgb * 0.125;
    c += inputTexture.Sample(inputSampler, uv + t * float2(-0.5, 0.5)).rgb * 0.125;
    c += inputTexture.Sample(inputSampler, uv + t * float2( 0.5, 0.5)).rgb * 0.125;

    c += inputTexture.Sample(inputSampler, uv + t * float2(-1.5, -1.5)).rgb * 0.0625;
    c += inputTexture.Sample(inputSampler, uv + t * float2( 1.5, -1.5)).rgb * 0.0625;
    c += inputTexture.Sample(inputSampler, uv + t * float2(-1.5, 1.5)).rgb * 0.0625;
    c += inputTexture.Sample(inputSampler, uv + t * float2( 1.5, 1.5)).rgb * 0.0625;

    return float4(c, 1.0);
}
