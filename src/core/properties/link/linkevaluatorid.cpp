/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"

#include "voreen/core/properties/link/boxobjecthelper.h"

namespace voreen {

void LinkEvaluatorId::eval(Property* src, Property* dst) throw (VoreenException) {
    BoxObject bo = BoxObjectHelper::createBoxObjectFromProperty(src);
    BoxObjectHelper::setPropertyFromBoxObject(dst, bo);
}

std::string LinkEvaluatorId::name() const {
    return "id";
}

bool LinkEvaluatorId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    if (typeid(*p1) == typeid(*p2))
        return true;

    if (dynamic_cast<const BoolProperty*>(p1) || dynamic_cast<const FloatProperty*>(p1) ||
        dynamic_cast<const IntProperty*>(p1)  || dynamic_cast<const StringProperty*>(p1) )
    {
        if (dynamic_cast<const BoolProperty*>(p2))
            return true;
        else if (dynamic_cast<const IntProperty*>(p2))
            return true;
        else if (dynamic_cast<const FloatProperty*>(p2))
            return true;
        else if (dynamic_cast<const StringProperty*>(p2))
            return true;
        else
            return false;
    }
    else if (dynamic_cast<const IntVec2Property*>(p1)   || dynamic_cast<const IntVec3Property*>(p1)   || dynamic_cast<const IntVec4Property*>(p1)   ||
             dynamic_cast<const FloatVec2Property*>(p1) || dynamic_cast<const FloatVec3Property*>(p1) || dynamic_cast<const FloatVec4Property*>(p1) )
    {
        if (dynamic_cast<const IntVec2Property*>(p2))
            return true;
        else if (dynamic_cast<const IntVec3Property*>(p2))
            return true;
        else if (dynamic_cast<const IntVec4Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec2Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec3Property*>(p2))
            return true;
        else if (dynamic_cast<const FloatVec4Property*>(p2))
            return true;
        else
            return false;
    }

    return false;
}

//-----------------------------------------------------------------------------

void LinkEvaluatorCameraId::eval(Property* src, Property* dst) throw (VoreenException) {
    CameraProperty* dstCast = static_cast<CameraProperty*>(dst);
    CameraProperty* srcCast = static_cast<CameraProperty*>(src);

    tgt::Camera cam = dstCast->get();
    tgt::Camera srcCam = srcCast->get();

    cam.setPosition(srcCam.getPosition());
    cam.setFocus(srcCam.getFocus());
    cam.setUpVector(srcCam.getUpVector());

    dstCast->set(cam);
}

bool LinkEvaluatorCameraId::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const CameraProperty*>(p1) && dynamic_cast<const CameraProperty*>(p2));
}

//-----------------------------------------------------------------------------

void LinkEvaluatorTransFuncId::eval(Property* src, Property* dst) throw (VoreenException) {
    TransFuncProperty* dstCast = static_cast<TransFuncProperty*>(dst);
    TransFuncProperty* srcCast = static_cast<TransFuncProperty*>(src);

    if(!srcCast->get()) {
        LERRORC("voreen.LinkEvaluatorTransFuncId", "src is has no TF");
        return;
    }

    TransFunc* tf = srcCast->get()->clone();

    dstCast->set(tf);
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

} // namespace
