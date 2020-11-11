varying vec4 dummy;

float process(float d)
{
$PROCESS$
    return d;
}

void main(void)
{
#ifdef GL_FRAGMENT_PRECISION_HIGH
    // should be declared highp since the multiplication can overflow in
    // mediump, particularly if mediump is implemented as fp16
    highp vec2 FragCoord = gl_FragCoord.xy;
#else
    vec2 FragCoord = gl_FragCoord.xy;
#endif
    float d = fract(FragCoord.x * FragCoord.y * 0.0001);

$MAIN$

    gl_FragColor = vec4(d, d, d, 1.0);
}
