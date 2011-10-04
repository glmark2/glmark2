attribute vec3 position;

uniform mat4 ModelViewProjectionMatrix;

void main(void)
{
    // Transform the position to clip coordinates
    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
