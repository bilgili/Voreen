/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_CAMERA_H
#define TGT_CAMERA_H

#include <cmath>
#include <vector>

#include "tgt/config.h"
#include "tgt/frustum.h"
#include "tgt/types.h"
#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/quaternion.h"
#include "tgt/glcanvas.h"

/**
    This class implements a standard Camera with a position, a focus point
    and an up-vector which make up its orientation.
*/

namespace tgt {

class Camera {
public:
    /// Selects eye for stereo viewing
    enum Eye {
        EYE_LEFT,
        EYE_RIGHT,
        EYE_MIDDLE  /// no stereo
    };

protected:

    vec3 position_; /// location of the camera
    vec3 focus_;    /// location, the camera looks at
    vec3 upVector_; /// up vector, always normalized

    /// A frustum is saved in order to cull objects that are not lying within the
    /// view of the camera
    /// In stereo mode, it will always be the frustum of the eye previously used when calling look()
    Frustum frust_;

    float eyesep_; /// Eye separation for stereo viewing

    /// This is the actual matrix that holds the current orientation and position of the Camera.
    mutable mat4 viewMatrix_;
    mutable bool viewMatrixValid_; /// if the model-view matrix is up-to-date

    /// viewMatrix will not always be up to date according to position-, focus- and upVector.
    /// Make sure it is up to date.
    void updateVM() const {
        if (!viewMatrixValid_) {
            viewMatrix_ = mat4::createLookAt(getPosition(), getFocus(), getUpVector() );
        }
        viewMatrixValid_ = true;
    }

    /// Mark viewMatrix as outdated.
    void invalidateVM() const {
        viewMatrixValid_ = false;
    }

public:

    /**
     * Constructor.  A standard starting speed and orientation are given.
     *
     * @param position coordinates of the point the camera should be located at
     * @param focus coordinates of the point the camera should look to
     * @param up the up-vector
     * @param fovy the field of view angle, in degrees, in the y direction
     * @param ratio the aspect ratio, ratio of x (width) to y (height)
     * @param distn distance to nearplane
     * @param distf distance to farplane
     */
    Camera( const vec3& position =  vec3(0.0f, 0.0f,  0.0f),
            const vec3& focus    =  vec3(0.0f, 0.0f, -1.0f),
            const vec3& up       =  vec3(0.0f, 1.0f,  0.0f),
            float fovy           =  45.f,
            float ratio          =  static_cast<float>(GLCanvas::DEFAULT_WINDOW_WIDTH) / GLCanvas::DEFAULT_WINDOW_HEIGHT,
            float distn          =  .1f,
            float distf          =  50.f);

    virtual ~Camera();

public:
    /// Setter / Getter
    void setPosition(const vec3& pos) {
        position_ = pos;
        invalidateVM();
    }
    void setFocus(const vec3& foc) {
        focus_ =foc;
        invalidateVM();
    }
    void setUpVector(const vec3& up) {
        upVector_ = normalize(up);
        invalidateVM();
    }

    void setFrustum(const Frustum& frust) { frust_ = frust; }

    /// get Camera's strafe vector - a vector directing to the 'right'
    vec3 getStrafe()   const { return cross(getLook(), getUpVector()); }

    vec3 getUpVector() const { return upVector_; }
    vec3 getLook()     const { return normalize( getFocus()-getPosition() ); }
    vec3 getPosition() const { return position_; }
    vec3 getFocus()    const { return focus_; }

    Frustum& getFrustum() { return frust_; }
    const Frustum& getFrustum() const { return frust_; }

    float getFovy() const { return frust_.getFovy(); }
    float getRatio() const { return frust_.getRatio(); }
    float getNearDist() const { return frust_.getNearDist(); }
    float getFarDist() const { return frust_.getFarDist(); }
    float getFrustLeft() const { return frust_.getLeft(); }
    float getFrustRight() const { return frust_.getRight(); }
    float getFrustTop() const { return frust_.getTop(); }
    float getFrustBottom() const { return frust_.getBottom(); }

    /// set vertical viewing angle of camera
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    /// @param fovy angle in degree
    void setFovy(float fovy) {
        frust_.setFovy(fovy);
    }

    /// set aspect ratio
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setRatio(float ratio) {
        frust_.setRatio(ratio);
    }

    /// set distance from camera to nearplane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setNearDist(float neardist) {
        frust_.setNearDist(neardist);
    }

    /// set distance from camera to farplane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFarDist(float fardist) {
        frust_.setFarDist(fardist);
    }

    /// set coordiante of left clipping plane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustLeft(float v) {
        frust_.setLeft(v);
    }

    /// set coordiante of right clipping plane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustRight(float v) {
        frust_.setRight(v);
    }

    /// set coordiante of top clipping plane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustTop(float v) {
        frust_.setTop(v);
    }

    /// set coordiante of bottom clipping plane
    ///
    /// normals of frustum do not get updated by this method, call updateFrustum manually before
    /// using e.g. culling methods!
    void setFrustBottom(float v) {
        frust_.setBottom(v);
    }

    void setEyeSeparation(float s) {
        eyesep_ = s;
    }

    void setFocalLength(float f)   {
        setFocus( getPosition() + f*getLook() );
    }

    float getEyeSeparation() const {
        return eyesep_;
    }

    float getFocalLength() const   {
        return distance(getFocus(), getPosition());
    }

    quat getQuat() const {
        updateVM();
        return tgt::generateQuatFromMatrix(viewMatrix_.getRotationalPart());
    }

    /// Used to reposition the Camera.
    void positionCamera(const vec3& pos, const vec3& focus, const vec3& up) {
        setPosition(pos);
        setFocus(focus);
        setUpVector(up);
    }

    /// actually turns on the Camera.
    void look(Eye eye = EYE_MIDDLE);

    /// Update the frustum with the current camera parameters.
    /// This method MUST be called before a culling-method is used.  The reason this is not called
    /// in the culling-methods themselves is that we may have a lot of culling-querys without
    /// acutally changing the camera and thus, the frustum; in that case, frequently updating the
    /// frustum would be a waste of time.
    void updateFrustum();

    /// If you need the view matrix that would be used if look() was called, but without
    /// actually setting it in OpenGL, use this function.
    mat4 getViewMatrix() const;

    /// Sets a new view matrix; "reverse engineers" \a position_ and other parameters
    void setViewMatrix(mat4 mvMat);

    /// This returns the inverse of the current ViewMatrix.
    mat4 getViewMatrixInverse() const;

    /// If you need only the matrix that represents the current rotation of the camera-tripod, use this function.
    mat4 getRotateMatrix() const;

    /// This method returns the matrix that we need as the projection-matrix for off-axis-stereo-rendering
    mat4 getStereoPerspectiveMatrix(Eye eye, float fov, float aspect, float near, float far) const;

    /// This method returns the frustum matrix
    mat4 getFrustumMatrix() const;

    /// This method returns the projection matrix
    virtual mat4 getProjectionMatrix() const;
};

//------------------------------------------------------------------------------

/*
  This class implements a orthograpic camera. The only difference to an camera-object
  is in construction of projection matrix.
 */
class OrthographicCamera : public Camera {
public:
    OrthographicCamera(const vec3& position =  vec3(0.0f, 0.0f,  0.0f),
                       const vec3& focus    =  vec3(0.0f, 0.0f, -1.0f),
                       const vec3& up       =  vec3(0.0f, 1.0f,  0.0f),
                       float left           =  -1.f,
                       float right          =   1.f,
                       float bottom         =  -1.f,
                       float top            =   1.f,
                       float distn          =  -1.f,
                       float distf          =   1.f);

    /// This method returns the projection matrix
    virtual mat4 getProjectionMatrix() const;

};

} //namespace tgt

#endif // TGT_CAMERA_H
