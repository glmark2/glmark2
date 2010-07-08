// Because we are using the varying float Diffuse in
// our vertex shader, if we wish to read it in here, we
// need to set the same variable, so I have done this 
// first.

// After that, I am simply changing the line:
// gl_FragColor = vec4(0,0,1,1) 
// to
// gl_FragColor = Diffuse * vec4(0,0,1,1);
// This multiplies the intensity of the light by the
// color we originally set the cube to.

// This can or cannot eliminate the need for materials,
// depending on your needs, but just for the record 
// (and future tutorials), materials are worth while
// and can be read from inside GLSL :)

varying float Diffuse;

void main(void)
{
    // Multiply the light Diffuse intensity by the color of the 
    gl_FragColor = Diffuse * vec4(0.0, 0.0, 1.0, 1.0);
}
