#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D Texture0;

varying vec2 TextureCoord;

void main(void)
{
    vec4 result;

#ifdef SCOTTY_WE_NEED_MORE_POWER
    const float Kernel0 = 0.2;
    result =
        texture2D(Texture0, TextureCoord + vec2(0.0, -2.0 * TextureStepY)) * Kernel0 +
        texture2D(Texture0, TextureCoord + vec2(0.0, -1.0 * TextureStepY)) * Kernel0 +
        texture2D(Texture0, TextureCoord + vec2(0.0, 0.0 * TextureStepY)) * Kernel0 +
        texture2D(Texture0, TextureCoord + vec2(0.0, 1.0 * TextureStepY)) * Kernel0 +
        texture2D(Texture0, TextureCoord + vec2(0.0, 2.0 * TextureStepY)) * Kernel0;
#else
    const float Kernel0 = 0.33333333;
    result =
        texture2D(Texture0, TextureCoord + vec2(0.0, -1.0 * TextureStepY)) * Kernel0  +
        texture2D(Texture0, TextureCoord + vec2(0.0, 0.0 * TextureStepY)) * Kernel0  +
        texture2D(Texture0, TextureCoord + vec2(0.0, 1.0 * TextureStepY)) * Kernel0;
#endif

    gl_FragColor = result;
}

