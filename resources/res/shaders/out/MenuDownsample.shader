





#version 420 core





















in vec2 g_uv;

out vec4 o_frag_color;
































































uniform sampler2DMS inputTexture;


struct DownsampleData {
    vec2 resolution;
    float animation;
};

layout(std140, binding = 1) uniform cb {
    DownsampleData g_data;
};

void main() {



    vec2 uv = vec2(g_uv.x, g_uv.y);


    ivec2 pixelCoord = ivec2(uv * g_data.resolution);

    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

    #pragma unroll
    for (uint i = 0u; i < 4u; ++i) {



        sum += texelFetch(inputTexture, pixelCoord, int(i));

    }

    sum /= float(4u);
    if (sum.a > 0.003)
        sum.rgb /= sum.a;
    sum.a *= g_data.animation;

    o_frag_color = sum; return;
}
