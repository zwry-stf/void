// predef
#ifdef R2_BACKEND_OPENGL
#define __version #version
__version 420 core
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define frac fract
#define lerp mix
#define saturate(x) clamp(x, 0.0, 1.0)
#endif

// vertex input
#ifdef R2_BACKEND_D3D11
struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

#define return_output(col) \
return col
#elif R2_BACKEND_OPENGL
in float2 g_uv;

out float4 o_frag_color;

#define return_output(col) \
o_frag_color = col; \
return
#endif

// constant buffer
#ifdef R2_BACKEND_D3D11
#define constant_buffer(_name, _reg) \
cbuffer _name : register(b##_reg)
#elif R2_BACKEND_OPENGL
#define constant_buffer(_name, _reg) \
layout(std140, binding = _reg) uniform _name
#endif

// input texture
#ifndef NO_TEXTURES
#ifdef R2_BACKEND_D3D11
Texture2D inputTexture : register(t0);
SamplerState inputSampler : register(s0);

#define sample_texture(uv) \
inputTexture.Sample(inputSampler, uv)
#elif R2_BACKEND_OPENGL
layout(binding = 0) uniform sampler2D inputTexture;

#define sample_texture(uv) \
texture(inputTexture, uv)
#endif
#endif // NO_TEXTURES

#ifdef R2_BACKEND_D3D11
#define main_entry(_name) \
float4 _name(PSInput input) : SV_TARGET
#elif R2_BACKEND_OPENGL
#define main_entry(_name) \
void _name()
#endif


#ifdef R2_BACKEND_D3D11
#define main_get_uv(_name) \
float2 _name = input.uv
#elif R2_BACKEND_OPENGL
#define main_get_uv(_name) \
float2 _name = g_uv
#endif

#ifdef R2_BACKEND_D3D11
#define do_unroll [unroll]
#elif R2_BACKEND_OPENGL
#define do_unroll #pragma unroll
#endif


#define AA_SCALE 0.7