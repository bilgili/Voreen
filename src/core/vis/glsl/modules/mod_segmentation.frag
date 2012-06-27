
// id of the segment to be shown
uniform float segment_;

/***
 * returns 1 if the current sample belongs to the current segement
 * else 0
 */
float applySegmentation(vec3 sample) {
    float seg = textureLookup3D(segmentation_, segmentationParameters_, sample).a;
    return step(abs(seg - segment_/255.0), (1.0 / 255.0) / 2.0);
}
