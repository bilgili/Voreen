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

#ifndef VRN_GEOMETRYPROCESSOR_H
#define VRN_GEOMETRYPROCESSOR_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/interaction/idmanager.h"
#include "voreen/core/properties/cameraproperty.h"

namespace voreen {

class CameraInteractionHandler;

/**
 * Renders GeometryRenderer objects.
 *
 * This processor renders all to it connected coprocessors. The projection and modelview matrices
 * are set according to the current viewing position.
 */
class GeometryProcessor : public RenderProcessor {
public:
    GeometryProcessor();
    ~GeometryProcessor();

    virtual void initialize() throw (VoreenException);
    virtual bool isReady() const;

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "GeometryProcessor"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const;

protected:
    virtual void process();
    virtual std::string generateHeader();

private:
    tgt::Shader* shaderPrg_;
    IDManager idManager_;

    CameraProperty camera_;
    CameraInteractionHandler* cameraHandler_;

    RenderPort inport_;
    RenderPort outport_;
    RenderPort tempPort_;
    RenderPort pickingPort_;
    GenericCoProcessorPort<GeometryRendererBase> cpPort_;
};

} // namespace voreen

#endif // VRN_GEOMETRYPROCESSOR_H
