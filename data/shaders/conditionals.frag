varying vec4 dummy;

void main(void)
{
    // should be declared highp since the multiplication can overflow in
    // mediump, particularly if mediump is implemented as fp16
    HIGHP_OR_DEFAULT vec2 FragCoord = gl_FragCoord.xy;
    float d = fract(FragCoord.x * FragCoord.y * 0.0001);

$MAIN$

    gl_FragColor = vec4(d, d, d, 1.0);
}

