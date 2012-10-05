/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"

#include "tgt/filesystem.h"

namespace voreen {

VolumePort::VolumePort(PortDirection direction, const std::string& id, const std::string& guiName,
      bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
    : GenericPort<VolumeBase>(direction, id, guiName, allowMultipleConnections, invalidationLevel),
      VolumeHandleObserver(),
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
    strstr  << getGuiName() << std::endl 
            << "Type: " << getClassName() << std::endl;

    if (getData() && getData()->getRepresentation<VolumeRAM>()) {
            const VolumeBase* h = getData();
            const VolumeRAM* v = getData()->getRepresentation<VolumeRAM>();
            std::string type;
            if (dynamic_cast<const VolumeRAM_UInt8*>(v))    type = "UInt8";    else
            if (dynamic_cast<const VolumeRAM_UInt16*>(v))   type = "UInt16";   else
            if (dynamic_cast<const VolumeRAM_UInt32*>(v))   type = "UInt32";   else
            if (dynamic_cast<const VolumeRAM_UInt64*>(v))   type = "UInt64";   else
            if (dynamic_cast<const VolumeRAM_Int8*>(v))     type = "Int8";     else
            if (dynamic_cast<const VolumeRAM_Int16*>(v))    type = "Int16";    else
            if (dynamic_cast<const VolumeRAM_Int32*>(v))    type = "Int32";    else
            if (dynamic_cast<const VolumeRAM_Int64*>(v))    type = "Int64";    else
            if (dynamic_cast<const VolumeRAM_Float*>(v))    type = "Float";    else
            if (dynamic_cast<const VolumeRAM_Double*>(v))   type = "Double";   else
            if (dynamic_cast<const VolumeRAM_2xUInt8*>(v))  type = "2xUInt8";  else
            if (dynamic_cast<const VolumeRAM_2xUInt16*>(v)) type = "2xUInt16"; else
            if (dynamic_cast<const VolumeRAM_2xUInt32*>(v)) type = "2xUInt32"; else
            if (dynamic_cast<const VolumeRAM_2xUInt64*>(v)) type = "2xUInt64"; else
            if (dynamic_cast<const VolumeRAM_2xInt8*>(v))   type = "2xInt8";   else
            if (dynamic_cast<const VolumeRAM_2xInt16*>(v))  type = "2xInt16";  else
            if (dynamic_cast<const VolumeRAM_2xInt32*>(v))  type = "2xInt32";  else
            if (dynamic_cast<const VolumeRAM_2xInt64*>(v))  type = "2xInt64";  else
            if (dynamic_cast<const VolumeRAM_2xFloat*>(v))  type = "2xFloat";  else
            if (dynamic_cast<const VolumeRAM_2xDouble*>(v)) type = "2xDouble"; else
            if (dynamic_cast<const VolumeRAM_3xUInt8*>(v))  type = "3xUInt8";  else
            if (dynamic_cast<const VolumeRAM_3xUInt16*>(v)) type = "3xUInt16"; else
            if (dynamic_cast<const VolumeRAM_3xUInt32*>(v)) type = "3xUInt32"; else
            if (dynamic_cast<const VolumeRAM_3xUInt64*>(v)) type = "3xUInt64"; else
            if (dynamic_cast<const VolumeRAM_3xInt8*>(v))   type = "3xInt8";   else
            if (dynamic_cast<const VolumeRAM_3xInt16*>(v))  type = "3xInt16";  else
            if (dynamic_cast<const VolumeRAM_3xInt32*>(v))  type = "3xInt32";  else
            if (dynamic_cast<const VolumeRAM_3xInt64*>(v))  type = "3xInt64";  else
            if (dynamic_cast<const VolumeRAM_3xFloat*>(v))  type = "3xFloat";  else
            if (dynamic_cast<const VolumeRAM_3xDouble*>(v)) type = "3xDouble"; else
            if (dynamic_cast<const VolumeRAM_4xUInt8*>(v))  type = "4xUInt8";  else
            if (dynamic_cast<const VolumeRAM_4xUInt16*>(v)) type = "4xUInt16"; else
            if (dynamic_cast<const VolumeRAM_4xUInt32*>(v)) type = "4xUInt32"; else
            if (dynamic_cast<const VolumeRAM_4xUInt64*>(v)) type = "4xUInt64"; else
            if (dynamic_cast<const VolumeRAM_4xInt8*>(v))   type = "4xInt8";   else
            if (dynamic_cast<const VolumeRAM_4xInt16*>(v))  type = "4xInt16";  else
            if (dynamic_cast<const VolumeRAM_4xInt32*>(v))  type = "4xInt32";  else
            if (dynamic_cast<const VolumeRAM_4xInt64*>(v))  type = "4xInt64";  else
            if (dynamic_cast<const VolumeRAM_4xFloat*>(v))  type = "4xFloat";  else
            if (dynamic_cast<const VolumeRAM_4xDouble*>(v)) type = "4xDouble"; else
                                                            type = "<unknown>";

    strstr << "Data Type: " << type << std::endl
                   << "Dimension: " << h->getDimensions()[0] << " x " << h->getDimensions()[1] << " x " << h->getDimensions()[2]  << std::endl
                   << "Spacing: "   << h->getSpacing()[0] << " x " << h->getSpacing()[1] << " x " << h->getSpacing()[2]    << std::endl
                   << "Bits Per Voxel: " << v->getBitsAllocated() << std::endl
                   << "Num Voxels: "<< v->getNumVoxels() << std::endl
                   << "Memory Size: ";
            size_t bytes = v->getNumBytes();
            float mb = tgt::round(bytes/104857.6f) / 10.f;    //calculate mb with 0.1f precision
            float kb = tgt::round(bytes/102.4f) / 10.f;
            if (mb >= 0.5f) {
                strstr << mb << " MB";
            }
            else if (kb >= 0.5f) {
                strstr << kb << " kB";
            }
            else {
                strstr << bytes << " bytes";
            }            
    }
    return strstr.str();
}

std::string VolumePort::getContentDescriptionHTML() const {
    std::stringstream strstr;
    strstr  << "<center><font><b>" << getGuiName() << "</b></font></center>"
            << "Type: " << getClassName() << "<br>";
                
    if (getData() && getData()->getRepresentation<VolumeRAM>()) {
            const VolumeBase* h = getData();
            const VolumeRAM* v = getData()->getRepresentation<VolumeRAM>();
            std::string type;
            if (dynamic_cast<const VolumeRAM_UInt8*>(v))    type = "UInt8";    else
            if (dynamic_cast<const VolumeRAM_UInt16*>(v))   type = "UInt16";   else
            if (dynamic_cast<const VolumeRAM_UInt32*>(v))   type = "UInt32";   else
            if (dynamic_cast<const VolumeRAM_UInt64*>(v))   type = "UInt64";   else
            if (dynamic_cast<const VolumeRAM_Int8*>(v))     type = "Int8";     else
            if (dynamic_cast<const VolumeRAM_Int16*>(v))    type = "Int16";    else
            if (dynamic_cast<const VolumeRAM_Int32*>(v))    type = "Int32";    else
            if (dynamic_cast<const VolumeRAM_Int64*>(v))    type = "Int64";    else
            if (dynamic_cast<const VolumeRAM_Float*>(v))    type = "Float";    else
            if (dynamic_cast<const VolumeRAM_Double*>(v))   type = "Double";   else
            if (dynamic_cast<const VolumeRAM_2xUInt8*>(v))  type = "2xUInt8";  else
            if (dynamic_cast<const VolumeRAM_2xUInt16*>(v)) type = "2xUInt16"; else
            if (dynamic_cast<const VolumeRAM_2xUInt32*>(v)) type = "2xUInt32"; else
            if (dynamic_cast<const VolumeRAM_2xUInt64*>(v)) type = "2xUInt64"; else
            if (dynamic_cast<const VolumeRAM_2xInt8*>(v))   type = "2xInt8";   else
            if (dynamic_cast<const VolumeRAM_2xInt16*>(v))  type = "2xInt16";  else
            if (dynamic_cast<const VolumeRAM_2xInt32*>(v))  type = "2xInt32";  else
            if (dynamic_cast<const VolumeRAM_2xInt64*>(v))  type = "2xInt64";  else
            if (dynamic_cast<const VolumeRAM_2xFloat*>(v))  type = "2xFloat";  else
            if (dynamic_cast<const VolumeRAM_2xDouble*>(v)) type = "2xDouble"; else
            if (dynamic_cast<const VolumeRAM_3xUInt8*>(v))  type = "3xUInt8";  else
            if (dynamic_cast<const VolumeRAM_3xUInt16*>(v)) type = "3xUInt16"; else
            if (dynamic_cast<const VolumeRAM_3xUInt32*>(v)) type = "3xUInt32"; else
            if (dynamic_cast<const VolumeRAM_3xUInt64*>(v)) type = "3xUInt64"; else
            if (dynamic_cast<const VolumeRAM_3xInt8*>(v))   type = "3xInt8";   else
            if (dynamic_cast<const VolumeRAM_3xInt16*>(v))  type = "3xInt16";  else
            if (dynamic_cast<const VolumeRAM_3xInt32*>(v))  type = "3xInt32";  else
            if (dynamic_cast<const VolumeRAM_3xInt64*>(v))  type = "3xInt64";  else
            if (dynamic_cast<const VolumeRAM_3xFloat*>(v))  type = "3xFloat";  else
            if (dynamic_cast<const VolumeRAM_3xDouble*>(v)) type = "3xDouble"; else
            if (dynamic_cast<const VolumeRAM_4xUInt8*>(v))  type = "4xUInt8";  else
            if (dynamic_cast<const VolumeRAM_4xUInt16*>(v)) type = "4xUInt16"; else
            if (dynamic_cast<const VolumeRAM_4xUInt32*>(v)) type = "4xUInt32"; else
            if (dynamic_cast<const VolumeRAM_4xUInt64*>(v)) type = "4xUInt64"; else
            if (dynamic_cast<const VolumeRAM_4xInt8*>(v))   type = "4xInt8";   else
            if (dynamic_cast<const VolumeRAM_4xInt16*>(v))  type = "4xInt16";  else
            if (dynamic_cast<const VolumeRAM_4xInt32*>(v))  type = "4xInt32";  else
            if (dynamic_cast<const VolumeRAM_4xInt64*>(v))  type = "4xInt64";  else
            if (dynamic_cast<const VolumeRAM_4xFloat*>(v))  type = "4xFloat";  else
            if (dynamic_cast<const VolumeRAM_4xDouble*>(v)) type = "4xDouble"; else
                                                            type = "<unknown>";

    strstr << "Data Type: " << type << "<br>"
                   << "Dimension: " << h->getDimensions()[0] << " x " << h->getDimensions()[1] << " x " << h->getDimensions()[2]  << "<br>"
                   << "Spacing: "   << h->getSpacing()[0] << " x " << h->getSpacing()[1] << " x " << h->getSpacing()[2]    << "<br>"
                   << "Bits Per Voxel: " << v->getBitsAllocated() << "<br>"
                   << "Num Voxels: "<< v->getNumVoxels() << "<br>"
                   << "Memory Size: ";
            size_t bytes = v->getNumBytes();
            float mb = tgt::round(bytes/104857.6f) / 10.f;    //calculate mb with 0.1f precision
            float kb = tgt::round(bytes/102.4f) / 10.f;
            if (mb >= 0.5f) {
                strstr << mb << " MB";
            }
            else if (kb >= 0.5f) {
                strstr << kb << " kB";
            }
            else {
                strstr << bytes << " bytes";
            }            
    }
    return strstr.str();
}

bool VolumePort::isReady() const {
    if (isOutport())
        return isConnected();
    else
        return (hasData() && getData()->getRepresentation<VolumeRAM>() && checkConditions());
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

void VolumePort::volumeHandleDelete(const VolumeBase* source) {
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
        VolumeCollection* volumeCollection = serializer->read(filename);
        tgtAssert(!volumeCollection->empty(), "empty collection");
        VolumeBase* dataset = volumeCollection->first();
        setData(dataset, true);
        //we do not need the collection, just the volume:
        delete volumeCollection;
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
