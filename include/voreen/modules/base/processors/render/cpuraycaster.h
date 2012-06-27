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

#ifndef VRN_CPURAYCASTER_H
#define VRN_CPURAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/datastructures/transfunc/transfuncintensitygradient.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/gradient.h"

namespace voreen {

/**
 * Performs a simple raycasting on the CPU.
 */
class CPURaycaster : public VolumeRaycaster {
public:
    CPURaycaster();
    virtual ~CPURaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "CPURaycaster"; }
    virtual std::string getCategory() const   { return "Raycasting";   }
    virtual CodeState getCodeState() const    { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;

    /**
     * All ports except the gradient port need to be connected.
     */
    virtual bool isReady() const;

protected:
    virtual void process();

    unsigned long long sampling_count;
    unsigned long long sampling_last_same;

    VolumePort volumePort_;
    VolumePort gradientVolumePort_;
    RenderPort entryPort_;
    RenderPort exitPort_;
    RenderPort outport_;

    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function

    float textureLookup3D(VolumeUInt8* vol, const tgt::vec3& p);
    tgt::vec4 apply1DTF(float intensity);
    tgt::vec4 apply2DTF(float intensity, float gradientMagnitude);
    virtual tgt::vec4 directRendering(const tgt::vec3& first, const tgt::vec3& last);

    float* entry;
    float* exit;
    float* output;
    bool gradientVolume_;
    VolumeUInt8* vol8;
    VolumeUInt8* volumeGradientMagnitudes_;
    bool intensityGradientTF_;
    tgt::Texture* tf_tex;
};


} // namespace voreen

#endif // VRN_CPURAYCASTER_H
