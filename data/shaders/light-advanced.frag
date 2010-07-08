// Make sure you remember to add the varying variables:
// varying vec3 Normal;
// varying vec3 Light;
// varying vec3 HalfVector;
// To the fragment shader so that we can read them in.

// First off here, we have to make sure that our Normal
// has been normalized so to make sure, we are just going
// to normalize it with the line:
// Normal = normalize(Normal);

// Next off we want to calculate our diffuse portion of 
// the lighting. Now unlike the basic lighting tutorial,
// I am going to take into account the materials we have
// attached to the object, and the settings we have applied
// to our light.

// Now for our diffuse portion of our equation we get the line:
// float Diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * max(dot(Normal, Light),0.0);
// Now the variable gl_FrontMaterial.diffuse is the diffuse
// material we have assigned to the front face of our object,
// and to get the diffuse portion of our light, we call the 
// variable gl_LightSource[0].diffuse. Then to take into account
// how intense this should be, we multiply it by:
// max(dot(Normal, Light),0.0)
// Which from our previous tutorial, we used to calculate how
// intense the light should be, using the angle of the surface
// compared to the position of the light.

// Seeing as though we now have the diffuse part of our lighting
// we now need the Ambient term. In this term there are no real
// calculations like the dot product, used in the diffuse term.
// Here we are just taking values set inside our OpenGL program
// and applying them to our object. The variables that we need
// to use are:
// gl_FrontMaterial.ambient
// gl_LightSource[0].ambient
// gl_LightModel.ambient
// For our Ambient term to work, we need to multiply
// gl_FrontMaterial.ambient and gl_LightSource[0].ambient
// and then add the product of gl_LightModel.ambient and
// gl_FrontMaterial.ambient
// So our equation looks like:
// float Ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
// Ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

// In this tutorial, I am not going to cover the emissive term
// that OpenGL supports (I will in a later tutorial), so I am 
// going to jump straight to the most complicated term in the
// lighting equation. Which is the specular term. The reason
// that this term is the most complicated, is that it needs
// to find the reflection of the lighting from the surface of
// the object, to the 'camera'. The first part of the equation
// is similar to the start of the previous equations and takes
// the materials specular term, and the lights specular term
// and multiplies them together to look like:
// gl_FrontMaterial.specular * gl_LightSource[0].specular
// and then comes the tricky part, we need to multiply this by
// pow(max(dot(Normal,HalfVector),0.0), gl_FrontMaterial.shininess);
// This takes the dot product of our Normal and HalfVector, and as
// we have seen in the diffuse term, sets a max of 0.0
// With the number we have just calculated, we need to raise that
// to the power of our shininess term set inside our OpenGL program.

// Because we are using the halfVector at this stage, we don't need
// to use the reflect() expression, but we will when we get into
// normal mapping.

// So in the end our specular expression looks like:
// float Specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(max(dot(Normal,HalfVector),0.0), gl_FrontMaterial.shininess);

// Now for the combining of all the expressions. We still need to
// multiply the diffuse term by the objects color. And with that
// we then need to add the Ambient term and the Specular term.
// So our final gl_FragColor call looks like:
// gl_FragColor = Ambient + (Diffuse * vec4(1,0,0,1)) + Specular;

varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{	
	vec3 N = normalize(Normal);

	vec4 Diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * max(dot(N, Light),0.0);

	vec4 Ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	Ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

	vec4 Specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(max(dot(Normal,HalfVector),0.0), gl_FrontMaterial.shininess);

	gl_FragColor = Ambient + (Diffuse * vec4(0,0,1,1)) + Specular;
}
