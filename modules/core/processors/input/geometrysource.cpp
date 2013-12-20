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

#include "geometrysource.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointsegmentlistgeometry.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometryindexed.h"

#include "voreen/core/properties/callmemberaction.h"
#include "tgt/filesystem.h"

#include <vector>
#include <fstream>

using tgt::vec3;
using tgt::ivec3;
using tgt::ivec2;
using std::vector;
using std::string;

namespace voreen {

const std::string GeometrySource::loggerCat_("voreen.core.GeometrySource");

GeometrySource::GeometrySource()
  : Processor(),
    geometryFile_("geometryFile", "Geometry File", "Open Geometry File", VoreenApplication::app()->getUserDataPath()),
    geometryType_("geometryType", "Geometry Type"),
    skipItemCount_("skipItems", "Items to skip after each point", 0),
    loadGeometry_("loadGeometry", "Load Geometry"),
    clearGeometry_("clearGeometry", "Clear Geometry"),
    useIndexedGeometry_("useIndexed", "Use indexed geometry (.ply)", false),
    calculateNormals_("calcNormals", "Calculate Normals (.ply)", false),
    outport_(Port::OUTPORT, "geometry.pointlist", "PointList Output")
{
    geometryType_.addOption("geometry", "Voreen Geometry (.vge)");
    geometryType_.addOption("pointlist", "Pointlist");
    geometryType_.addOption("segmentlist", "Segmented Pointlist");

    loadGeometry_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::readGeometry));
    clearGeometry_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::clearGeometry));
    geometryFile_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::updatePropertyVisibility));
    geometryType_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::updatePropertyVisibility));
    useIndexedGeometry_.onChange(CallMemberAction<GeometrySource>(this, &GeometrySource::readGeometry));

    addProperty(geometryFile_);
    addProperty(geometryType_);
    addProperty(skipItemCount_);
    addProperty(loadGeometry_);
    addProperty(clearGeometry_);

    addProperty(useIndexedGeometry_);
    addProperty(calculateNormals_);

    addPort(outport_);
}

Processor* GeometrySource::create() const {
    return new GeometrySource();
}

void GeometrySource::process() {}

void GeometrySource::initialize() throw (tgt::Exception) {
    Processor::initialize();
    if (geometryFile_.get() != "")
        readGeometry();
    updatePropertyVisibility();
}

void GeometrySource::readGeometry() {
    std::string filename = geometryFile_.get();

    if (geometryFile_.get() == "")
        return;

    if (geometryType_.isSelected("geometry")) {
        LINFO("Reading geometry file: " << geometryFile_.get());
        if(endsWith(filename, ".ply")) {
            LINFO("Reading PLY file.");
            try {
                Geometry* geometry = readPLYGeometry(filename);
                tgtAssert(geometry, "null pointer returned (exception expected)");
                outport_.setData(geometry);
            }
            catch (VoreenException& e) {
                LERROR(e.what());
            }
        }
        else {
            try {
                Geometry* geometry = readVoreenGeometry(geometryFile_.get());
                tgtAssert(geometry, "null pointer returned (exception expected)");
                outport_.setData(geometry);
            }
            catch (VoreenException& e) {
                LERROR(e.what());
            }
        }
    }
    else if (geometryType_.isSelected("pointlist") || geometryType_.isSelected("segmentlist")) {
        PointListType listType;
        if (geometryType_.isSelected("pointlist")) {
            LINFO("Reading point list from file: " << geometryFile_.get());
            listType = PointList;
        }
        else {
            LINFO("Reading segmented point list from file: " << geometryFile_.get());
            listType = SegmentedPointList;
        }

        try {
            Geometry* geometry = readPointList(geometryFile_.get(), listType, skipItemCount_.get());
            tgtAssert(geometry, "null pointer returned (exception expected)");
            outport_.setData(geometry);
        }
        catch (VoreenException& e) {
            LERROR(e.what());
        }
    }
    else {
        LWARNING("Unknown geometry type: " << geometryType_.get());
    }

    updatePropertyVisibility();
}

Geometry* GeometrySource::readPLYGeometry(const std::string& filename) const
    throw (VoreenException)
{
    // read PLY file (very incomplete reader!)
    tgt::File* file = FileSys.open(filename);
    std::string line = file->getLine();
    if(line != "ply")
        throw tgt::CorruptedFileException("Expected 'ply' at first", filename);

    // read header:
    line = trim(file->getLine());
    int numVertices = -1;
    int numFaces = -1;
    while(line != "end_header") {
        vector<std::string> expl = strSplit(line, ' ');
        if(expl.size() > 0) {
            string command = expl[0];

            if(command == "format") {
                if(expl.size() != 3)
                    throw tgt::CorruptedFileException("Could not parse format", filename);
                else {
                    if((expl[1] != "ascii") || (expl[2] != "1.0"))
                        throw tgt::CorruptedFileException("Unknown format: " + line, filename);
                }
            }
            else if(command == "comment") {
                LINFO(line);
            }
            else if(command == "element") {
                if(expl.size() != 3)
                    throw tgt::CorruptedFileException("Could not parse element" + line, filename);
                else {
                    string type = expl[1];
                    if(type == "vertex") {
                        numVertices = stoi(expl[2]);
                        LINFO(numVertices << " vertices");
                    }
                    else if(type == "face") {
                        numFaces = stoi(expl[2]);
                        LINFO(numFaces << " faces");
                    }
                }
            }
            else if(command == "property") {
            }
        }

        line = trim(file->getLine());
    }

    // read data:
    vector<VertexVec3> vertices;
    for(int i=0; i<numVertices; i++) {
        line = trim(file->getLine());
        vector<std::string> expl = strSplit(line, ' ');

        if(expl.size() >= 3) {
            vec3 pos;
            pos.x = stof(expl[0]);
            pos.y = stof(expl[1]);
            pos.z = stof(expl[2]);
            vertices.push_back(VertexVec3(pos, vec3(1.0f)));
            //vertices.push_back(VertexVec3(pos, vec3(stof(expl[4]))));
        }
    }

    // let the user choose between clippable trianglemeshgeom and non-clippable (at least, for now, on the CPU), but rendering-wise more efficient indexed geometry
    Geometry* mesh;
    if(useIndexedGeometry_.get())
        mesh = new TriangleMeshGeometryUInt32IndexedVec3();
    else
        mesh = new TriangleMeshGeometryVec3();

    std::vector<tgt::ivec3> faces;
    std::vector<tgt::vec3> vertexNormals;
    if(calculateNormals_.get())
        vertexNormals.resize(numVertices, tgt::vec3(0.f));

    for(int i=0; i<numFaces; i++) {
        line = trim(file->getLine());
        vector<std::string> expl = strSplit(line, ' ');

        if(expl.size() == 4) {
            if(expl[0] == "3") {
                int v1 = stoi(expl[1]);
                int v2 = stoi(expl[2]);
                int v3 = stoi(expl[3]);

                faces.push_back(tgt::ivec3(v1, v2, v3));

                if(calculateNormals_.get()) {
                    tgt::vec3 norm = normalize(cross(vertices[v2].pos_ - vertices[v1].pos_, vertices[v3].pos_ - vertices[v1].pos_));
                    vertexNormals[v1] += norm;
                    vertexNormals[v2] += norm;
                    vertexNormals[v3] += norm;
                }
            }
        }
    }

    if(calculateNormals_.get()) {
        for(int i = 0; i < numVertices; i++)
            vertices[i].attr1_ = normalize(vertexNormals.at(i));
    }

    if(useIndexedGeometry_.get())
        (static_cast<TriangleMeshGeometryUInt32IndexedVec3*>(mesh))->setVertices(vertices);

    for(int i=0; i<numFaces; i++) {
        if(useIndexedGeometry_.get())
            (static_cast<TriangleMeshGeometryUInt32IndexedVec3*>(mesh))->addTriangle(faces.at(i));
        else
            (static_cast<TriangleMeshGeometryVec3*>(mesh))->addTriangle(Triangle<VertexVec3>(vertices[faces.at(i).x], vertices[faces.at(i).y], vertices[faces.at(i).z]));
    }

    LINFO("Read " << vertices.size() << " vertices and " << numFaces << " triangles " << mesh->getBoundingBox());

    file->close();
    delete file;
    return mesh;
}

Geometry* GeometrySource::readVoreenGeometry(const std::string& filename) const
    throw (VoreenException)
{
    // read Voreen geometry serialization (.vge)
    std::ifstream stream;
    stream.open(filename.c_str(), std::ios_base::in);
    if (stream.fail())
        throw VoreenException("Failed to open file " + geometryFile_.get() + " for reading");

    XmlDeserializer deserializer;
    try {
        deserializer.read(stream);
        Geometry* geometry = 0;
        deserializer.deserialize("Geometry", geometry);
        return geometry;
    }
    catch (SerializationException &e) {
        throw VoreenException("Failed to deserialize Voreen Geometry from " + geometryFile_.get() + ": " + e.what());
    }
}

Geometry* GeometrySource::readPointList(const std::string& filename, PointListType listType, int skipItems) const
    throw (VoreenException)
{
    tgtAssert(skipItems >= 0, "skipItems must be non-negative");

    std::ifstream inFile;
    inFile.open(filename.c_str());
    if (inFile.fail())
        throw VoreenException("Failed to open file for reading: " + filename);

    PointListGeometryVec3* pointListGeometry = 0;

    PointSegmentListGeometry<tgt::vec3>* pointSegmentListGeometry = 0;
    std::vector<std::vector<tgt::vec3> > segments;

    if (listType == PointList)
        pointListGeometry = new PointListGeometryVec3();
    else if (listType == SegmentedPointList)
        pointSegmentListGeometry = new PointSegmentListGeometryVec3();
    else {
        tgtAssert(false, "unknown point list type");
        throw VoreenException("Unknown point list type: " + genericToString(listType));
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

        if (listType == SegmentedPointList) {
            if (!(inFile >> segID))
                goto end;
        }

        // skip items according to property
        for (int i=0; i<skipItems; ++i) {
            if (!(inFile >> dummy))
                goto end;
        }

        if (listType == SegmentedPointList) {
            // if new segment started, append vector for it
            if (segID > lastSegID) {
                segments.push_back(std::vector<tgt::vec3>());
                lastSegID = segID;
            }

            // append next skelPoint to last segment vector
            segments.back().push_back(point);
        }
        else if (listType == PointList) {
            tgtAssert(pointListGeometry, "no pointListGeometry");
            pointListGeometry->addPoint(point);
        }
    }

end:

    if (listType == SegmentedPointList) {
        size_t numPoints = 0;
        for (size_t i=0; i<segments.size(); ++i)
            numPoints += segments[i].size();
        LINFO("Read " << segments.size() << " segments consisting of " << numPoints << " points.");
        tgtAssert(pointSegmentListGeometry, "no pointSegmentListGeometry");
        pointSegmentListGeometry->setData(segments);
    }
    else if (listType == PointList) {
        tgtAssert(pointListGeometry, "no pointListGeometry");
        LINFO("Read " << pointListGeometry->getNumPoints() << " points.");
    }

    if (pointListGeometry)
        return pointListGeometry;
    else if (pointSegmentListGeometry)
        return pointSegmentListGeometry;
    else {
        tgtAssert(false, "no geometry created");
        return 0;
    }
}

void GeometrySource::clearGeometry() {
    outport_.setData(0);
    geometryFile_.set("");
    updatePropertyVisibility();
}

void GeometrySource::updatePropertyVisibility() {
    loadGeometry_.setWidgetsEnabled(geometryFile_.get() != "");
    clearGeometry_.setWidgetsEnabled(outport_.getData());
    skipItemCount_.setVisible(!geometryType_.isSelected("geometry"));

    useIndexedGeometry_.setVisible(endsWith(geometryFile_.get(), ".ply"));
    calculateNormals_.setVisible(endsWith(geometryFile_.get(), ".ply"));
}

} // namespace voreen
