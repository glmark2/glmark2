// First off, I am going to explain a little thing
// with shaders, that will help you to optimize them
// in the future. Vertex shaders are called for every
// vertex passed through, and Fragment shaders are
// called for every pixel on the screen. Because of this
// it is wise to do as many calculations in the vertex
// shader as possible, as they are called less, and if
// the calculation does not need to be called on a per
// pixel basis, why do it?

// Now as for the tutorial, you will see that I have
// removed the variable Diffuse from our vertex shader.
// This is because we are now going to calculate that in
// the fragment shader so we can get per pixel lighting.

// In our vertex shader, I have added the varying variables:
// varying vec3 Normal;
// varying vec3 Light;
// varying vec3 HalfVector;
// These hold the surface normal of our vertex, the direction
// vector of the light source, and the half vector of the
// light source (you may want to look into the maths behind
// lighting, I may add something in the maths section at a 
// later date).

// These variables are going to be calculated here, and passed
// through to our fragment shader. This is because these variables
// will not change, so there is no need calculating them on a
// per pixel basis.

// Now that we have these variables, we need to calculate them.
// Our Normal and Light variables stay exactly the same
// as in the previous tutorial. The only new one is the HalfVector
// which is called similar to our Light variable, just replace
// position with halfVector, and we get the light sources half
// vector supplied by OpenGL.

varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{			
	Normal = normalize(gl_NormalMatrix * gl_Normal);

	Light = normalize(gl_LightSource[0].position.xyz);

	HalfVector = normalize(gl_LightSource[0].halfVector.xyz);

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}