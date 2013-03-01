/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_GEOMETRYPROCESSOR_H
#define VRN_GEOMETRYPROCESSOR_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/interaction/idmanager.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/boolproperty.h"

namespace voreen {

class CameraInteractionHandler;

/**
 * Renders GeometryRenderer objects.
 *
 * This processor renders all to it connected coprocessors. The projection and modelview matrices
 * are set according to the current viewing position.
 */
class VRN_CORE_API GeometryProcessor : public RenderProcessor {
public:
    GeometryProcessor();
    ~GeometryProcessor();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "GeometryProcessor"; }
    virtual std::string getCategory() const { return "Geometry"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Manages the connected GeometryRenderer objects. Holds a vector of GeometryRenderer Objects and renders all of them on <i>render()</i>. The current projection and modelview matrices are propagated to the GeometryRenderer objects.");
    }

    virtual void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    virtual std::string generateHeader();

private:
    /// Requests the received output render size from the render inport.
    void passThroughSizeRequest();

    tgt::Shader* shaderPrg_;
    IDManager idManager_;

    BoolProperty renderGeometries_;
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
