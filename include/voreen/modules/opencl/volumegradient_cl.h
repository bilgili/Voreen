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

#ifndef VRN_VOLUMEGRADIENT_CL_H
#define VRN_VOLUMEGRADIENT_CL_H

#include "voreen/modules/base/processors/volume/volumeprocessor.h"
#include "voreen/core/utils/clwrapper.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"

#include <string>

namespace voreen {

class VolumeHandle;

class VolumeGradientCL : public VolumeProcessor {
public:
    VolumeGradientCL();
    virtual ~VolumeGradientCL();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "VolumeGradientCL"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;

private:
    virtual Processor* create() const { return new VolumeGradientCL(); }
    virtual void process();
    virtual void initialize() throw(VoreenException);

    void clearCL();

    StringOptionProperty technique_;
    BoolProperty copyIntensityChannel_;  ///< if true, a four channel volume is created including the original intensity channel

    VolumePort inport_;
    VolumePort outport_;

    cl::OpenCL* opencl_;
    cl::Context* context_;
    cl::CommandQueue* queue_;
    cl::Program* prog_;

    static const std::string loggerCat_; ///< category used in logging
};

}   //namespace

#endif
