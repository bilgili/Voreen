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

#ifndef VRN_INTENSITYMEASURE_H
#define VRN_INTENSITYMEASURE_H

#include "voreen/core/processors/imageprocessor.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/utils/stringconversion.h"
#include "tgt/font.h"
#include "tgt/glmath.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class IntensityMeasure : public ImageProcessor {
public:
    IntensityMeasure();
    ~IntensityMeasure();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Utility"; }
    virtual std::string getClassName() const { return "IntensityMeasure"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual bool isUtility() const           { return true; }

    virtual std::string getProcessorInfo() const;
    virtual bool isReady();

    void measure(tgt::MouseEvent* e);

protected:
    void process();

private:
    tgt::ivec2 cropToViewport(tgt::ivec2 mousePos);
    tgt::vec3 transformWorld2Voxel(tgt::vec3 worldPos, VolumePort volPort);

    RenderPort imgInport_;
    RenderPort fhpInport_;
    RenderPort outport_;
    VolumePort volumePort0_;
    VolumePort volumePort1_;
    VolumePort volumePort2_;
    VolumePort volumePort3_;

    EventProperty<IntensityMeasure>* mouseEventProp_;
    BoolProperty applyWorldToVoxel_; // the w2v matrix should be applied when dealing with multiple volume entry/exit points
    BoolProperty useVolume0_;
    BoolProperty useVolume1_;
    BoolProperty useVolume2_;
    BoolProperty useVolume3_;
    StringProperty unit0_;
    StringProperty unit1_;
    StringProperty unit2_;
    StringProperty unit3_;
    FloatProperty unitFactor0_;
    FloatProperty unitFactor1_;
    FloatProperty unitFactor2_;
    FloatProperty unitFactor3_;
    IntProperty numDigits0_;
    IntProperty numDigits1_;
    IntProperty numDigits2_;
    IntProperty numDigits3_;

    tgt::ivec2 mousePos2D_;
    tgt::Color mousePos3D_;
    bool mouseDown_;

    tgt::Font* font_;
};

} // namespace

#endif // VRN_INTENSITYMEASURE_H
