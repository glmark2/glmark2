#ifdef GL_ES
precision mediump float;
#endif

varying vec3 vertex_normal;
varying vec4 vertex_position;

void main(void)
{
    const vec4 lightAmbient = vec4(0.1, 0.1, 0.1, 1.0);
    const vec4 lightDiffuse = vec4(0.8, 0.8, 0.8, 1.0);
    const vec4 lightSpecular = vec4(0.8, 0.8, 0.8, 1.0);
    const vec4 matAmbient = vec4(1.0, 1.0, 1.0, 1.0);
    const vec4 matDiffuse = vec4(0.0, 0.0, 1.0, 1.0);
    const vec4 matSpecular = vec4(1.0, 1.0, 1.0, 1.0);
    const float matShininess = 100.0;
    vec3 eye_direction = normalize(-vertex_position.xyz);
    vec3 light_direction = normalize(LightSourcePosition.xyz/LightSourcePosition.w -
                                     vertex_position.xyz/vertex_position.w);
    vec3 normalized_normal = normalize(vertex_normal);
    vec3 reflection = reflect(-light_direction, normalized_normal);
    float specularTerm = pow(max(0.0, dot(reflection, eye_direction)), matShininess);
    float diffuseTerm = max(0.0, dot(normalized_normal, light_direction));
    vec4 specular = (lightSpecular * matSpecular);
    vec4 ambient = (lightAmbient * matAmbient);
    vec4 diffuse = (lightDiffuse * matDiffuse);
    gl_FragColor = (specular * specularTerm) + ambient + (diffuse * diffuseTerm);
}
