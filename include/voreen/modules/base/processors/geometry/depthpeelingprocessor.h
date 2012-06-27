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

#ifndef VRN_DEPTHPEELINGPROCESSOR_H
#define VRN_DEPTHPEELINGPROCESSOR_H

#include "voreen/core/processors/renderprocessor.h"

namespace voreen {

class GeometryRendererBase;
class CameraInteractionHandler;

/**
 * Implementation of the 'Order-Independent Transparency' approach by Cass Everitt.
 */
class DepthPeelingProcessor : public RenderProcessor {
public:
    DepthPeelingProcessor();
    ~DepthPeelingProcessor();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "DepthPeelingProcessor"; }
    virtual std::string getCategory() const  { return "Geometry"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual bool isReady() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    RenderPort inport_;
    RenderPort outport_;
    RenderPort tempPort_;
    GenericCoProcessorPort<GeometryRendererBase> cpPort_;

    tgt::Shader* shaderPrg_;

    CameraProperty camera_;
    CameraInteractionHandler* cameraHandler_;
};

} // namespace voreen

#endif // VRN_DEPTHPEELINGPROCESSOR_H
