// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#version 430 core

layout(location = 0) in vec4 position;

uniform mat4 MV, MVP;
uniform vec4 clipPlane;

out float depth;

void main()
{
    vec4 cameraPos = MV * position;
    depth = cameraPos.z;
    gl_ClipDistance[0] = dot(position, clipPlane);
    gl_Position = MVP * position;
}
