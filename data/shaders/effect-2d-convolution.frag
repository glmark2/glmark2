uniform sampler2D Texture0;
varying vec2 TextureCoord;

void main(void)
{
    vec4 result;

    $CONVOLUTION$

    gl_FragColor = result;
}

