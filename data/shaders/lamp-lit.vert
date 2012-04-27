uniform mat4 projection;
uniform mat4 modelview;
varying vec3 vertex_normal;
varying vec4 vertex_position;
varying vec3 eye_direction;
in vec3 vertex;
in vec3 normal;

vec3 unitvec(vec4 v1, vec4 v2)
{
    if (v1.w == 0.0 && v2.w == 0.0)
        return vec3(v2 - v1);
    if (v1.w == 0.0)
        return vec3(-v1);
    if (v2.w == 0.0)
        return vec3(v2);
    return v2.xyz/v2.w - v1.xyz/v1.w;
}

void main()
{
    vec4 curVertex = vec4(vertex.x, vertex.y, vertex.z, 1.0);
    mat3 normalMatrix = {modelview[0].xyz, modelview[1].xyz, modelview[2].xyz};
    gl_Position = projection * modelview * curVertex;
    vertex_normal = inverse(transpose(normalMatrix)) * normal;
    vertex_position = modelview * curVertex;
    eye_direction = normalize(unitvec(vertex_position, vec4(0.0, 0.0, 0.0, 1.0)));
}
