uniform sampler2D tex;
out varying vec4 fragColor;

void main()
{
    vec2 curPos;
    curPos.x = float(int(gl_FragCoord.x) % 32) / 32.0;
    curPos.y = float(int(gl_FragCoord.y) % 32) / 32.0;
    vec4 color = texture(tex, curPos);
    if (color.w < 0.5)
        discard;
    fragColor = color;
}
