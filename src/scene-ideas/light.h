#ifndef LIGHT_H_
#define LIGHT_H_

#include "vec.h"

class LightSourceParameters
{
public:
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
    vec4 halfVector;
    vec3 spotDirection;
    float spotExponent;
    float spotCutoff;
    float spotCosCutoff;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

#endif // LIGHT_H_
