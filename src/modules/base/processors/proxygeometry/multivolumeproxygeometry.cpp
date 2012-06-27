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

#include "voreen/modules/base/processors/proxygeometry/multivolumeproxygeometry.h"

namespace voreen {

MultiVolumeProxyGeometry::MultiVolumeProxyGeometry()
    : Processor()
    , geometry_(0)
    , inport_(Port::INPORT, "volumehandle.volumehandle", true)
    , outport_(Port::OUTPORT, "proxygeometry.geometry")
{
    addPort(inport_);
    addPort(outport_);
}

MultiVolumeProxyGeometry::~MultiVolumeProxyGeometry() {
}

Processor* MultiVolumeProxyGeometry::create() const {
    return new MultiVolumeProxyGeometry();
}

std::string MultiVolumeProxyGeometry::getProcessorInfo() const {
    return "Provides a cube mesh proxy geometry for multi-volume raycasting. "
           "See MeshEntryExitPoints, MultiVolumeRaycaster.";
}

void MultiVolumeProxyGeometry::process() {
    tgtAssert(inport_.getData()->getVolume(), "no volume");
    tgtAssert(geometry_, "no geometry object");

    geometry_->clear();
    std::vector<VolumeHandle*> data = inport_.getAllData();
    for(size_t d=0; d<data.size(); ++d) {
        if(!data[d])
            continue;

        Volume* volume = data[d]->getVolume();
        tgt::vec3 volumeSize = volume->getCubeSize();

        tgt::vec3 coordLlf = -(volumeSize / static_cast<tgt::vec3::ElemType>(2));
        tgt::vec3 coordUrb =  (volumeSize / static_cast<tgt::vec3::ElemType>(2));

        MeshGeometry mesh = MeshGeometry::createCube(coordLlf, coordUrb, coordLlf, coordUrb);
        //apply dataset transformation matrix:
        mesh.transform(volume->getTransformation());

        //reset tex coords to coords after transformation:
        for(size_t j=0; j<mesh.getFaceCount(); ++j) {
            FaceGeometry& fg = mesh.getFace(j);
            for(size_t k=0; k<fg.getVertexCount(); ++k) {
                VertexGeometry& vg = fg.getVertex(k);
                vg.setTexCoords(vg.getCoords());
            }
        }
        geometry_->addMesh(mesh);
    }

    outport_.setData(geometry_);
}

void MultiVolumeProxyGeometry::initialize() throw (VoreenException) {
    Processor::initialize();
    geometry_ = new MeshListGeometry();
}

void MultiVolumeProxyGeometry::deinitialize() throw (VoreenException) {
    outport_.setData(0);
    delete geometry_;
    geometry_ = 0;

    Processor::deinitialize();
}

} // namespace
