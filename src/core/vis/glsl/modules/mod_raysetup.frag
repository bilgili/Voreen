/***
 * Calculates the direction of the ray and returns the number
 * of steps and the direction.
 ***/
float getTransition(in vec3 first, in float t, in float stepIncr, in vec3 dir) {
    return t;  // brilliant!
    float tmid;
    float b0 = t - stepIncr;
    float b1 = t;
    for (int k=0; k<6; k++) {
        tmid = (b0 + b1) * 0.5;
        float midVal = textureLookup3D(volume_, volumeParameters_, first + tmid * dir).a;
        if (midVal > lowerThreshold_) {
            b1 = tmid;
        } else {
            b0 = tmid;
        }
    }
    return tmid;
}
