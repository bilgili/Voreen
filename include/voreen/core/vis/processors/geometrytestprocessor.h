/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_GEOMETRYTESTPROCESSOR
#define VRN_GEOMETRYTESTPROCESSOR

#include <typeinfo>
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/geometry/tgtvec3pointlistgeometry.h"
#include "voreen/core/geometry/geometrycontainer.h"
#include "voreen/core/opengl/texturecontainer.h"

using std::vector;

namespace voreen {

class GeometryTestProcessorA : public Processor
{
    public:
        GeometryTestProcessorA() : portName_("geometry.CubePointList")
        {
            // create an out port for geometry
            //
            createOutport(portName_);

            geo_ = new TGTvec3PointListGeometry();

            // create the geometry (a cube's vertices) and store it.
            //
            vector<tgt::vec3> vecTemp;
            vecTemp.push_back( tgt::vec3(-1.0f, -1.0f, 1.0f) );
            vecTemp.push_back( tgt::vec3(1.0f, -1.0f, 1.0f) );
            vecTemp.push_back( tgt::vec3(1.0f, -1.0f, -1.0f) );
            vecTemp.push_back( tgt::vec3(-1.0f, -1.0f, -1.0f) );
            vecTemp.push_back( tgt::vec3(-1.0f, 1.0f, 1.0f) );
            vecTemp.push_back( tgt::vec3(1.0f, 1.0f, 1.0f) );
            vecTemp.push_back( tgt::vec3(1.0f, 1.0f, -1.0f) );
            vecTemp.push_back( tgt::vec3(-1.0f, 1.0f, -1.0f) );
            geo_->setData(vecTemp);
        }

        virtual ~GeometryTestProcessorA()
        {
            // make sure to remove the geometry
            // from the container.
            // this is not necessary when clear() or
            // clearDeleting() are called on the
            // container, but this way is nicer: the
            // processor adds it - the processor removes it
            //
            if( geo_ != 0 )
            {
                geoContainer_->deleteGeometry(geo_->getID());
                delete geo_;
                geo_ = 0;
            }
        }

        virtual const Identifier getClassName() const { return "GeometryTestProcessor.GeometryTestProcessorA"; }
        virtual const std::string getProcessorInfo() const {return "Demonstrates how to use the geometry container.";}
        virtual Processor* create() const { return new GeometryTestProcessorA(); }
        virtual void process(voreen::LocalPortMapping* portMapping)
        {
            const int geoID = portMapping->getGeometryNumber(portName_);
            geoContainer_->addGeometry(geoID, geo_, "CubePoints");
        }

    protected:
        TGTvec3PointListGeometry* geo_;
        Identifier portName_;
};

class GeometryTestProcessorB : public Processor
{
    public:
        GeometryTestProcessorB() : inportName_("geometry.inport"), outportName_("image.output")
        {
            createInport(inportName_);
            createOutport(outportName_);
        }

        virtual ~GeometryTestProcessorB() { }
        virtual const Identifier getClassName() const { return "GeometryTestProcessor.GeometryTestProcessorB"; }
        virtual const std::string getProcessorInfo() const {return "Demonstrates how to use the geometry container.";}
        virtual Processor* create() const { return new GeometryTestProcessorB(); }

        virtual void process(voreen::LocalPortMapping* portMapping)
        {
            int dest = portMapping->getTarget(outportName_);
            const int geoID = portMapping->getGeometryNumber(inportName_);
            if( (dest <= -1) || (geoID <= 0) )
            {
                return;
            }

            Geometry* geo = geoContainer_->getGeometry(geoID);
            if( (geo != 0) && (typeid(*geo) == typeid(TGTvec3PointListGeometry)) )
            {
                tc_->setActiveTarget(dest);

                tgt::Color clearColor = backgroundColor_.get();
                glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_LIGHTING);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glPointSize(5.0f);

                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                tgt::multMatrix(camera_->getViewMatrix());
                TGTvec3PointListGeometry* geoPL = static_cast<TGTvec3PointListGeometry*>(geo);
                geoPL->draw();
                glMatrixMode(GL_MODELVIEW);
                glPopMatrix();
            }
        }

    protected:
        Identifier inportName_;
        Identifier outportName_;
};

} // namespace
#endif //VRN_GEOMETRYTESTPROCESSOR
