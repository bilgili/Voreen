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

#include "voreen/modules/connectedcomponents/connectedcomponents3d.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include "connexe/connexe.h"

namespace voreen {

const std::string ConnectedComponents3D::loggerCat_("voreen.ConnectedComponents3D");

ConnectedComponents3D::ConnectedComponents3D()
    : VolumeProcessor(),
      inport_(Port::INPORT, "inport"),
      outport_(Port::OUTPORT, "outport"),
      enableProcessing_("enabled", "Enable", true),
      connectivity_("connectivity", "Connectivity"),
      minComponentSize_("minComponentSize", "Min Component Size", 1, 1, 10000000),
      maxComponents_("maxComponents", "Max Components", (1<<16)-1, 1, (1<<16)-1),
      componentSorting_("sorting", "Component Sorting"),
      binarizeOutput_("binarizeOutput", "Binarize Output", false),
      stretchLabels_("stretchLabels", "Stretch Labels", true),
      volumeOwner_(false)
{
    addPort(inport_);
    addPort(outport_);

    connectivity_.addOption("6-neighborhood", "6 Neighborhood", 6);
    connectivity_.addOption("10-neighborhood", "10 Neighborhood", 10);
    connectivity_.addOption("18-neighborhood", "18 Neighborhood", 18);
    connectivity_.addOption("26-neighborhood", "26 Neighborhood", 26);
    connectivity_.select("18-neighborhood");

    componentSorting_.addOption("none", "None");
    componentSorting_.addOption("decreasing", "Decreasing Size");
    componentSorting_.addOption("increasing", "Increasing Size");

    addProperty(enableProcessing_);
    addProperty(connectivity_);
    addProperty(minComponentSize_);
    addProperty(maxComponents_);
    addProperty(componentSorting_);
    addProperty(binarizeOutput_);
    addProperty(stretchLabels_);
}

ConnectedComponents3D::~ConnectedComponents3D() {
}

Processor* ConnectedComponents3D::create() const {
    return new ConnectedComponents3D();
}

std::string ConnectedComponents3D::getProcessorInfo() const {
    return "Detects connected components in a volume data set and intensity-codes the assigned labels "
        "in a 16 bit output volume of the same dimensions."
        "<p><strong>Properties</strong>:"
        "<ul>"
        "  <li>Connectivity: voxel neighborhood to consider.</li>"
        "  <li>Min Component Size: components consisting of less voxels are discarded.</li>"
        "  <li>Max Components: maximal number of connected components to put out.</li>"
        "  <li>Component Sorting: determines in which order the component labels are assigned.</li>"
        "  <li>Binarize Output: all connected components are assigned the same label (max intensity).</li>"
        "  <li>Stretch Labels: component labels are stretched over the whole intensity range of the output volume in order to maximize contrast.</li>"
        "</ul></p>"
        "<p><strong>See</strong>: ConnectedComponents2D</p>";
}

void ConnectedComponents3D::process() {

    tgtAssert(inport_.getData() && inport_.getData()->getVolume(), "No volume");

    if (!enableProcessing_.get()) {
        outport_.setData(inport_.getData(), volumeOwner_);
        volumeOwner_ = false;
        return;
    }

    Volume* volume = inport_.getData()->getVolume();
    tgt::ivec3 volDim = volume->getDimensions();

    VolumeUInt8* volUint8 = dynamic_cast<VolumeUInt8*>(volume);
    VolumeUInt16* volUint16 = dynamic_cast<VolumeUInt16*>(volume);
    VolumeFloat* volFloat = dynamic_cast<VolumeFloat*>(volume);
    if (!volUint8 && !volUint16 && !volFloat) {
        LERROR("Unsupported volume type: VolumeUInt8, VolumeUInt16 or VolumeFloat expected.");
        outport_.setData(0, volumeOwner_);
        volumeOwner_ = false;
        return;
    }

    // compute connected component labels
    uint16_t* labels = 0;
    try {
        labels = new uint16_t[volume->getNumVoxels()];
    }
    catch (std::bad_alloc) {
        LERROR("Failed to create label volume: bad allocation");
        outport_.setData(0, volumeOwner_);
        volumeOwner_ = false;
        return;
    }
    int numLabels = CountConnectedComponentsWithAllParams(
        reinterpret_cast<void*>(volume->getData()), (volUint8 ? UCHAR_TYPE : (volUint16 ? USHORT_TYPE : FLOAT_TYPE)),
        reinterpret_cast<void*>(labels), USHORT_TYPE, volume->getDimensions().elem, 1.0,
        connectivity_.getValue(),minComponentSize_.get(), maxComponents_.get(), binarizeOutput_.get() ? 1 : 0);

    // sort components
    if ((numLabels > 0) && !componentSorting_.isSelected("none") && !binarizeOutput_.get()) {
        int decreasing = componentSorting_.isSelected("decreasing") ? 1 : 0;
        RelabelConnectedComponentsBySize(labels, USHORT_TYPE, volDim.elem, decreasing);
    }

    // stretch labels to use full range
    if ((numLabels > 0) && stretchLabels_.get() && !binarizeOutput_.get()) {
        float scale = static_cast<float>((1<<16) - 2) / numLabels;
        for (size_t i=0; i<volume->getNumVoxels(); i++)
            labels[i] = tgt::ifloor(labels[i]*scale);
    }

    // assign label volume to outport
    VolumeUInt16* labelVolume = new VolumeUInt16(labels, volume->getDimensions(), volume->getSpacing());
    labelVolume->setTransformation(volume->getTransformation());
    outport_.setData(new VolumeHandle(labelVolume), volumeOwner_);
    volumeOwner_ = true;

    LGL_ERROR;
}

void ConnectedComponents3D::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    VolumeProcessor::deinitialize();
}

} // namespace
