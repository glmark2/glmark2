attribute vec3 position;
attribute vec3 normal;
attribute vec2 texture;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec4 LightSourcePosition;
uniform vec3 LightSourceHalfVector;

varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{			
    // Transform the normal to eye coordinates
	Normal = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));

    // The LightSourcePosition is actually its direction for directional light
    Light = normalize(LightSourcePosition.xyz);

	// The HalfVector is used in the Blinn-Phong shading model for calculating
	// specular lighting.
	HalfVector = normalize(LightSourceHalfVector);

    // Transform the position to clip coordinates
	gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
