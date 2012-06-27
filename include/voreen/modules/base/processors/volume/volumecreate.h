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

#ifndef VRN_VOLUMECREATE_H
#define VRN_VOLUMECREATE_H

#include <string>
#include "voreen/core/processors/volumeprocessor.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

class VolumeHandle;

class VolumeCreate : public VolumeProcessor {
public:
    VolumeCreate();
    virtual ~VolumeCreate();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "VolumeCreate";      }
    virtual std::string getCategory() const   { return "Volume Processing"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;  }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void deinitialize() throw (VoreenException);

    void fillPlane(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 normal, uint8_t value);
    void fillCircle(VolumeUInt8* vds, tgt::vec3 center, float radius, uint8_t value);
    void fillOrientedCircle(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 normal, float radius, uint8_t value);
    void fillSphere(VolumeUInt8* vds, tgt::vec3 center, float radius, uint8_t value);
    void fillEllipsoid(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 radius, uint8_t value);
    void fillBox(VolumeUInt8* vds, tgt::ivec3 start, tgt::ivec3 end, uint8_t value);
    void fillOrientedBox(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 dir, float lengthA, float lengthB, float yStart, float yEnd, uint8_t value);
    void fillCone(VolumeUInt8* vds, tgt::vec3 center, tgt::vec3 dir, float length, float yStart, float yEnd, float topRadius, float bottomRadius, uint8_t value);

    void applyPerturbation(Volume* vds, tgt::ivec3 dimensions, tgt::vec3 frequency, tgt::vec3 amplitude);

private:
    VolumePort outport_;
    unsigned int currentSeed_;

    static const std::string loggerCat_; ///< category used in logging

    StringOptionProperty operation_;
    IntProperty dimension_;
    ButtonProperty regenerate_;
    IntProperty numShapes_;
    BoolProperty keepCurrentShapes_;
};

}   //namespace

#endif
