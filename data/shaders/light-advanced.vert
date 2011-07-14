attribute vec3 position;
attribute vec3 normal;
attribute vec2 texcoord;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 NormalMatrix;

varying vec3 Normal;
varying vec2 TextureCoord;

void main(void)
{
    TextureCoord = texcoord;

    // Transform the normal to eye coordinates
    Normal = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));

    // Transform the position to clip coordinates
    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
