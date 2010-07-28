#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 LightSourceAmbient;
uniform vec3 LightSourceDiffuse;
uniform vec3 LightSourceSpecular;
uniform vec3 MaterialAmbient;
uniform vec3 MaterialDiffuse;
uniform vec3 MaterialSpecular;
uniform vec3 MaterialColor;

varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(Light);
    vec3 H = normalize(HalfVector);

    // Calculate the diffuse color according to Lambertian reflectance
    vec3 diffuse = MaterialDiffuse * LightSourceDiffuse * max(dot(N, L), 0.0);

    // Calculate the ambient color
    vec3 ambient = MaterialAmbient * LightSourceAmbient;

    // Calculate the specular color according to the Blinn-Phong model
    vec3 specular = MaterialSpecular * LightSourceSpecular *
                    pow(max(dot(N,H), 0.0), 100.0);

    // Calculate the final color
    gl_FragColor = vec4(ambient, 1.0) + vec4(specular, 1.0) +
                   vec4(diffuse, 1.0) * vec4(MaterialColor, 1.0);
}
