#ifdef GL_ES
precision mediump float;
#endif

varying vec4 Color;
varying vec2 TextureCoord;

void main(void)
{
    gl_FragColor = Color;
}
