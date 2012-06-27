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

#include "voreen/modules/base/processors/utility/coordinatetransformation.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/geometry/pointlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointsegmentlistgeometry.h"
#include "voreen/core/properties/callmemberaction.h"

#include <vector>

using tgt::vec3;
using tgt::ivec3;
using tgt::ivec2;
using std::vector;

namespace voreen {

const std::string CoordinateTransformation::loggerCat_("voreen.CoordinateTransformation");

CoordinateTransformation::CoordinateTransformation()
  : Processor(),
    sourceCoordinateSystem_("sourceCoordinateSystem", "Source coordinate system"),
    targetCoordinateSystem_("targetCoordinateSystem", "Dest coordinate system"),
    geometryInport_(Port::INPORT, "geometry.inport"),
    volumeInport_(Port::INPORT, "volume.inport"),
    geometryOutport_(Port::OUTPORT, "geometry.outport"),
    forceUpdate_(true)
{

    std::vector<std::string> sourceCoordinates;
    sourceCoordinateSystem_.addOption("voxel-coordinates", "Voxel Coordinates");
    /*sourceCoordinateSystem_.push_back("volume-coordinates", "Volume Coordinates");
    sourceCoordinateSystem_.push_back("texture-coordinates", "Texture Coordinates");*/
    sourceCoordinateSystem_.onChange(CallMemberAction<CoordinateTransformation>(this, &CoordinateTransformation::forceUpdate));
    addProperty(sourceCoordinateSystem_);

    targetCoordinateSystem_.addOption("voxel-coordinates", "Voxel Coordinates");
    targetCoordinateSystem_.addOption("volume-coordinates", "Volume Coordinates");
    targetCoordinateSystem_.addOption("texture-coordinates", "Texture Coordinates");
    targetCoordinateSystem_.select("volume-coordinates");
    targetCoordinateSystem_.onChange(CallMemberAction<CoordinateTransformation>(this, &CoordinateTransformation::forceUpdate));
    addProperty(targetCoordinateSystem_);

    addPort(geometryInport_);
    addPort(volumeInport_);
    addPort(geometryOutport_);
}

CoordinateTransformation::~CoordinateTransformation() {
}

Processor* CoordinateTransformation::create() const {
    return new CoordinateTransformation();
}

std::string CoordinateTransformation::getProcessorInfo() const {
    return std::string("Transforms geometry coordinates between volume-dependent coordinate systems.<br>(see GeometrySource)");
}

void CoordinateTransformation::process() {

    // do nothing, if neither input data nor coordinate system selection has changed
    if (!geometryInport_.hasChanged() && !volumeInport_.hasChanged() && !forceUpdate_)
        return;

    // clear output
    delete geometryOutport_.getData();
    geometryOutport_.setData(0);
    forceUpdate_ = false;

    // return if no input data present
    if (!geometryInport_.hasData() || !volumeInport_.hasData())
        return;

    // retrieve and check geometry from inport
    PointListGeometryVec3* geometrySrc = dynamic_cast<PointListGeometryVec3*>(geometryInport_.getData());
    PointSegmentListGeometryVec3* geometrySegmentSrc = dynamic_cast<PointSegmentListGeometryVec3*>(geometryInport_.getData());
    if (!geometrySrc && !geometrySegmentSrc) {
        LWARNING("Geometry of type TGTvec3PointListGeometry or TGTvec3PointSegmentListGeometry expected");
        return;
    }

    // retrieve volume handle from inport
    VolumeHandle* volumeHandle = volumeInport_.getData();
    tgtAssert(volumeHandle, "No volume handle");
    tgtAssert(volumeHandle->getVolume(), "No volume");

    // create output geometry object
    PointListGeometryVec3* geometryConv = 0;
    PointSegmentListGeometryVec3* geometrySegmentConv = 0;
    if (geometrySrc)
        geometryConv = new PointListGeometryVec3();
    else if (geometrySegmentSrc)
        geometrySegmentConv = new PointSegmentListGeometryVec3();

    //
    // convert points
    //

    // voxel coordinates (no transformation necessary)
    if (targetCoordinateSystem_.get() == "voxel-coordinates") {
        // pointlist
        if (geometrySrc) {
            std::vector<tgt::vec3> geomPointsConv = std::vector<tgt::vec3>(geometrySrc->getData());
            geometryConv->setData(geomPointsConv);
        }
        // segmentlist
        else if (geometrySegmentSrc) {
            std::vector< std::vector<tgt::vec3> >geomPointsConv = std::vector< std::vector<tgt::vec3> >(geometrySegmentSrc->getData());
            geometrySegmentConv->setData(geomPointsConv);
        }
    }
    // volume coordinates
    else if (targetCoordinateSystem_.get() == "volume-coordinates") {
        tgt::vec3 dims(volumeHandle->getVolume()->getDimensions());
        tgt::vec3 cubeSize = volumeHandle->getVolume()->getCubeSize();
        // pointlist
        if (geometrySrc) {
            std::vector<tgt::vec3> geomPointsConv = std::vector<tgt::vec3>(geometrySrc->getData());
            for (size_t i=0; i<geomPointsConv.size(); i++)
                geomPointsConv[i] = ((geomPointsConv[i]/dims) - 0.5f) * cubeSize;
            geometryConv->setData(geomPointsConv);
        }
        // segmentlist
        else if (geometrySegmentSrc) {
            std::vector< std::vector<tgt::vec3> >geomPointsConv = std::vector< std::vector<tgt::vec3> >(geometrySegmentSrc->getData());
            for (size_t i=0; i<geomPointsConv.size(); ++i) {
                for (size_t j=0; j<geomPointsConv[i].size(); ++j) {
                    geomPointsConv[i][j] = ((geomPointsConv[i][j]/dims) - 0.5f) * cubeSize;
                }
                geometrySegmentConv->addSegment(geomPointsConv[i]);
            }
        }
    }
    // texture coordinates: [0:1.0]^3
    else if (targetCoordinateSystem_.get() == "texture-coordinates") {
        tgt::vec3 dims(volumeHandle->getVolume()->getDimensions());
        // pointlist
        if (geometrySrc) {
            std::vector<tgt::vec3> geomPointsConv = std::vector<tgt::vec3>(geometrySrc->getData());
            for (size_t i=0; i<geomPointsConv.size(); i++)
                geomPointsConv[i] /= dims;
            geometryConv->setData(geomPointsConv);
        }
        // segmentlist
        else if (geometrySegmentSrc) {
            std::vector< std::vector<tgt::vec3> >geomPointsConv = std::vector< std::vector<tgt::vec3> >(geometrySegmentSrc->getData());
            for (size_t i=0; i<geomPointsConv.size(); ++i) {
                for (size_t j=0; j<geomPointsConv[i].size(); ++j) {
                    geomPointsConv[i][j] /= dims;
                }
                geometrySegmentConv->addSegment(geomPointsConv[i]);
            }
        }
    }

    // assign result to outport
    if (geometrySrc)
        geometryOutport_.setData(geometryConv);
    else if (geometrySegmentSrc)
        geometryOutport_.setData(geometrySegmentConv);
    else {
        LWARNING("No geometry object created");
    }
}

void CoordinateTransformation::deinitialize() throw (VoreenException) {
    delete geometryOutport_.getData();
    geometryOutport_.setData(0);

    Processor::deinitialize();
}

void CoordinateTransformation::forceUpdate() {
    forceUpdate_ = true;
}


} // namespace voreen
