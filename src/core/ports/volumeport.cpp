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

#include "voreen/core/ports/volumeport.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumelist.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumedisk.h"
#include "voreen/core/datastructures/volume/volumegl.h"
#include "voreen/core/datastructures/octree/volumeoctreebase.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"

#include "tgt/filesystem.h"

namespace voreen {

VolumePort::VolumePort(PortDirection direction, const std::string& id, const std::string& guiName,
      bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
    : GenericPort<VolumeBase>(direction, id, guiName, allowMultipleConnections, invalidationLevel),
      VolumeObserver(),
      texFilterMode_("textureFilterMode_", "Texture Filtering"),
      texClampMode_("textureClampMode_", "Texture Clamp"),
      texBorderIntensity_("textureBorderIntensity", "Texture Border Intensity", 0.f)
{
    // add texture access properties for inports
    if (!isOutport()) {
        // volume texture filtering
        texFilterMode_.addOption("nearest", "Nearest",  GL_NEAREST);
        texFilterMode_.addOption("linear",  "Linear",   GL_LINEAR);
        texFilterMode_.selectByKey("linear");
        addProperty(texFilterMode_);

        // volume texture clamping
        texClampMode_.addOption("clamp",           "Clamp",             GL_CLAMP);
        texClampMode_.addOption("clamp-to-edge",   "Clamp to Edge",     GL_CLAMP_TO_EDGE);
        texClampMode_.addOption("clamp-to-border", "Clamp to Border",   GL_CLAMP_TO_BORDER);
        texClampMode_.selectByKey("clamp-to-edge");
        addProperty(texClampMode_);
        addProperty(texBorderIntensity_);

        // assign texture access properties to property group
        texFilterMode_.setGroupID(id);
        texClampMode_.setGroupID(id);
        texBorderIntensity_.setGroupID(id);
        setPropertyGroupGuiName(id, (isInport() ? "Inport: " : "Outport: ") + guiName);

        showTextureAccessProperties(false);
    }
}

std::string VolumePort::getContentDescription() const {
    std::stringstream strstr;
    //port values
    strstr  << Port::getContentDescription();

    if (hasData()) {
        const VolumeBase* vol = getData();

        strstr << std::endl <<"Data Type: " << vol->getFormat() << std::endl
               << "Dimension: " << vol->getDimensions()[0] << " x " << vol->getDimensions()[1] << " x " << vol->getDimensions()[2]  << std::endl
               << "Voxel Size: "   << vol->getSpacing()[0] << " x " << vol->getSpacing()[1] << " x " << vol->getSpacing()[2] << " mm" << std::endl
               //<< "Bytes per Voxel: " << vol->getBytesPerVoxel() << " bytes" << std::endl
               << "Memory Size: " << formatMemorySize(vol->getNumVoxels()*vol->getNumChannels()*vol->getBytesPerVoxel()) << "\n";

        std::vector<std::string> representations;
        if (vol->hasRepresentation<VolumeDisk>())
            representations.push_back("Disk");
        if (vol->hasRepresentation<VolumeRAM>())
            representations.push_back("RAM");
        if (vol->hasRepresentation<VolumeOctreeBase>())
            representations.push_back("Octree");
        if (vol->hasRepresentation<VolumeGL>())
            representations.push_back("VolumeGL");
        strstr << "Representations: " << strJoin(representations, ",");

    }
    return strstr.str();
}

std::string VolumePort::getContentDescriptionHTML() const {
    std::stringstream strstr;
    //port values
    strstr  << Port::getContentDescriptionHTML();

    if (hasData()) {
        const VolumeBase* vol = getData();
        strstr << "<br/>" << "Data Type: " << vol->getFormat() << "<br/>"
               << "Dimension: " << vol->getDimensions()[0] << " x " << vol->getDimensions()[1] << " x " << vol->getDimensions()[2]  << "<br/>"
               << "Voxel Size: "   << vol->getSpacing()[0] << " x " << vol->getSpacing()[1] << " x " << vol->getSpacing()[2] << " mm"  << "<br/>"
               //<< "Bytes per Voxel: " << vol->getBytesPerVoxel() << "<br/>"
               << "Memory Size: " << formatMemorySize(vol->getNumChannels()*vol->getNumVoxels()*vol->getBytesPerVoxel()) << "<br/>";

        std::vector<std::string> representations;
        if (vol->hasRepresentation<VolumeDisk>())
            representations.push_back("Disk");
        if (vol->hasRepresentation<VolumeRAM>())
            representations.push_back("RAM");
        if (vol->hasRepresentation<VolumeOctreeBase>())
            representations.push_back("Octree");
        if (vol->hasRepresentation<VolumeGL>())
            representations.push_back("VolumeGL");
        strstr << "Representations: " << strJoin(representations, ", ");

    }
    return strstr.str();
}

bool VolumePort::isReady() const {
    if (isOutport())
        return isConnected();
    else
        return (hasData() && /*getData()->getRepresentation<VolumeRAM>() &&*/ checkConditions());
}

void VolumePort::setData(const VolumeBase* handle, bool takeOwnership) {
    tgtAssert(isOutport(), "called setData on inport!");

    if (portData_ != handle) {
        if (portData_)
            portData_->removeObserver(this);

        GenericPort<VolumeBase>::setData(handle, takeOwnership);
        //portData_ = handle;

        if (handle)
            handle->addObserver(this);
    }
    invalidatePort();
}

void VolumePort::volumeDelete(const VolumeBase* source) {
    if (getData() == source) {
        portData_ = 0; // we dont want to trigger automatic delete due to ownership
        invalidatePort();
        //setData(0);
    }
}

void VolumePort::volumeChange(const VolumeBase* source) {
    if (getData() == source) {
        hasChanged_ = true;
        invalidatePort();
    }
}

void VolumePort::showTextureAccessProperties(bool show) {
    if (isOutport()) {
        LERROR("showTextureAccessProperties(): texture access properties only available for inports.");
    }
    else {
        setPropertyGroupVisible(getID(), show);
    }
}

IntOptionProperty& VolumePort::getTextureFilterModeProperty() {
    if (isOutport())
        LERROR("getTextureFilterModeProperty(): only allowed for inports");
    return texFilterMode_;
}

GLEnumOptionProperty& VolumePort::getTextureClampModeProperty() {
    if (isOutport())
        LERROR("getTextureClampModeProperty(): only allowed for inports");
    return texClampMode_;
}

FloatProperty& VolumePort::getTextureBorderIntensityProperty() {
    if (isOutport())
        LERROR("getTextureBorderIntensityProperty(): only allowed for inports");
    return texBorderIntensity_;
}

bool VolumePort::supportsCaching() const {
    return true;
}

std::string VolumePort::getHash() const {
    if (hasData())
        return getData()->getHash();
    else
        return "";
}

void VolumePort::saveData(const std::string& path) const throw (VoreenException) {
    if (!hasData())
        throw VoreenException("Port has no volume");
    tgtAssert(!path.empty(), "empty path");

    // append .vvd if no extension specified
    std::string filename = path;
    if (tgt::FileSystem::fileExtension(filename).empty())
        filename += ".vvd";

    VolumeSerializerPopulator serializerPop;
    const VolumeSerializer* serializer = serializerPop.getVolumeSerializer();
    try {
        serializer->write(filename, getData());
    }
    catch (VoreenException) {
        throw;
    }
    catch (std::exception& e) {
        throw VoreenException(e.what());
    }
}

void VolumePort::loadData(const std::string& path) throw (VoreenException) {
    tgtAssert(!path.empty(), "empty path");

    // append .vvd if no extension specified
    std::string filename = path;
    if (tgt::FileSystem::fileExtension(filename).empty())
        filename += ".vvd";

    VolumeSerializerPopulator serializerPop;
    const VolumeSerializer* serializer = serializerPop.getVolumeSerializer();
    try {
        VolumeList* volumeList = serializer->read(filename);
        tgtAssert(!volumeList->empty(), "empty collection");
        VolumeBase* dataset = volumeList->first();
        setData(dataset, true);
        //we do not need the collection, just the volume:
        delete volumeList;
    }
    catch (VoreenException) {
        throw;
    }
    catch (std::exception& e) {
        throw VoreenException(e.what());
    }
}

tgt::col3 VolumePort::getColorHint() const {
    return tgt::col3(255, 0, 0);
}

} // namespace
