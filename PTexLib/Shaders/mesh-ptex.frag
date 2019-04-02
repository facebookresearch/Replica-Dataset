#version 430 core
#include "atlas.glsl"

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D atlasTex;

uniform float exposure;
uniform float gamma;

in vec2 uv;

void main()
{
    vec4 c = textureAtlas(atlasTex, gl_PrimitiveID, uv * tileSize);
    c.rgb = pow(c.rgb * exposure, vec3(gamma));
    FragColor = vec4(c.xyz, 1.0f);
}
