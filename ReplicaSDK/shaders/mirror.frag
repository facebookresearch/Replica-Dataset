// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#version 430 core

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D reflectionTex;
layout(binding = 1) uniform sampler2D maskTex;

uniform float reflectivity;
uniform vec2 texSize;

in block
{
    vec2 texCoord;
    vec3 normal;
    vec3 eyeSpacePos;
    noperspective vec2 screenCoord;
} In;

void main()
{
    vec3 n = normalize(In.normal);
    vec3 v = normalize(In.eyeSpacePos);
    float fresnel = reflectivity + (1.0 - reflectivity) * pow(1.0 - abs(dot(n, v)), 5.0);

    vec4 c = texture(reflectionTex, In.screenCoord);
    float mask = texture(maskTex, In.texCoord).x;

    FragColor = vec4(c.rgb, fresnel * mask);
}
