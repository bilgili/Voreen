/*
 * bsplineanimation.cpp
 *
 *  Created on: 11.07.2009
 *      Author: jan
 */

#include "bsplineanimation.h"

#include "tgt/spline.h"

#include <cmath>

namespace tgt {

using std::ceil;

BSplineAnimation::BSplineAnimation(vector<KeyFrame> keyFrames, int fps) :
    AbstractAnimation(fps, 0) {
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
        totalTime += ((*iter).timeToNextFrame_);
        if (firstTwice/*skip first*/&& !lastTwice/*skip last*/)
            knotValues.push_back(totalTime);
        if (!lastTwice && iter == keyFrames.end() - 1) {
            lastTwice = true;
            iter--;
        }
    }

    this->camPositionSpline_ = new BSpline(positionVector, knotValues);
    this->camFocusSpline_ = new BSpline(focusVector, knotValues);
    this->camUpSpline_ = new BSpline(upVector, knotValues);

    AbstractAnimation::fps_ = fps;
    AbstractAnimation::numFrames_ = ceil(float(fps) * totalTime);
}

BSplineAnimation::BSplineAnimation(vector<vec3> positionVector,
        vector<vec3> focusVector, vector<vec3> upVector,
        vector<float> knotValues, int fps, int totalFps) :
    AbstractAnimation(fps, totalFps) {
    this->camPositionSpline_ = new BSpline(positionVector, knotValues);
    this->camFocusSpline_ = new BSpline(focusVector, knotValues);
    this->camUpSpline_ = new BSpline(upVector, knotValues);
}

BSplineAnimation::~BSplineAnimation() {
    delete this->camPositionSpline_;
    delete this->camFocusSpline_;
    delete this->camUpSpline_;
}

void BSplineAnimation::moveToFrame(int frame) {
    this->curIndex_ = float(frame) / float(AbstractAnimation::numFrames_);
}

vec3 BSplineAnimation::getUp() {
    return this->camUpSpline_->getPoint(this->curIndex_);
}

vec3 BSplineAnimation::getCenter() {
    return this->camFocusSpline_->getPoint(this->curIndex_);
}

vec3 BSplineAnimation::getEye() {
    return this->camPositionSpline_->getPoint(this->curIndex_);
}

} //namespace tgt
