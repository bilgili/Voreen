/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef CAMERAINTERPOLATIONFUNCTIONS_H
#define CAMERAINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"
#include "tgt/camera.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<tgt::Camera>;
#endif

class VRN_CORE_API CameraLinearInterpolationFunction : public InterpolationFunction<tgt::Camera> {
public:
    CameraLinearInterpolationFunction();
    InterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraSphericalLinearInterpolationFunction : public InterpolationFunction<tgt::Camera> {
public:
    CameraSphericalLinearInterpolationFunction();
    InterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraCubicSplineInterpolationFunction: public MultiPointInterpolationFunction<tgt::Camera>{
public:
    CameraCubicSplineInterpolationFunction();
    MultiPointInterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(std::vector<PropertyKeyValue<tgt::Camera>*> controlpoints, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraStartInterpolationFunction : public InterpolationFunction<tgt::Camera> {
public:
    CameraStartInterpolationFunction();
    InterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraEndInterpolationFunction : public InterpolationFunction<tgt::Camera> {
public:
    CameraEndInterpolationFunction();
    InterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraStartEndInterpolationFunction : public InterpolationFunction<tgt::Camera> {
public:
    CameraStartEndInterpolationFunction();
    InterpolationFunction<tgt::Camera>* clone() const;
    tgt::Camera interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraCatmullRomInterpolationFunction : public MultiPointInterpolationFunction<tgt::Camera>{
public:
    CameraCatmullRomInterpolationFunction();
    MultiPointInterpolationFunction<tgt::Camera>* clone() const;

    tgt::Camera interpolate(std::vector<PropertyKeyValue<tgt::Camera>*> controlpoints, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

class VRN_CORE_API CameraSquadInterpolationFunction : public MultiPointInterpolationFunction<tgt::Camera>{
public:
    CameraSquadInterpolationFunction();
    MultiPointInterpolationFunction<tgt::Camera>* clone() const;

    tgt::Camera interpolate(std::vector<PropertyKeyValue<tgt::Camera>*> controlpoints, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

}

#endif // CAMERAINTERPOLATIONFUNCTIONS_H
