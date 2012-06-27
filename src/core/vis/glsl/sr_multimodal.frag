uniform sampler3D volumeCT_;
uniform sampler3D volumePET_;
uniform sampler1D transferFuncCT_;
uniform sampler1D transferFuncPET_;

vec4 applyTFCT(float intensity) {
    #ifdef BITDEPTH_12
        return texture1D(transferFuncCT_, intensity * 16.0);
    #else
        return texture1D(transferFuncCT_, intensity);
    #endif
}

void main() {
    // fetch intensity
    vec4 intensityCT = texture3D(volumeCT_, gl_TexCoord[0].xyz);
    vec4 intensityPET = texture3D(volumePET_, gl_TexCoord[0].xyz);

    vec4 mat = applyTFCT(intensityCT.a);
    
    // FIXME: hack until transfer function is accessible
    if (intensityPET.a > 0.57)
		mat.r += intensityPET.a;
		
    gl_FragColor = mat;
}
