#include "_def.h"


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

constant_buffer(cb, 1) {
    BlurData g_data;
};

main_entry(main) {
    main_get_uv(uv);
    
    float3 color = g_data.gaussianp[0].weight * sample_texture(uv).rgb;
    
    do_unroll
    for (uint i = 1u; i < g_data.parts; ++i) {
        float w = g_data.gaussianp[i].weight;
        float o = g_data.gaussianp[i].offset;
        float2 du = g_data.dir * o;

        float3 c_pos = sample_texture(uv + du).rgb;
        float3 c_neg = sample_texture(uv - du).rgb;

        color += w * c_pos;
        color += w * c_neg;
    }

    return_output(float4(color, 1.0));
}