#include "bsplineanimation.h"

#include "tgt/spline.h"

#include <cmath>

namespace tgt {

using std::ceil;

BSplineAnimation::BSplineAnimation(vector<KeyFrame> keyFrames, int fps)
    : AbstractAnimation(fps, 0)
{
    std::vector<vec3> positionVector;
    std::vector<vec3> focusVector;
    std::vector<vec3> upVector;
    std::vector<float> knotValues;
    // artificial starting knots
    knotValues.push_back(0.f);
    knotValues.push_back(0.f);
    float totalTime = 0.f;

    // we need first and last knot to be double points
    bool firstTwice = false;
    bool lastTwice = false;
    for (vector<KeyFrame>::iterator iter = keyFrames.begin(); iter
            != keyFrames.end(); iter++) {
        if (!firstTwice && iter == keyFrames.begin() + 1) {
            firstTwice = true;
            iter--;
        }
        positionVector.push_back((*iter).position_);
        focusVector.push_back((*iter).focus_);
        upVector.push_back((*iter).up_);
        totalTime += static_cast<float>((*iter).timeToNextFrame_);
        if (firstTwice/*skip first*/&& !lastTwice/*skip last*/)
            knotValues.push_back(totalTime);
        if (!lastTwice && iter == keyFrames.end() - 1) {
            lastTwice = true;
            iter--;
        }
    }

    camPositionSpline_ = new BSpline(positionVector, knotValues);
    camFocusSpline_ = new BSpline(focusVector, knotValues);
    camUpSpline_ = new BSpline(upVector, knotValues);

    fps_ = fps;
    numFrames_ = static_cast<int>(ceil(float(fps) * totalTime));
}

BSplineAnimation::BSplineAnimation(vector<vec3> positionVector,
        vector<vec3> focusVector, vector<vec3> upVector,
        vector<float> knotValues, int fps, int totalFps)
    : AbstractAnimation(fps, totalFps)
{
    camPositionSpline_ = new BSpline(positionVector, knotValues);
    camFocusSpline_ = new BSpline(focusVector, knotValues);
    camUpSpline_ = new BSpline(upVector, knotValues);
}

BSplineAnimation::~BSplineAnimation() {
    delete camPositionSpline_;
    delete camFocusSpline_;
    delete camUpSpline_;
}

void BSplineAnimation::moveToFrame(int frame) {
    curIndex_ = float(frame) / float(numFrames_);
}

vec3 BSplineAnimation::getUp() {
    return camUpSpline_->getPoint(curIndex_);
}

vec3 BSplineAnimation::getCenter() {
    return camFocusSpline_->getPoint(curIndex_);
}

vec3 BSplineAnimation::getEye() {
    return camPositionSpline_->getPoint(curIndex_);
}

} //namespace tgt
