


#version 330 core













layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 i_uv;

out vec2 g_uv;






void main()

{






    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
    g_uv = vec2(i_uv.x, 1.0 - i_uv.y);

}
