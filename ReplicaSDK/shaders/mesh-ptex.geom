#version 430 core
// generates UVs for quads

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 uv;

void main()
{
    gl_PrimitiveID = gl_PrimitiveIDIn;

    uv = vec2(1.0, 0.0);
    gl_ClipDistance[0] = gl_in[1].gl_ClipDistance[0];    
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    uv = vec2(0.0, 0.0);
    gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    uv = vec2(1.0, 1.0);
    gl_ClipDistance[0] = gl_in[2].gl_ClipDistance[0];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    uv = vec2(0.0, 1.0);
    gl_ClipDistance[0] = gl_in[3].gl_ClipDistance[0];
    gl_Position = gl_in[3].gl_Position;
    EmitVertex();

    EndPrimitive();
}
