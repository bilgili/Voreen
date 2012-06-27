#define MOD_TF_SIMPLE 1
#include "modules/mod_transfunc.frag"
#line 3

/*
    uniforms
*/

uniform sampler3D volumeDataset_;
uniform sampler2D lookupTable_;

/*
    conditional uniforms and varyings
*/

#if defined(USE_LOWER_THRESHOLD) || defined(USE_UPPER_THRESHOLD)
    uniform  vec2 threshold_;
#endif // defined(USE_LOWER_THRESHOLD) || defined(USE_UPPER_THRESHOLD)

#ifdef USE_PHONG_LIGHTING
    varying vec3 v_;

#ifdef USE_CALC_GRADIENTS
    uniform vec3 datasetDimensions_;
#endif // USE_CALC_GRADIENTS
#endif // USE_PHONG_LIGHTING

#ifdef USE_SAMPLING_RATE
    uniform float samplingRate_;
#endif // USE_SAMPLING_RATE

#ifdef USE_PRE_INTEGRATION
    varying vec3 sb_;
#endif // USE_PRE_INTEGRATION

/*
    some additional defines
*/
#define LOWER_THRESHOLD threshold_.x
#define UPPER_THRESHOLD threshold_.y

/*
    let's go
*/

void main() {
    // fetch intensity
    vec4 intensity = texture3D(volumeDataset_, gl_TexCoord[0].xyz);

    /*
        threshold checks
    */
#ifdef USE_LOWER_THRESHOLD
    if (intensity.a < LOWER_THRESHOLD)
        discard;
#endif // USE_LOWER_THRESHOLD

#ifdef USE_UPPER_THRESHOLD
    if (intensity.a >= UPPER_THRESHOLD)
        discard;
#endif // USE_UPPER_THRESHOLD

    /*
        calculate normal
    */
#ifdef USE_PHONG_LIGHTING
    vec3 n;
#endif // USE_PHONG_LIGHTING

#ifdef USE_PHONG_LIGHTING
#ifdef USE_CALC_GRADIENTS
    /*
        calculate gradient on the fly
    */
    vec3 offset = vec3(1.0, 1.0, 1.0);
    offset /= datasetDimensions_;

    // six lookups for the volume
    vec3 sample1, sample2;
    sample2.x = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(-offset.x, 0.0, 0.0)).a;
    sample2.y = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, -offset.y, 0.0)).a;
    sample2.z = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, 0.0, -offset.z)).a;
    sample1.x = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3( offset.x, 0.0, 0.0)).a;
    sample1.y = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0,  offset.y, 0.0)).a;
    sample1.z = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, 0.0,  offset.z)).a;

    // six lookups for the transfer function
    vec3 alpha1, alpha2;
    alpha1.x = applyTF(sample1.x).a;
    alpha1.y = applyTF(sample1.y).a;
    alpha1.z = applyTF(sample1.z).a;
    alpha2.x = applyTF(sample2.x).a;
    alpha2.y = applyTF(sample2.y).a;
    alpha2.z = applyTF(sample2.z).a;

    n = normalize(alpha2 - alpha1);
    intensity.xyz = n;
    n = normalize(gl_NormalMatrix * normalize(n));
#else // USE_CALC_GRADIENTS
    n = normalize(gl_NormalMatrix * normalize(intensity.xyz));
    intensity.xyz -= 0.5;   //is this done by the matrix above? the un-normalized normal is needed for the 2d TF
#endif // USE_CALC_GRADIENTS
#endif // USE_PHONG_LIGHTING

    /*
        get value via the transfer functions or the pre-integration lookup table
    */
#ifdef USE_PRE_INTEGRATION
    vec2 lookup;
    lookup.x = intensity.a;
    lookup.y = texture3D(volumeDataset_, sb_).a;
    vec4 mat = texture2D(lookupTable_, lookup);
#else // USE_PRE_INTEGRATION
    vec4 mat = applyTF(intensity.a);
#endif // USE_PRE_INTEGRATION

    /*
        keep sampling rate in mind
    */
#ifdef USE_SAMPLING_RATE
    mat.a *= samplingRate_;
#endif // USE_SAMPLING_RATE

    /*
        calculate phong lighting
    */
#ifdef USE_PHONG_LIGHTING
    vec3 l = normalize(gl_LightSource[0].position.xyz - v_);
    vec3 r = 2.0 * dot(n, l) * (n - l);
//     vec3 h = normalize(l + v);


    vec4 ambi = gl_LightSource[0].ambient  * mat * 0.1;
    vec4 diff = gl_LightSource[0].diffuse  * mat * max( dot(n, l), 0.0 );
    vec4 spec = gl_LightSource[0].specular * mat * pow( max(dot(r, v_), 0.0), 2.0);

    float d = length(v_);
    float factor = 1.0 /
        ( gl_LightSource[0].constantAttenuation
        + gl_LightSource[0].linearAttenuation * d
        + gl_LightSource[0].quadraticAttenuation * d*d);

    vec4 fragColor = min( vec4(1.0, 1.0, 1.0, 1.0), /*gl_Material*/ ambi + factor * (spec + diff) ) * gl_Color;
    fragColor.a = mat.a * gl_Color.a;
#else // USE_PHONG_LIGHTING
    vec4 fragColor = mat; //* gl_Color;
#endif // USE_PHONG_LIGHTING

    /*
        set output fragment
    */
    gl_FragColor = fragColor;
}
