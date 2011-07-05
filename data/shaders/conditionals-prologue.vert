attribute vec3 position;

uniform mat4 ModelViewProjectionMatrix;

// Removing this varying causes an inexplicable performance regression
// with r600g... Keeping it for now.
varying vec4 dummy;

void main(void)
{
    dummy = vec4(1.0);

    float d = fract(position.x);

