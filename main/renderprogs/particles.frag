#version 120
uniform sampler2D u_Image;
uniform int u_BlendMode;
uniform sampler2D u_Depth;
uniform vec4 u_Viewport;
uniform vec2 u_DepthSize;
uniform vec2 u_Projection;
uniform float u_Softness;
varying vec2 v_TexCoord;
varying vec4 v_Color;
float eyeDepth(float depth) {
    return u_Projection.y / (depth * 2.0 - 1.0 + u_Projection.x);
}
void main() {
    vec2 pixel = gl_FragCoord.xy - u_Viewport.xy;
    float scene = texture2D(u_Depth, pixel / u_DepthSize).r;
    float separation = eyeDepth(scene) - eyeDepth(gl_FragCoord.z);
    float fade = clamp(separation / u_Softness, 0.0, 1.0);
    vec4 color = texture2D(u_Image, v_TexCoord) * v_Color;
    if (u_BlendMode == 3) color.a = v_Color.a;
    color.a *= fade;
    if (u_BlendMode == 1) color.rgb *= color.a;
    else if (u_BlendMode == 2) color.rgb = mix(vec3(1.0), color.rgb, color.a);
    gl_FragColor = color;
}
