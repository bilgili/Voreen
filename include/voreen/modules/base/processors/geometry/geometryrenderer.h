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

#ifndef VRN_GEOMETRYRENDERER_H
#define VRN_GEOMETRYRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"

namespace voreen {

/**
 * Basic processor for rendering arbitrary geometry, simply taking
 * a Geometry object through its inport und calling render() on it.
 */
class GeometryRenderer : public GeometryRendererBase {
public:
    GeometryRenderer();
    ~GeometryRenderer();
    virtual Processor* create() const;

    virtual std::string getCategory() const   { return "Geometry";         }
    virtual std::string getClassName() const  { return "GeometryRenderer"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;  }
    virtual std::string getProcessorInfo() const;

    /**
     * Calls render() on the Geometry object.
     */
    virtual void render();

protected:
    GeometryPort inport_;
    GLEnumOptionProperty polygonMode_;
};

//---------------------------------------------------------------------------

/**
 * Renders a geometry with a texture applied to it.
 */
class TexturedGeometryRenderer : public GeometryRenderer {
public:
    TexturedGeometryRenderer();
    ~TexturedGeometryRenderer();

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "TexturedGeometryRenderer"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const {
        return "Renders a geometry with a texture.";
    }

    virtual Processor* create() const { return new TexturedGeometryRenderer(); }

    /**
     * Binds the texture on the inport and calls render() on the Geometry object.
     */
    virtual void render();

protected:
    RenderPort texPort_;
};

}

#endif // VRN_GEOMETRYRENDERER_H

