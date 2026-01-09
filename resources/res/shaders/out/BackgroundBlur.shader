




#version 420 core





















in vec2 g_uv;

out vec4 o_frag_color;
























layout(binding = 0) uniform sampler2D inputTexture;


































struct BlurSample {
    float weight;
    float offset;
    vec2 _pad;
};

struct BlurData {
    uint samples;
    uint parts;
    vec2 dir;
    
    BlurSample gaussianp[51];
};

layout(std140, binding = 1) uniform cb {
    BlurData g_data;
};

void main() {
    vec2 uv = g_uv;
    
    vec3 color = g_data.gaussianp[0].weight * texture(inputTexture, uv).rgb;
    
    #pragma unroll
    for (uint i = 1u; i < g_data.parts; ++i) {
        float w = g_data.gaussianp[i].weight;
        float o = g_data.gaussianp[i].offset;
        vec2 du = g_data.dir * o;

        vec3 c_pos = texture(inputTexture, uv + du).rgb;
        vec3 c_neg = texture(inputTexture, uv - du).rgb;

        color += w * c_pos;
        color += w * c_neg;
    }

    o_frag_color = vec4(color, 1.0); return;
}
