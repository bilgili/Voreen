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

#ifndef VRN_VOLUMECURVATURE_H
#define VRN_VOLUMECURVATURE_H

#include <string>
#include "voreen/modules/base/processors/volume/volumeprocessor.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"

namespace voreen {

class VolumeHandle;

class VolumeCurvature : public VolumeProcessor {
public:
    VolumeCurvature();
    virtual ~VolumeCurvature();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "VolumeCurvature"; }
    virtual CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const { return new VolumeCurvature(); }

private:
    virtual void process();

    StringOptionProperty curvatureType_;

    VolumeHandle* processedVolumeHandle_;   /** VolumeHandle for the local Volume */

    VolumePort inport_;
    VolumePort outport_;
};

}   //namespace

#endif // VRN_VOLUMECURVATURE_H
