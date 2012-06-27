/*
 * bsplineanimation.h
 *
 *  Created on: 11.07.2009
 *      Author: jan
 */

#ifndef BSPLINEANIMATION_H_
#define BSPLINEANIMATION_H_

#include "abstractanimation.h"

#include <vector>

using std::vector;

namespace tgt{

// forward declarations
class BSpline;

/**
 * as example we wrap the b-splines animation
 */
class BSplineAnimation : public AbstractAnimation {
public:

    BSplineAnimation(vector<KeyFrame> keyFrames, int fps);

    BSplineAnimation(vector<vec3> positionVector, vector<vec3> focusVector,
            vector<vec3> upVector, vector<float> knotValues, int fps,
            int totalFps);

    virtual ~BSplineAnimation();

    virtual void moveToFrame(int frame);
    virtual vec3 getUp();
    virtual vec3 getCenter();
    virtual vec3 getEye();

private:
    BSpline *camPositionSpline_;
    BSpline *camFocusSpline_;
    BSpline *camUpSpline_;
    float curIndex_;
};

} // namespace tgt


#endif /* BSPLINEANIMATION_H_ */
