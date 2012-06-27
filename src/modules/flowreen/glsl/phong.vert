varying vec3 normalVec;
varying vec3 lightVec;
varying vec3 viewVec;
varying float depth;

uniform vec3 camPos_;
uniform vec3 lightDir_;

void main()
{
    // The uniforms and gl_xxx were given in world coordinates, transform them
    // into eye space
    //
    normalVec = normalize(gl_NormalMatrix * gl_Normal);
    lightVec = normalize((gl_ModelViewMatrix * vec4(-lightDir_, 0.0)).xyz);
    viewVec = normalize((gl_ModelViewMatrix * (vec4(camPos_, 1.0) - gl_Vertex)).xyz);

    gl_FrontColor = gl_Color;
    gl_Position = ftransform();
    depth = gl_Position.z;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
