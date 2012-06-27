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

#ifndef VRN_RGBARAYCASTER_H
#define VRN_RGBARAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

namespace voreen {

/**
 * Performs raycasting of an RGB volume by applying a hue-based transfer function.
 */
class RGBRaycaster : public VolumeRaycaster {
public:
    RGBRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "RGBRaycaster";     }
    virtual std::string getCategory() const  { return "Raycasting";       }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);

    virtual void loadShader();
    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);
    virtual void compile(VolumeHandle* volumeHandle);

private:
    VolumePort volumePort_;     ///< volume inport
    RenderPort entryPort_;      ///< entry points texture
    RenderPort exitPort_;       ///< exit points texture
    RenderPort outport_;        ///< output rendering

    TransFuncProperty transferFunc_;     ///< the property that controls the transfer-function
    BoolProperty applyColorModulation_;
    CameraProperty camera_;
};

} // namespace

#endif // VRN_RGBARAYCASTER_H
