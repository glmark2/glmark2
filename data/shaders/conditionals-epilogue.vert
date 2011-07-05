    vec4 pos = vec4(position.x,
                    position.y + 0.1 * d * fract(position.x),
                    position.z, 1.0);

    // Transform the position to clip coordinates
    gl_Position = ModelViewProjectionMatrix * pos;
}

