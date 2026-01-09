#include "_def.h"


constant_buffer(cb_down, 1) {
    float2 g_inv_src_size;
    float2 _pad;
};

main_entry(main) {
#ifdef R2_BACKEND_D3D11
    float2 uv = input.uv;
#elif R2_BACKEND_OPENGL
    float2 uv = g_uv;
#endif

    float2 t = g_inv_src_size;
    float3 c = float3(0.0, 0.0, 0.0);

    c += sample_texture(uv).rgb * 0.25;

    c += sample_texture(uv + t * float2(-0.5, -0.5)).rgb * 0.125;
    c += sample_texture(uv + t * float2( 0.5, -0.5)).rgb * 0.125;
    c += sample_texture(uv + t * float2(-0.5,  0.5)).rgb * 0.125;
    c += sample_texture(uv + t * float2( 0.5,  0.5)).rgb * 0.125;

    c += sample_texture(uv + t * float2(-1.5, -1.5)).rgb * 0.0625;
    c += sample_texture(uv + t * float2( 1.5, -1.5)).rgb * 0.0625;
    c += sample_texture(uv + t * float2(-1.5,  1.5)).rgb * 0.0625;
    c += sample_texture(uv + t * float2( 1.5,  1.5)).rgb * 0.0625;

    return_output(float4(c, 1.0));
}