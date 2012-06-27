/*
    pre-integration stuff
*/
#ifdef USE_PRE_INTEGRATION
#version 110

#line 7

uniform float sliceDistance_;
varying vec3 sb_;

#endif // USE_PRE_INTEGRATION

/*
    other vars
*/

varying vec3 v_;

void main() {
    gl_FrontColor = gl_Color;
    v_ = (gl_ModelViewMatrix * gl_Vertex).xyz;
    gl_TexCoord[0] = gl_TextureMatrix[VOL_TEX] * gl_MultiTexCoord0;

    /*
        compute sb_ for pre-integration
    */
#ifdef USE_PRE_INTEGRATION
    vec4 vPos = vec4(0, 0, 0, 1);
    vPos = gl_ModelViewMatrixInverse * vPos;
    vec4 vDir = vec4(0, 0, -1, 1);
    vDir = normalize(gl_ModelViewMatrixInverse * vDir);

    vec4 eyeToVert = normalize(gl_Vertex - vPos);
    vec4 sb = gl_Vertex - eyeToVert * (sliceDistance_ / dot(vDir, eyeToVert));
    sb_ = (gl_TextureMatrix[VOL_TEX] * sb).xyz;
#endif // USE_PRE_INTEGRATION

    /*
        set out vertex
    */
    gl_Position = ftransform();
}
