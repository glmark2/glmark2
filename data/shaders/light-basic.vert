// For this tutorial, I am going show you how to perform per
// vertex lighting using shaders. Yes, shaders support per pixel
// but this is just a starting point.

// First I am going to teach you about varying variables. These
// are variables that can be shared between the vertex and fragment
// shaders. These variables are set inside the vertex shader, and 
// then read from the fragment shader. In this case we are going
// to share a float type variable from the Vertex shader to the 
// Fragment shader. This variable is going to be called Diffuse
// and will hold a number that will tell us how lit our vertex 
// should be.

// Because we are using per vertex lighting, the GLSL shader will
// automatically interpolate the color between the vertices. Just
// like OpenGL does.

// Now first off we need to know the surface normal of the current
// vertex. Because I am using a GLUT cube, these are already 
// calculated and sent off with the glNormal command. Any numbers
// sent from OpenGL through glNormal(1,2,3); can be read with
// the variable gl_Normal. In later tutorials, I will be using
// self calculated normals, using my own normal generation code.

// Now as for our normal, I am storing it in the variable vec3 Normal.
// But first, I have to multiply the surface normal (gl_Normal) by
// the gl_NormalMatrix. This places the normal in coordinates that
// we can use. (Later on I will show you how to use tangent space
// normals). We also then have to normalize this multiplication so
// that all of the normal vectors are between (and including) -1 and 1.
// This makes sure that we have no scaling errors.

// Next on, we are going to work with our light. I am storing this
// in a variable called vec3 Light. In this, I am calling for the
// position of glLight0. This is gathered by the call:
// gl_LightSource[0].position.xyz
// This gives us the position of glLight0, and to get the position of
// any other light, we just change the number 0 to the light number we
// are after. We do not need to multiply this expression by the 
// gl_NormalMatrix, but we do have to normalize it still.

// Now that we have our surface Normal and the position of our Light.
// We can now calculate the Diffuse value of our lighting at the 
// given vertex. To do this we need to take the dot product of both
// the Normal and the Light together. If you have any idea on
// calculating Normals, then you should have a fair idea of what the
// dot product does. It works as such:
// dot(a,b) = (x1 * x2) + (y1 * y2) + (z1 * z3)
// If this happens to be equal to 0, then the vectors a and b are 
// perpendicular (the light meets the surface at a 90 degree angle).
// Because this point is when the light is at its brightest, we need
// to set the maximum value for our diffuse value to 0.0.
// So our Diffuse equation becomes:
// Diffuse = max(dot(Normal, Light),0.0);

// And from that, we have now calculated the intensity of the light
// at the given vertex. That started to turn into a bit of a maths
// lesson, but it even cleared up some stuff for me without even
// thinking about it. GO MATHS :)

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texture;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec4 LightSourcePosition;
uniform vec3 LightSourceDiffuse;
uniform vec3 MaterialDiffuse;

varying float Diffuse;

void main(void)
{
    vec3 N = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));

    vec3 L = normalize(LightSourcePosition.xyz);

    Diffuse = max(dot(N, L), 0.0);

    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
}
