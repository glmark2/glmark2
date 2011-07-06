#ifdef GL_ES
precision mediump float;
#endif

varying vec4 dummy;

void main(void)
{
    float n = fract(gl_FragCoord.x * gl_FragCoord.y * 0.0001);
