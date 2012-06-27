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

#ifndef VRN_POINTSEGMENTLISTRENDERER_H
#define VRN_POINTSEGMENTLISTRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/properties/propertyvector.h"

namespace voreen {

class Geometry;

/**
 * Renders a PointSegmentListGeometry<tgt::vec3> which is passed through a geometry inport.
 * The point list can be rendered as points, spheres or illuminated spheres.
 */
class PointSegmentListRenderer : public GeometryRendererBase {
public:

    /**
     *   Constructor
     */
    PointSegmentListRenderer();
    virtual ~PointSegmentListRenderer();
    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "PointSegmentListRenderer"; }
    virtual CodeState getCodeState() const { return Processor::CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const { return new PointSegmentListRenderer(); }

    /**
     * Does not actually render, but retrieves the geometry
     * and stores it internally.
     */
    virtual void process();


    /**
     * Actually renders the internally stored geometry. Uses a display list.
     */
    virtual void render();

protected:
    /**
     * Generates the display list for the passed point list according
     * to current property states. Is called by PointSegmentListRenderer::render
     */
    virtual void generateDisplayList(const std::vector<std::vector<tgt::vec3> >& segmentList);

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
    ColorProperty color_;
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
