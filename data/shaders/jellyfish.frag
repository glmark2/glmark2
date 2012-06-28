#ifdef GL_ES
precision highp float;
#endif
  
uniform sampler2D uSampler;
uniform sampler2D uSampler1;

uniform float uShaderDebug;
uniform float uCurrentTime;
  
varying vec2 vTextureCoord;
varying vec3 vVertexNormal;
varying vec3 vWorldEyeVec;
varying vec4 vWorld;
varying vec4 vWorldView;
varying vec4 vWorldViewProj;
varying vec4 vWorldInvTranspose;
varying vec4 vViewInv;

varying float vDepth;
varying vec3 vDiffuse;
varying vec3 vSpecular;
varying vec3 vAmbient;
varying vec3 vFresnel;
varying vec3 vFog;

void main(void)
{
    vec3 caustics = texture2D(uSampler1, vec2(vWorld.x / 24.0 + uCurrentTime / 20.0, (vWorld.z - vWorld.y)/48.0 + uCurrentTime / 40.0)).rgb;
    vec4 colorMap = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));
    float transparency = colorMap.a + pow(vFresnel.r, 2.0) - 0.3;
    vec4 composit = vec4(((vAmbient + vDiffuse + caustics) * colorMap.rgb), transparency);

    gl_FragColor = composit;
}
