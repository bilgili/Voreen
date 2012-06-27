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

#include "voreen/core/properties/volumehandleproperty.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"

#include "voreen/core/properties/propertywidgetfactory.h"

namespace voreen {

VolumeHandleProperty::VolumeHandleProperty(const std::string& id, const std::string& guiText,
                    VolumeHandle* const value, Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<VolumeHandle*>(id, guiText, value, invalidationLevel)
    , handleOwner_(false)
{}

void VolumeHandleProperty::deinitialize() throw (VoreenException) {
    if (handleOwner_) {
        delete value_;
        handleOwner_ = false;
    }
    value_ = 0;
}

void VolumeHandleProperty::set(VolumeHandle* handle) {

    if (get() != handle) {

        VolumeHandle* prevHandle = get();
        std::string errorMsg;
        validate(handle, errorMsg, false);

        if (value_ != handle) {
            LWARNINGC("voreen.VolumeHandleProperty", errorMsg);
            return;
        }

        // execute links
        executeLinks();

        // free handle, if stored value has changed and property is owner
        if ((prevHandle != get()) && handleOwner_)
            delete prevHandle;

        // property does not take ownership of assigned volume handles
        handleOwner_ = false;

        // check if conditions are met and exec actions
        for(size_t j = 0; j < conditions_.size(); ++j)
            conditions_[j]->exec();

        // issues invalidateOwner and updateWidgets
        Property::invalidate();
    }
}

void VolumeHandleProperty::loadVolume(const std::string& filename) throw (tgt::FileException, std::bad_alloc){

    VolumeSerializerPopulator serializerPopulator;
    VolumeCollection* volumeCollection = serializerPopulator.getVolumeSerializer()->load(filename);

    if (volumeCollection && !volumeCollection->empty()) {

        VolumeHandle* handle = volumeCollection->first();
        tgtAssert(handle, "No handle");

        set(handle);

        // property does take ownership of loaded handles
        handleOwner_ = true;
    }

    delete volumeCollection;
}

PropertyWidget* VolumeHandleProperty::createWidget(PropertyWidgetFactory* f)     {
    return f->createWidget(this);
}

void VolumeHandleProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    if (value_)
        s.serialize("value", value_);
}

void VolumeHandleProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    try {
        VolumeHandle* handle = 0;
        try {
            s.deserialize("value", handle);
        }
        catch (XmlSerializationNoSuchDataException&) {
            if (s.getErrors().back().find("'value'") != std::string::npos)
                s.removeLastError();
        }

        if (handle && handle->getVolume())
            set(handle);
        else {
            delete handle;
            set(0);
        }
    }
    catch (SerializationException&) {
        set(0);
    }
}

std::string VolumeHandleProperty::getTypeString() const {
    return "VolumeHandle";
}

} // namespace voreen
