/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/interaction/voreentrackball.h"

namespace voreen {

void LinkEvaluatorCameraId::eval(Property* src, Property* dst) throw (VoreenException) {

    CameraProperty* dstCast = static_cast<CameraProperty*>(dst);
    CameraProperty* srcCast = static_cast<CameraProperty*>(src);

    tgt::Camera cam = dstCast->get();
    tgt::Camera srcCam = srcCast->get();

    cam.setPosition(srcCam.getPosition());
    cam.setFocus(srcCam.getFocus());
    cam.setUpVector(srcCam.getUpVector());
    cam.setFrustum(srcCam.getFrustum());
    cam.setProjectionMode(srcCam.getProjectionMode());
    cam.setStereoEyeMode(srcCam.getStereoEyeMode(), false);
    cam.setStereoEyeSeparation(srcCam.getStereoEyeSeparation(), false);
    cam.setStereoWidth(srcCam.getStereoWidth(), false);
    cam.setStereoFocalLength(srcCam.getStereoFocalLength(), false);
    cam.setStereoRelativeFocalLength(srcCam.getStereoRelativeFocalLength(), false);
    cam.setUseRealWorldFrustum(srcCam.getUseRealWorldFrustum(), false);
    cam.setStereoAxisMode(srcCam.getStereoAxisMode(), false);
    cam.enableOffset(srcCam.isOffsetEnabled());
    cam.setOffset(srcCam.getOffset());

    dstCast->setMinValue(srcCast->getMinValue());
    dstCast->setMaxValue(srcCast->getMaxValue());
    dstCast->setAdaptOnChange(srcCast->getAdaptOnChange());
    dstCast->setTrackballCenterBehaviour(srcCast->getTrackballCenterBehaviour());
    dstCast->getTrackball().setCenter(srcCast->getTrackball().getCenter());
    dstCast->setSceneBounds(srcCast->getSceneBounds());

    // order is important: only now set cam and cause further link execution
    dstCast->set(cam);
}

bool LinkEvaluatorCameraId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const CameraProperty*>(p2));
}

// -----------------------------------

void LinkEvaluatorCameraOrientationId::eval(Property* src, Property* dst) throw (VoreenException) {
    CameraProperty* dstCast = static_cast<CameraProperty*>(dst);
    CameraProperty* srcCast = static_cast<CameraProperty*>(src);

    tgt::Camera cam = dstCast->get();
    tgt::Camera srcCam = srcCast->get();

    cam.positionCamera(cam.getFocus() - cam.getFocalLength() * srcCam.getLook(), cam.getFocus(), srcCam.getUpVector());

    dstCast->set(cam);
}

bool LinkEvaluatorCameraOrientationId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const CameraProperty*>(p2));
}

//-----------------------------------------------------------------------------
//
void LinkEvaluatorCameraPosId::eval(Property* src, Property* dst) throw (VoreenException) {
    bool camToProp = true;
    CameraProperty* camProp = dynamic_cast<CameraProperty*>(src);
    FloatVec3Property* vecProp = dynamic_cast<FloatVec3Property*>(dst);
    if(!camProp) {
        camToProp = false;
        camProp = dynamic_cast<CameraProperty*>(dst);
        vecProp = dynamic_cast<FloatVec3Property*>(src);
    }

    tgt::Camera cam = camProp->get();
    if(camToProp)
        vecProp->set(cam.getPosition());
    else {
        cam.setPosition(vecProp->get());
        camProp->set(cam);
    }
}

bool LinkEvaluatorCameraPosId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");
    bool result = false;
    result |= (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const FloatVec3Property*>(p2));
    result |= (dynamic_cast<const CameraProperty*>(p2) && dynamic_cast<const FloatVec3Property*>(p1));
    return result;
}

//-----------------------------------------------------------------------------

void LinkEvaluatorCameraLookId::eval(Property* src, Property* dst) throw (VoreenException) {
    bool camToProp = true;
    CameraProperty* camProp = dynamic_cast<CameraProperty*>(src);
    FloatVec3Property* vecProp = dynamic_cast<FloatVec3Property*>(dst);
    if(!camProp) {
        camToProp = false;
        camProp = dynamic_cast<CameraProperty*>(dst);
        vecProp = dynamic_cast<FloatVec3Property*>(src);
    }

    tgt::Camera cam = camProp->get();
    if(camToProp)
        vecProp->set(cam.getLook());
    else {
        if(length(vecProp->get()) == 0.f) {
            LERRORC("voreen.LinkEvaluatorCameraLookId", "Can not use 0 vector to set look vector of camera");
            return;
        }
        cam.setFocus(cam.getPosition() + cam.getFocalLength() * normalize(vecProp->get()));
        camProp->set(cam);
    }
}

bool LinkEvaluatorCameraLookId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");
    bool result = false;
    result |= (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const FloatVec3Property*>(p2));
    result |= (dynamic_cast<const CameraProperty*>(p2) && dynamic_cast<const FloatVec3Property*>(p1));
    return result;
}

//-----------------------------------------------------------------------------
//
void LinkEvaluatorCameraFocusId::eval(Property* src, Property* dst) throw (VoreenException) {
    bool camToProp = true;
    CameraProperty* camProp = dynamic_cast<CameraProperty*>(src);
    FloatVec3Property* vecProp = dynamic_cast<FloatVec3Property*>(dst);
    if(!camProp) {
        camToProp = false;
        camProp = dynamic_cast<CameraProperty*>(dst);
        vecProp = dynamic_cast<FloatVec3Property*>(src);
    }

    tgt::Camera cam = camProp->get();
    if(camToProp)
        vecProp->set(cam.getFocus());
    else {
        cam.setFocus(vecProp->get());
        camProp->set(cam);
    }
}

bool LinkEvaluatorCameraFocusId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");
    bool result = false;
    result |= (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const FloatVec3Property*>(p2));
    result |= (dynamic_cast<const CameraProperty*>(p2) && dynamic_cast<const FloatVec3Property*>(p1));
    return result;
}

//-----------------------------------------------------------------------------

void LinkEvaluatorCameraFrustumId::eval(Property* src, Property* dst) throw (VoreenException) {
    CameraProperty* srcProp = dynamic_cast<CameraProperty*>(src);
    CameraProperty* dstProp = dynamic_cast<CameraProperty*>(dst);

    tgt::Camera cam = dstProp->get();
    cam.setFrustum(srcProp->get().getFrustum());
    dstProp->set(cam);
}

bool LinkEvaluatorCameraFrustumId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");
    return dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const CameraProperty*>(p2);
}

//-----------------------------------------------------------------------------

void LinkEvaluatorTransFuncId::eval(Property* src, Property* dst) throw (VoreenException) {
    TransFuncProperty* dstCast = static_cast<TransFuncProperty*>(dst);
    TransFuncProperty* srcCast = static_cast<TransFuncProperty*>(src);

    if(!srcCast->get()) {
        LERRORC("voreen.LinkEvaluatorTransFuncId", "src is has no TF");
        return;
    }

    TransFunc* srcTF = srcCast->get();
    TransFunc* destTF = dstCast->get();

    if (destTF && typeid(srcTF) == typeid(destTF)) { //< try to avoid the creation/deletion of tf objects
        destTF->updateFrom(srcTF);
        dstCast->invalidate();
    }
    else { //< use cloning as fallback
        dstCast->set(srcTF->clone());
    }

    dstCast->setDomainFittingStrategy(srcCast->getDomainFittingStrategy());
}

bool LinkEvaluatorTransFuncId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const TransFuncProperty*>(p1) && dynamic_cast<const TransFuncProperty*>(p2));
}

//-----------------------------------------------------------------------------

void LinkEvaluatorButtonId::eval(Property* /*src*/, Property* dst) throw (VoreenException) {
    static_cast<ButtonProperty*>(dst)->clicked();
}

bool LinkEvaluatorButtonId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const ButtonProperty*>(p1) && dynamic_cast<const ButtonProperty*>(p2));
}

//-----------------------------------------------------------------------------

void LinkEvaluatorLightSourceId::eval(Property* src, Property* dst) throw (VoreenException) {
    // "set" is overloaded in LightSourceProperty, but not virtual; we have to explicitly call this function
    static_cast<LightSourceProperty*>(dst)->set(static_cast<FloatVec4Property*>(src)->get());
}

bool LinkEvaluatorLightSourceId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const FloatVec4Property*>(p1) && dynamic_cast<const LightSourceProperty*>(p2));
}

} // namespace
