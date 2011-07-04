#ifdef GL_ES
precision mediump float;
#endif

varying vec4 Color;

float rand(vec2 n)
{
    return fract(sin(dot(n.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
    float n = rand(gl_FragCoord.xy);
