




#version 420 core





















in vec2 g_uv;

out vec4 o_frag_color;
























layout(binding = 0) uniform sampler2D inputTexture;


































layout(std140, binding = 1) uniform cb_down {
    vec2 g_inv_src_size;
    vec2 _pad;
};

void main() {



    vec2 uv = g_uv;


    vec2 t = g_inv_src_size;
    vec3 c = vec3(0.0, 0.0, 0.0);

    c += texture(inputTexture, uv).rgb * 0.25;

    c += texture(inputTexture, uv + t * vec2(-0.5, -0.5)).rgb * 0.125;
    c += texture(inputTexture, uv + t * vec2( 0.5, -0.5)).rgb * 0.125;
    c += texture(inputTexture, uv + t * vec2(-0.5, 0.5)).rgb * 0.125;
    c += texture(inputTexture, uv + t * vec2( 0.5, 0.5)).rgb * 0.125;

    c += texture(inputTexture, uv + t * vec2(-1.5, -1.5)).rgb * 0.0625;
    c += texture(inputTexture, uv + t * vec2( 1.5, -1.5)).rgb * 0.0625;
    c += texture(inputTexture, uv + t * vec2(-1.5, 1.5)).rgb * 0.0625;
    c += texture(inputTexture, uv + t * vec2( 1.5, 1.5)).rgb * 0.0625;

    o_frag_color = vec4(c, 1.0); return;
}
