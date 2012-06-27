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

#include "voreen/modules/base/processors/datasource/geometrysource.h"

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

const std::string GeometrySource::loggerCat_("voreen.GeometrySource");

GeometrySource::GeometrySource()
  : Processor(),
    geometryFile_("geometryFile", "Geometry file", "Open Geometry File", VoreenApplication::app()->getDataPath()),
    geometryType_("geometryType", "Geometry type"),
    skipItemCount_("skipItems", "Items to skip after each point", 0),
    outportPointList_(Port::OUTPORT, "geometry.pointlist")
{

    geometryType_.addOption("pointlist", "Pointlist");
    geometryType_.addOption("segmentlist", "Segmented Pointlist");

    geometryFile_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::readPointList));
    geometryType_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::readPointList));
    skipItemCount_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::readPointList));

    addProperty(geometryFile_);
    addProperty(geometryType_);
    addProperty(skipItemCount_);

    addPort(outportPointList_);
}

GeometrySource::~GeometrySource() {
}

Processor* GeometrySource::create() const {
    return new GeometrySource();
}

std::string GeometrySource::getProcessorInfo() const {
    return "Loads a point list or segmented point list from a text file. Each point is expected to "
           "consist of three components that are separated by white space. </p>"
           "<p>For a segmented point list, each point is expected to be followed by a numeric segment identifier."
           "The segments have to be listed in ascending order. </p>"
           "<p>Additionally, the number of items that are to be skipped after each point can be specified.";
}

void GeometrySource::process() {
    // nothing
}

void GeometrySource::initialize() throw (VoreenException) {
    Processor::initialize();
    if (!geometryFile_.get().empty()) {
        readPointList();
        if (!outportPointList_.hasData())
            geometryFile_.set("");
    }
}

void GeometrySource::deinitialize() throw (VoreenException) {
    Geometry* outGeom = outportPointList_.getData();
    outportPointList_.setData(0);
    delete outGeom;

    Processor::deinitialize();
}

void GeometrySource::readPointList() {

    if (!isInitialized())
        return;

    delete outportPointList_.getData();
    outportPointList_.setData(0);

    std::string filename = geometryFile_.get();
    if (filename.empty()) {
        return;
    }

    LINFO("Reading geometry file: " << filename);

    std::ifstream inFile;
    inFile.open(filename.c_str());
    if (inFile.fail()) {
        LERROR("Unable to open geometry file for reading: " << filename);
        return;
    }

    PointListGeometryVec3* pointListGeometry = 0;

    PointSegmentListGeometry<tgt::vec3>* pointSegmentListGeometry = 0;
    std::vector<std::vector<tgt::vec3> > segments;

    if (geometryType_.get() == "pointlist")
        pointListGeometry = new PointListGeometryVec3();
    else if (geometryType_.get() == "segmentlist")
        pointSegmentListGeometry = new PointSegmentListGeometry<tgt::vec3>();
    else {
        LWARNING("Unknown geometry type. Abort.");
        inFile.close();
        return;
    }

    int lastSegID = -1;
    while (true) {

        tgt::vec3 point;
        float dummy;
        int segID;

        if (!(inFile >> point.x))
            goto end;

        if (!(inFile >> point.y))
            goto end;

        if (!(inFile >> point.z))
            goto end;

        if (geometryType_.get() == "segmentlist") {
            if (!(inFile >> segID))
                goto end;
        }

        // skip items according to property
        for (int i=0; i<skipItemCount_.get(); ++i) {
            if (!(inFile >> dummy))
                goto end;
        }

        if (geometryType_.get() == "segmentlist") {
            // if new segment started, append vector for it
            if (segID > lastSegID) {
                segments.push_back(std::vector<tgt::vec3>());
                lastSegID = segID;
            }

            // append next skelPoint to last segment vector
            segments.back().push_back(point);
        }
        else if (geometryType_.get() == "pointlist") {
            pointListGeometry->addPoint(point);
        }
    }

end:

    if (geometryType_.get() == "segmentlist") {
        int numPoints = 0;
        for (size_t i=0; i<segments.size(); ++i)
            numPoints += segments[i].size();
        LINFO("Read " << segments.size() << " segments consisting of " << numPoints << " points.");
        pointSegmentListGeometry->setData(segments);
    }
    else if (geometryType_.get() == "pointlist") {
        LINFO("Read " << pointListGeometry->getNumPoints() << " points.");
    }

    if (pointListGeometry)
        outportPointList_.setData(pointListGeometry);
    else if (pointSegmentListGeometry)
        outportPointList_.setData(pointSegmentListGeometry);
    else {
        LERROR("No geometry object created");
    }

}

} // namespace voreen
