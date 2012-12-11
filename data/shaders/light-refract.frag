uniform sampler2D DistanceMap;
uniform sampler2D NormalMap;
uniform sampler2D ImageMap;

varying vec3 vertex_normal;
varying vec4 vertex_position;
varying vec4 MapCoord;

void main()
{
    const vec4 lightSpecular = vec4(0.8, 0.8, 0.8, 1.0);
    const vec4 matSpecular = vec4(1.0, 1.0, 1.0, 1.0);
    const float matShininess = 100.0;
    const vec2 point_five = vec2(0.5);
    // Need the normalized eye direction and surface normal vectors to
    // compute the refraction vector through the "front" surface of the object.
    vec3 eye_direction = normalize(-vertex_position.xyz);
    vec3 normalized_normal = normalize(vertex_normal);
    vec3 front_refraction = refract(eye_direction, normalized_normal, 1.5);
    // Offset the base map coordinate by the refaction vector, and re-normalize
    // to texture coordinate space [0, 1].
    vec2 normcoord = (MapCoord.st + front_refraction.st + point_five) * point_five;
    vec4 back_normal = texture2D(NormalMap, normcoord);
    // Now refract again, using the minus normal from the lookup.
    vec3 back_refraction = refract(front_refraction, back_normal.xyz, 1.0);
    vec2 imagecoord = (normcoord + back_refraction.st + point_five) * point_five;
    vec4 texel = texture2D(ImageMap, imagecoord);
    // Add in a specular component
    vec3 light_direction = normalize(vertex_position.xyz/vertex_position.w -
                                     LightSourcePosition.xyz/LightSourcePosition.w);
    vec3 reflection = reflect(light_direction, normalized_normal);
    float specularTerm = pow(max(0.0, dot(reflection, eye_direction)), matShininess);
    vec4 specular = (lightSpecular * matSpecular);
    gl_FragColor = (specular * specularTerm) + texel;
}
