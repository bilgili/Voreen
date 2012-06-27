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

#include "voreen/core/properties/link/boxobjecthelper.h"

#include "voreen/core/properties/allproperties.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"

namespace voreen {

void BoxObjectHelper::setPropertyFromBoxObject(Property* p , const BoxObject& newValue) throw (VoreenException) {
    if (BoolProperty* destCast = dynamic_cast<BoolProperty*>(p))
        destCast->set(newValue.getBool());
    else if (FileDialogProperty* destCast = dynamic_cast<FileDialogProperty*>(p))
        destCast->set(newValue.getString());
    else if (FloatProperty* destCast = dynamic_cast<FloatProperty*>(p))
        destCast->set(newValue.getFloat());
    else if (IntProperty* destCast = dynamic_cast<IntProperty*>(p))
        destCast->set(newValue.getInt());
    else if (StringProperty* destCast = dynamic_cast<StringProperty*>(p))
        destCast->set(newValue.getString());
    else if (FloatVec2Property* destCast = dynamic_cast<FloatVec2Property*>(p))
        destCast->set(newValue.getVec2());
    else if (FloatVec3Property* destCast = dynamic_cast<FloatVec3Property*>(p))
        destCast->set(newValue.getVec3());
    else if (FloatVec4Property* destCast = dynamic_cast<FloatVec4Property*>(p))
        destCast->set(newValue.getVec4());
    else if (FloatMat2Property* destCast = dynamic_cast<FloatMat2Property*>(p))
        destCast->set(newValue.getMat2());
    else if (FloatMat3Property* destCast = dynamic_cast<FloatMat3Property*>(p))
        destCast->set(newValue.getMat3());
    else if (FloatMat4Property* destCast = dynamic_cast<FloatMat4Property*>(p))
        destCast->set(newValue.getMat4());
    else if (IntVec2Property* destCast = dynamic_cast<IntVec2Property*>(p))
        destCast->set(newValue.getIVec2());
    else if (IntVec3Property* destCast = dynamic_cast<IntVec3Property*>(p))
        destCast->set(newValue.getIVec3());
    else if (IntVec4Property* destCast = dynamic_cast<IntVec4Property*>(p))
        destCast->set(newValue.getIVec4());
    else if (CameraProperty* destCast = dynamic_cast<CameraProperty*>(p))
        destCast->set(*newValue.getCamera());
    else if (TransFuncProperty* destCast = dynamic_cast<TransFuncProperty*>(p)) {
        TransFunc* tf = destCast->get();
        TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
        if (tfi) {
            const TransFunc* newTF = newValue.getTransFunc();
            if(newTF) {
                const TransFuncIntensity* newTFI = dynamic_cast<const TransFuncIntensity*>(newTF);
                if (newTFI) {
                    tfi->updateFrom(*newTFI);
                    destCast->notifyChange();
                }
                else {
                    throw VoreenException("PropertyLink execution failed: TransFuncIntensity expected as return type");
                }
            }
            else {
                throw VoreenException("PropertyLink execution failed: No TF in BoxObject");
            }
        }
        else {
            throw VoreenException("PropertyLink execution failed: Transfer function linking currently only supported for TransFuncIntensity");
        }
    }
    else if (OptionPropertyBase* destCast = dynamic_cast<OptionPropertyBase*>(p))
        destCast->set(newValue.getString());
    else if (ShaderProperty* destCast = dynamic_cast<ShaderProperty*>(p)) {
        destCast->set(newValue.getShader());
        destCast->invalidate();
    }
    else if (ButtonProperty* destCast = dynamic_cast<ButtonProperty*>(p) )
        destCast->clicked();
    else if (VolumeHandleProperty* destCast = dynamic_cast<VolumeHandleProperty*>(p))
        destCast->set(const_cast<VolumeHandle*>(newValue.getVolumeHandle()));
    else if (dynamic_cast<VolumeCollectionProperty*>(p)) {
        throw VoreenException("PropertyLink execution failed: Linking of VolumeCollections not supported");
    }
    else if (PlotSelectionProperty* destCast = dynamic_cast<PlotSelectionProperty*>(p))
        destCast->set(newValue.getPlotSelectionEntryVec());
    else
        throw VoreenException("PropertyLink execution failed: Unsupported property type.");
}

BoxObject BoxObjectHelper::createBoxObjectFromProperty(Property* p) throw (VoreenException) {
    if (BoolProperty* srcCast = dynamic_cast<BoolProperty*>(p))
        return BoxObject(srcCast->get());
    else if (FileDialogProperty* srcCast = dynamic_cast<FileDialogProperty*>(p))
        return BoxObject(srcCast->get());
    else if (FloatProperty* srcCast = dynamic_cast<FloatProperty*>(p))
        return BoxObject(srcCast->get());
    else if (IntProperty* srcCast = dynamic_cast<IntProperty*>(p))
        return BoxObject(srcCast->get());
    else if (StringProperty* srcCast = dynamic_cast<StringProperty*>(p))
        return BoxObject(srcCast->get());
    else if (FloatVec2Property* srcCast = dynamic_cast<FloatVec2Property*>(p))
        return BoxObject(srcCast->get());
    else if (FloatVec3Property* srcCast = dynamic_cast<FloatVec3Property*>(p))
        return BoxObject(srcCast->get());
    else if (FloatVec4Property* srcCast = dynamic_cast<FloatVec4Property*>(p))
        return BoxObject(srcCast->get());
    else if (FloatMat2Property* srcCast = dynamic_cast<FloatMat2Property*>(p))
        return BoxObject(srcCast->get());
    else if (FloatMat3Property* srcCast = dynamic_cast<FloatMat3Property*>(p))
        return BoxObject(srcCast->get());
    else if (FloatMat4Property* srcCast = dynamic_cast<FloatMat4Property*>(p))
        return BoxObject(srcCast->get());
    else if (IntVec2Property* srcCast = dynamic_cast<IntVec2Property*>(p))
        return BoxObject(srcCast->get());
    else if (IntVec3Property* srcCast = dynamic_cast<IntVec3Property*>(p))
        return BoxObject(srcCast->get());
    else if (IntVec4Property* srcCast = dynamic_cast<IntVec4Property*>(p))
        return BoxObject(srcCast->get());
    else if (CameraProperty* srcCast = dynamic_cast<CameraProperty*>(p))
        return BoxObject(srcCast->get());
    else if (TransFuncProperty* srcCast = dynamic_cast<TransFuncProperty*>(p)) {
        TransFunc* tf = srcCast->get();
        if(tf) {
            TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(tf);
            if (tfi)
                return BoxObject(tfi);
            else
                throw VoreenException("PropertyLink execution failed: Transfer function linking currently only supported for TransFuncIntensity");
        }
        else
            throw VoreenException("PropertyLink execution failed: Transfer function is null");
    }
    else if (OptionPropertyBase* srcCast = dynamic_cast<OptionPropertyBase*>(p))
        return BoxObject(srcCast->get());
    else if (ShaderProperty* srcCast = dynamic_cast<ShaderProperty*>(p))
        return BoxObject(srcCast->get());
    else if (dynamic_cast<ButtonProperty*>(p))
        return BoxObject();
    else if (VolumeHandleProperty* srcCast = dynamic_cast<VolumeHandleProperty*>(p))
        return BoxObject(srcCast->get());
    else if (dynamic_cast<VolumeCollectionProperty*>(p))
        throw VoreenException("PropertyLink execution failed: Linking of VolumeCollections not supported");
    else if (PlotSelectionProperty* srcCast = dynamic_cast<PlotSelectionProperty*>(p))
        return BoxObject(srcCast->get());
    else
        throw VoreenException("PropertyLink execution failed: Unsupported property type.");

    return BoxObject(); //prevent warning
}

} // namespace
