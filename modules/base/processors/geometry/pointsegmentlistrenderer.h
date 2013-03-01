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

#ifndef VRN_POINTSEGMENTLISTRENDERER_H
#define VRN_POINTSEGMENTLISTRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/properties/propertyvector.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/vectorproperty.h"

namespace voreen {

class Geometry;

/**
 * Renders a PointSegmentListGeometry<tgt::vec3> which is passed through a geometry inport.
 * The point list can be rendered as points, spheres or illuminated spheres.
 */
class VRN_CORE_API PointSegmentListRenderer : public GeometryRendererBase {
public:
    PointSegmentListRenderer();
    virtual ~PointSegmentListRenderer();
    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "PointSegmentListRenderer"; }
    virtual CodeState getCodeState() const { return Processor::CODE_STATE_STABLE; }
    virtual Processor* create() const { return new PointSegmentListRenderer(); }

protected:
    virtual void setDescriptions() {
        setDescription("Renders a list of segments, each consisting of points (PointSegmentListGeometryVec3), as point primitives, spheres or illuminated spheres. Each segment can be assigned a different color.\
<p>see GeometrySource</p>");
    }

    /**
     * Does not actually render, but retrieves the geometry
     * and stores it internally.
     */
    virtual void process();

    virtual void deinitialize() throw (VoreenException);

    /**
     * Actually renders the internally stored geometry. Uses a display list.
     */
    virtual void render();

    /**
     * Generates the display list for the passed point list according
     * to current property states. Is called by PointSegmentListRenderer::render
     *
     * @param m Transformation matrix, @see Geometry::transform()
     */
    virtual void generateDisplayList(const std::vector<std::vector<tgt::vec3> >& segmentList, const tgt::mat4 m);

    /**
     * Property callback for invalidating the display list on property changes.
     */
    void invalidateDisplayList();

    /**
     * Initializes the segment colors according to an internal color map.
     */
    void initializeColorMap();

    /// Geometry passed through the inport. Is stored by process method.
    Geometry* geometry_;
    /// display list generated from the geometry.
    GLuint displayList_;

    // properties
    StringOptionProperty coordinateSystem_;
    StringOptionProperty renderingPrimitiveProp_;
    BoolProperty applyUniformColor_;
    FloatVec4Property color_;
    PropertyVector* segmentColors_;
    BoolProperty depthTest_;
    FloatProperty pointSize_;
    BoolProperty pointSmooth_;
    FloatProperty sphereDiameter_;
    IntProperty sphereSlicesStacks_;

    GeometryPort geometryInport_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_POINTSEGMENTLISTRENDERER
