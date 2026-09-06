#version 120
uniform sampler2D u_Image;
uniform int u_BlendMode;
uniform sampler2D u_Depth;
uniform vec4 u_Viewport;
uniform vec2 u_DepthSize;
uniform mat4 u_InverseViewProjection;
uniform vec3 u_Origin;
uniform vec3 u_ViewOrigin;
uniform vec3 u_Right;
uniform vec3 u_Up;
uniform vec3 u_Normal;
varying vec4 v_Color;
void main() {
    vec2 pixel = gl_FragCoord.xy - u_Viewport.xy;
    float depth = texture2D(u_Depth, pixel / u_DepthSize).r;
    vec4 position = u_InverseViewProjection * vec4(pixel / u_Viewport.zw * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec3 world = position.xyz / position.w;
    vec3 delta = world - u_Origin;
    vec3 local = vec3(dot(delta, u_Right), dot(delta, u_Up), dot(delta, u_Normal));
    // Compute derivatives before divergent discard; reject grazing receivers.
    vec3 receiver = normalize(cross(dFdx(world), dFdy(world)));
    if (dot(receiver, u_ViewOrigin - world) < 0.0) receiver = -receiver;
    float facing = dot(receiver, normalize(u_Normal));
    if (depth >= 0.999999 || any(greaterThan(abs(local), vec3(1.0))) || facing <= 0.1) discard;
    vec4 color = texture2D(u_Image, local.xy * vec2(0.5, -0.5) + 0.5) * v_Color;
    if (u_BlendMode == 3) color.a = v_Color.a;
    color.a *= smoothstep(0.1, 0.3, facing);
    if (u_BlendMode == 1) color.rgb *= color.a;
    else if (u_BlendMode == 2) color.rgb = mix(vec3(1.0), color.rgb, color.a);
    gl_FragColor = color;
}
