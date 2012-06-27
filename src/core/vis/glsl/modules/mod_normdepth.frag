uniform float minDepth_;
uniform float maxDepth_;

/***
 * Normalize the current input depth value based on minDepth_ and maxDepth_.
 *
 * @depth - depth value to be normalized
 ***/
float normDepth(float depth) {
	return (depth - minDepth_) * (1.0 / (maxDepth_ - minDepth_));
}
