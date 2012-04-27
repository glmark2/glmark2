uniform mat4 projection;
uniform mat4 modelview;
in vec3 vertex;
out varying vec4 color;

void main()
{
    vec4 curVertex = vec4(vertex.x, vertex.y, vertex.z, 1.0);
    gl_Position = projection * modelview * curVertex;
    color = vec4(0.0, 0.0, 0.0, 1.0);
}
