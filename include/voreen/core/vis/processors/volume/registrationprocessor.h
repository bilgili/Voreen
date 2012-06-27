/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_REGISTRATIONPROCESSOR_H
#define VRN_REGISTRATIONPROCESSOR_H

#include <string>
#include "voreen/core/vis/processors/volume/volumeprocessor.h"
#include "voreen/core/vis/properties/boolproperty.h"
#include "voreen/core/vis/properties/vectorproperty.h"
#include "tgt/vector.h"

namespace voreen {

class Volume;
class VolumeHandle;

class RegistrationProcessor : public VolumeProcessor {
public:
    RegistrationProcessor();
    virtual ~RegistrationProcessor();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "RegistrationProcessor"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new RegistrationProcessor(); }

    virtual void process();

private:
    void forceUpdate();
    void registerVolume();
    tgt::vec3 transformFromVoxelToWorldCoords(tgt::vec3 point, Volume* vol);

private:
    VolumeHandle* inputVolumeHandle_;       /** VolumeHandle from the inport */
    VolumeHandle* referenceInputVolumeHandle_;       /** VolumeHandle from the reference inport */
    VolumeHandle* outputVolumeHandle_;      /** VolumeHandle for the outport */
    VolumeHandle* processedVolumeHandle_;   /** VolumeHandle for the locally Volume */

    BoolProperty enableProcessingProp_;
    FloatVec3Property p1_;
    FloatVec3Property p2_;
    FloatVec3Property p3_;
    FloatVec3Property ref_p1_;
    FloatVec3Property ref_p2_;
    FloatVec3Property ref_p3_;
    bool forceUpdate_;

    static const std::string inportName_;
    static const std::string referenceInportName_;
    static const std::string outportName_;

    VolumePort inport_;
    VolumePort referenceInport_;
    VolumePort outport_;
};

}   //namespace

#endif
