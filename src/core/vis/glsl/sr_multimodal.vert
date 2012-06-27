varying vec3 v;

void main() {
    gl_FrontColor = gl_Color;
    v = (gl_ModelViewMatrix * gl_Vertex).xyz;
    gl_TexCoord[0] = gl_TextureMatrix[VOL_TEX] * gl_MultiTexCoord0;

    // set output vertex
    gl_Position = ftransform();
}
