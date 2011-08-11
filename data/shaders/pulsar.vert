attribute vec3 position;
attribute vec4 vertex_color;

uniform mat4 ModelViewProjectionMatrix;

varying vec4 Color;
varying vec2 TextureCoord;

void main(void)
{
    Color = vertex_color;

    // Set the texture coordinates as a varying
    TextureCoord = position.xy * 0.5 + 0.5;

    // Transform the position to clip coordinates
    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
