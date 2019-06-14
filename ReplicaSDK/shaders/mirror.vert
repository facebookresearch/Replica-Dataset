// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 2) in vec3 normal;
layout (location = 8) in vec2 texCoord;

uniform mat4 MV_matrix;
uniform mat4 MVP_matrix;

out block
{
    vec2 texCoord;
    vec3 normal;
    vec3 eyeSpacePos;
    noperspective vec2 screenCoord;
} Out;

void main()
{
    Out.texCoord = texCoord;
    Out.normal = mat3(MV_matrix) * normal;
    Out.eyeSpacePos = vec3(MV_matrix * position);
    gl_Position = MVP_matrix * position;
    Out.screenCoord = (gl_Position.xy / gl_Position.w)*0.5+0.5;    
}
