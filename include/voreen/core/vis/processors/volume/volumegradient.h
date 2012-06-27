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

#ifndef VRN_VOLUMEGRADIENT_H
#define VRN_VOLUMEGRADIENT_H

#include <string>
#include "voreen/core/vis/processors/volume/volumeprocessor.h"
#include "voreen/core/vis/properties/optionproperty.h"

namespace voreen {

class VolumeHandle;

class VolumeGradient : public VolumeProcessor {
public:
    VolumeGradient();
    virtual ~VolumeGradient();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "VolumeGradient"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new VolumeGradient(); }

    virtual void process();

private:
    VolumeHandle* processedVolumeHandle_;   /** VolumeHandle for the locally Volume */
    StringOptionProperty technique_;

    VolumePort inport_;
    VolumePort outport_;
};

}   //namespace

#endif
