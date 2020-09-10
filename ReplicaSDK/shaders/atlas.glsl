// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
uniform int tileSize;
uniform int widthInTiles;

layout(std430, binding = 1) buffer MeshAdjFaces
{
    uint meshAdjFaces[];
};

ivec2 FaceToAtlasPos(int faceID, int tileSize)
{
    ivec2 tilePos;
    tilePos.y = faceID / widthInTiles;
    tilePos.x = faceID - (tilePos.y * widthInTiles);
    return tilePos * tileSize;
}

// rotate UVs into neighbouring face frame
// rot = number of 90 degree anti-clockwise rotations
ivec2 RotateUVs(ivec2 p, int rot, int size)
{
    switch(rot)
    {
        case 0:
            return p;
        case 1:
            return ivec2(p.y, (size - 1) - p.x);
        case 2:
            return ivec2((size - 1) - p.x, (size - 1) - p.y);
        case 3:
            return ivec2((size - 1) - p.y, p.x);
    }
}

const int ROTATION_SHIFT = 30;
const int FACE_MASK = 0x3FFFFFFF;

int GetAdjFace(int face, int edge, out int rot)
{
    uint data = meshAdjFaces[face * 4 + edge];
    rot = int(data >> ROTATION_SHIFT);
    return int(data & FACE_MASK);
}

bool IsValid(int adjFace)
{
    return adjFace != FACE_MASK;
}

// fetch texel from atlas
// p is integer tile coordinate
// is p is outside tile, fetches from correct adjacent tile, taking account of rotation
int indexAdjacentFaces(int faceID, inout ivec2 p, int tsize)
{
    int rot;
    // edge 0
    if(p.y < 0)
    {
        int adjFace = GetAdjFace(faceID, 0, rot);

        if (IsValid(adjFace))
        {
            p.y += tsize;
            if(p.x > tsize - 1)
            {
                p.x -= tsize;
                p = RotateUVs(p, rot, tsize);

                adjFace = GetAdjFace(adjFace, (1 - rot) & 3, rot);
                if (IsValid(adjFace))
                {
                    p = RotateUVs(p, rot, tsize);
                    return adjFace;
                }
            }
            else if(p.x < 0)
            {
                p.x += tsize;
                p = RotateUVs(p, rot, tsize);

                adjFace = GetAdjFace(adjFace, (3 - rot) & 3, rot);
                if (IsValid(adjFace))
                {
                    p = RotateUVs(p, rot, tsize);
                    return adjFace;
                }
            }
            else
            {
                p = RotateUVs(p, rot, tsize);
                return adjFace;
            }
        }
    }
    // edge 2
    else if(p.y > tsize - 1)
    {
        int adjFace = GetAdjFace(faceID, 2, rot);

        if (IsValid(adjFace))
        {
            p.y -= tsize;
            if(p.x > tsize - 1)
            {
                p.x -= tsize;
                p = RotateUVs(p, rot, tsize);

                adjFace = GetAdjFace(adjFace, (1 - rot) & 3, rot);
                if (IsValid(adjFace))
                {
                    p = RotateUVs(p, rot, tsize);
                    return adjFace;
                }
            }
            else if(p.x < 0)
            {
                p.x += tsize;
                p = RotateUVs(p, rot, tsize);

                adjFace = GetAdjFace(adjFace, (3 - rot) & 3, rot);
                if (IsValid(adjFace))
                {
                    p = RotateUVs(p, rot, tsize);
                    return adjFace;
                }
            }
            else
            {
                p = RotateUVs(p, rot, tsize);
                return adjFace;
            }
        }
    }
    else
    {
        // edge 3
        if(p.x < 0)
        {
            int adjFace = GetAdjFace(faceID, 3, rot);
            if(IsValid(adjFace))
            {
                p.x += tsize;
                p = RotateUVs(p, rot, tsize);
                return adjFace;
            }
        }
        // edge 1
        else if(p.x > tsize - 1)
        {
            int adjFace = GetAdjFace(faceID, 1, rot);
            if(IsValid(adjFace))
            {
                p.x -= tsize;
                p = RotateUVs(p, rot, tsize);
                return adjFace;
            }
        }
    }

    return faceID;
}

// load texel from atlas, handling adjacent faces
vec4 texelFetchAtlasAdj(sampler2D tex, int faceID, ivec2 p)
{
    // fetch from adjacent face if necessary
    faceID = indexAdjacentFaces(faceID, p, tileSize);

    // clamp to tile edge
    p = clamp(p, ivec2(0, 0), ivec2(tileSize - 1, tileSize - 1));

    ivec2 atlasPos = FaceToAtlasPos(faceID, tileSize);
    return texelFetch(tex, atlasPos + p, 0);
}

// fetch with bilinear filtering
vec4 textureAtlas(sampler2D tex, int faceID, vec2 p)
{
    p -= 0.5;
    ivec2 i = ivec2(floor(p));
    vec2 f = p - vec2(i);
    return mix(mix(texelFetchAtlasAdj(tex, faceID, ivec2(i)),
                   texelFetchAtlasAdj(tex, faceID, ivec2(i.x + 1, i.y)),
                   f.x),
               mix(texelFetchAtlasAdj(tex, faceID, ivec2(i.x, i.y + 1)),
                   texelFetchAtlasAdj(tex, faceID, ivec2(i.x + 1, i.y + 1)),
                   f.x),
               f.y);
}

void applySaturation(inout vec4 c, float saturation)
{
    float Pr = 0.299f;
    float Pg = 0.587f;
    float Pb = 0.114f;

    const float P = sqrt(c.r * c.r * Pr + c.g * c.g * Pg + c.b * c.b * Pb);

    c.r = P + (c.r - P) * saturation;
    c.g = P + (c.g - P) * saturation;
    c.b = P + (c.b - P) * saturation;
}
