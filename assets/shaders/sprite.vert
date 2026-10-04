#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_col;
layout(push_constant) uniform Push {
    vec4 u_screen;
} pc;
layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_col;
void main() {
    gl_Position = vec4(in_pos.x * pc.u_screen.x + pc.u_screen.z,
                       in_pos.y * pc.u_screen.y + pc.u_screen.w, 0.0, 1.0);
    v_uv = in_uv;
    v_col = in_col;
}
