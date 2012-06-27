#include "mod_colorcoding.frag"

uniform sampler2D vectorField2D_;

float getFlowMagnitude(const vec2 r) {
    vec2 v = texture2D(vectorField2D_, r).xy;
    if (v != vec2(0.0)) {
        v = (v * (maxValue_ - minValue_)) + minValue_;
        return length(v);
    }
    return 0.0;
}

vec4 getColorFromFlowDirection(const vec2 r) {
    vec2 v = texture2D(vectorField2D_, r).xy;
    if (v != vec2(0.0))
        v = normalize((v * (maxValue_ - minValue_)) + minValue_);
    return vec4(abs(v.x), 0.0, abs(v.y), 1.0);
}

void main() {
    // look up the flow at the position and calculate the color
    //
    float magnitude = getFlowMagnitude(gl_TexCoord[0].st);

#if COLOR_MODE == COLOR_MODE_DIRECTION
    gl_FragColor = getColorFromFlowDirection(gl_TexCoord[0].st);
#elif COLOR_MODE == COLOR_MODE_MONOCHROME
    gl_FragColor = clamp(color_, vec4(0.0), vec4(1.0));
#else
    gl_FragColor = getColorFromFlowMagnitude(magnitude);
#endif
}
