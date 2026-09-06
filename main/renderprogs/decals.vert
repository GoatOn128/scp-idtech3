#version 120
varying vec4 v_Color;
void main() {
    gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);
    v_Color = gl_Color;
}
