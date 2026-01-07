



















struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};





































































Texture2DMS<float4, 4u> inputTexture;




struct DownsampleData {
    float2 resolution;
    float animation;
};

cbuffer cb : register(b1) {
    DownsampleData g_data;
};

float4 main(PSInput input) : SV_TARGET {

    float2 uv = input.uv;




    int2 pixelCoord = int2(uv * g_data.resolution);

    float4 sum = float4(0.0, 0.0, 0.0, 0.0);

    [unroll]
    for (uint i = 0u; i < 4u; ++i) {

        sum += inputTexture.Load(pixelCoord, i);



    }

    sum /= float(4u);
    if (sum.a > 0.003)
        sum.rgb /= sum.a;
    sum.a *= g_data.animation;

    return sum;
}
