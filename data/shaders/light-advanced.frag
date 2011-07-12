#ifdef GL_ES
precision mediump float;
#endif

varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{
    const vec4 LightSourceAmbient = vec4(0.1, 0.1, 0.1, 1.0);
    const vec4 LightSourceDiffuse = vec4(0.8, 0.8, 0.8, 1.0);
    const vec4 LightSourceSpecular = vec4(0.8, 0.8, 0.8, 1.0);
    const vec4 MaterialAmbient = vec4(1.0, 1.0, 1.0, 1.0);
    const vec4 MaterialDiffuse = vec4(0.0, 0.0, 1.0, 1.0);
    const vec4 MaterialSpecular = vec4(1.0, 1.0, 1.0, 1.0);
    const float MaterialShininess = 100.0;

    vec3 N = normalize(Normal);
    vec3 L = normalize(Light);
    vec3 H = normalize(HalfVector);

    // Calculate the diffuse color according to Lambertian reflectance
    vec4 diffuse = MaterialDiffuse * LightSourceDiffuse * max(dot(N, L), 0.0);

    // Calculate the ambient color
    vec4 ambient = MaterialAmbient * LightSourceAmbient;

    // Calculate the specular color according to the Blinn-Phong model
    vec4 specular = MaterialSpecular * LightSourceSpecular *
                    pow(max(dot(N,H), 0.0), MaterialShininess);

    // Calculate the final color
    gl_FragColor = ambient + specular + diffuse;
}
