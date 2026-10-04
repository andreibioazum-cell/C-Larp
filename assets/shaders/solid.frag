#version 450

layout(location = 1) in vec4 v_col;
layout(location = 0) out vec4 out_color;
void main() {
    out_color = v_col;
}
