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

#ifndef VRN_SINGLEVOLUMESLICER_H
#define VRN_SINGLEVOLUMESLICER_H

#include "voreen/modules/base/processors/render/volumeslicer.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

namespace voreen {

/**
 * Slicing-based volume renderer using view plane aligned slices.
 */
class SingleVolumeSlicer : public VolumeSlicer {
public:
    SingleVolumeSlicer();
    virtual ~SingleVolumeSlicer();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "SingleVolumeSlicer"; }
    virtual std::string getCategory() const  { return "Slice Rendering"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

    virtual bool isReady() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    virtual void loadShader();

    virtual std::string generateHeader();
    virtual void compile();

private:

    tgt::Shader* slicingPrg_;

    VolumePort volumeInport_;
    RenderPort outport_;

    // interaction handlers
    CameraInteractionHandler cameraHandler_;
};


} // namespace voreen

#endif // VRN_SINGLEVOLUMESLICER_H
