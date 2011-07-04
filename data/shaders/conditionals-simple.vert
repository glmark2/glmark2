attribute vec3 position;

uniform mat4 ModelViewProjectionMatrix;

varying vec4 Color;

void main(void)
{
    Color = vec4(1.0);

    // Transform the position to clip coordinates
    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
