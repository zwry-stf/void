#define NO_TEXTURES
#include "../background/_def.h"


#define SAMPLE_COUNT 4u

#ifdef R2_BACKEND_D3D11
Texture2DMS<float4, SAMPLE_COUNT> inputTexture;
#elif R2_BACKEND_OPENGL
uniform sampler2DMS inputTexture;
#endif

struct DownsampleData {
    float2 resolution;
    float animation;
};

constant_buffer(cb, 1) {
    DownsampleData g_data;
};

main_entry(main) {
#ifdef R2_BACKEND_D3D11
    float2 uv = input.uv;
#elif R2_BACKEND_OPENGL
    float2 uv = g_uv;
#endif

    int2 pixelCoord = int2(uv * g_data.resolution);

    float4 sum = float4(0.0, 0.0, 0.0, 0.0);

    do_unroll
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
#ifdef R2_BACKEND_D3D11
        sum += inputTexture.Load(pixelCoord, i);
#elif R2_BACKEND_OPENGL
        sum += texelFetch(inputTexture, pixelCoord, int(i));
#endif
    }

    sum /= float(SAMPLE_COUNT);
    if (sum.a > 0.003)
        sum.rgb /= sum.a;
    sum.a *= g_data.animation;

    return_output(sum);
}