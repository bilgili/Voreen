varying vec4 eep_;

/**
 * Use the entry exit positions interpolated by OpenGL as color value.
 */
void main() {
	/*
	if ((eep_.x < 0.0) || (eep_.x > 1.0)) discard;
    if ((eep_.y < 0.0) || (eep_.y > 1.0)) discard;
    if ((eep_.z < 0.0) || (eep_.z > 1.0)) discard;
    */
     //clamp(eep_, 0.0, 1.0);
    gl_FragColor = eep_;
}