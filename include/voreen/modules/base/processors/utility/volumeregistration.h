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

#ifndef VRN_VOLUMEREGISTRATION_H
#define VRN_VOLUMEREGISTRATION_H

#include <string>
#include "voreen/core/processors/volumeprocessor.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "tgt/vector.h"

namespace voreen {

class Volume;
class VolumeHandle;

class VolumeRegistration : public VolumeProcessor {
public:
    VolumeRegistration();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "VolumeRegistration";    }
    virtual std::string getCategory() const  { return "Utility";               }
    virtual CodeState getCodeState() const   { return CODE_STATE_EXPERIMENTAL; }
    virtual bool isUtility() const           { return true; }
    virtual std::string getProcessorInfo() const;

    virtual bool isReady() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    tgt::mat4 computeRigidBodyTransformation(const VolumeHandle* inputHandle, const VolumeHandle* refHandle,
        tgt::vec3 p0, tgt::vec3 p0ref, tgt::vec3 p1, tgt::vec3 p1ref, tgt::vec3 p2, tgt::vec3 p2ref, bool ignoreReferenceTrafoMatrix);

    tgt::mat4 computeAffineTransformation(const VolumeHandle* inputHandle, const VolumeHandle* refHandle,
        std::vector<tgt::vec3> inputPoints, std::vector<tgt::vec3> refPoints, bool ignoreReferenceTrafoMatrix);

    tgt::vec3 transformFromVoxelToVolumeCoords(tgt::vec3 point, const Volume* vol);

    void updateGuiState();

    VolumePort inportVolume_;
    VolumePort inportReference_;
    GeometryPort inportLandmarks_;
    GeometryPort inportReferenceLandmarks_;
    VolumePort outport_;

    BoolProperty enableProcessing_;
    StringOptionProperty registrationMode_;
    StringOptionProperty landmarkSpecification_;
    BoolProperty ignoreReferenceTrafoMatrix_;
    FloatVec3Property p1_;
    FloatVec3Property ref_p1_;
    FloatVec3Property p2_;
    FloatVec3Property ref_p2_;
    FloatVec3Property p3_;
    FloatVec3Property ref_p3_;
    FloatVec3Property p4_;
    FloatVec3Property ref_p4_;
    FloatMat4Property registrationMatrix_;

    bool volumeOwner_;

    static const std::string loggerCat_;
};

} //namespace

#endif // VRN_VOLUMEREGISTRATION_H
