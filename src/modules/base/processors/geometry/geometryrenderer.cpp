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

#include "voreen/modules/base/processors/geometry/geometryrenderer.h"
#include "voreen/core/datastructures/geometry/geometry.h"

namespace voreen {

GeometryRenderer::GeometryRenderer()
    : GeometryRendererBase()
    , inport_(Port::INPORT, "inport.geometry")
    , polygonMode_("polygonMode", "Polygon Mode")
{
    addPort(inport_);

    polygonMode_.addOption("point", "Point", GL_POINT);
    polygonMode_.addOption("line",  "Line",  GL_LINE);
    polygonMode_.addOption("fill",  "Fill",  GL_FILL);
    polygonMode_.select("fill");
    addProperty(polygonMode_);
}

GeometryRenderer::~GeometryRenderer() {
}

Processor* GeometryRenderer::create() const {
    return new GeometryRenderer();
}

std::string GeometryRenderer::getProcessorInfo() const {
    return "Basic processor for rendering arbitrary geometry, simply taking "
        "a Geometry object through its inport und calling render() on it.";
}

void GeometryRenderer::render() {
    tgtAssert(inport_.hasData(), "No geometry");

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode_.getValue());
    inport_.getData()->render();
    glPopAttrib();
}

//---------------------------------------------------------------------------

TexturedGeometryRenderer::TexturedGeometryRenderer()
    : GeometryRenderer()
    , texPort_(Port::INPORT, "texture")
{
    addPort(texPort_);
}

TexturedGeometryRenderer::~TexturedGeometryRenderer() {
}

void TexturedGeometryRenderer::render() {
    tgtAssert(inport_.hasData(), "No geometry");

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode_.getValue());
    texPort_.bindColorTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    inport_.getData()->render();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

}

