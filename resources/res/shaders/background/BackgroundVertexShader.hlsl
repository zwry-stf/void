#ifdef R2_BACKEND_OPENGL
#define __version #version
__version 330 core
#endif

#ifdef R2_BACKEND_D3D11
struct VSInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};
#elif R2_BACKEND_OPENGL
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 i_uv;

out vec2 g_uv;
#endif


#ifdef R2_BACKEND_D3D11
PSInput main(VSInput input)
#elif R2_BACKEND_OPENGL
void main()
#endif
{
#ifdef R2_BACKEND_D3D11
    PSInput output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.uv = input.uv;
    return output;
#elif R2_BACKEND_OPENGL
    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
    g_uv = vec2(i_uv.x, 1.0 - i_uv.y);
#endif
}