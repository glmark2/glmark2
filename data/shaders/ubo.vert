#version 140
attribute vec3 position;

varying vec4 Color;
varying vec2 TextureCoord;

layout(std140) uniform ublock {
    vec4 color;
};

void main(void)
{
    gl_Position = vec4(position, 1.0);

    Color = color;
}
