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

#ifndef VRN_VOLUMEHANDLEPROPERTY_H
#define VRN_VOLUMEHANDLEPROPERTY_H

#include "voreen/core/vis/properties/templateproperty.h"
#include "voreen/core/vis/properties/condition.h"

namespace voreen {

class VolumeHandle;

/**
 * Property encapsulating a VolumeHandle object.
 */
class VolumeHandleProperty : public TemplateProperty<VolumeHandle*> {

public:

    /**
     * Constructor.
     *
     * @param ident identifier that is used in serialization
     * @param guiText text that is shown in the gui
     * @param value the initial volume handle to assign
     *
     */
    VolumeHandleProperty(const std::string& id, const std::string& guiText, VolumeHandle* const value = 0,
       Processor::InvalidationLevel invalidationLevel = Processor::INVALID_PARAMETERS);

    virtual ~VolumeHandleProperty();

    /**
     * Sets the stored volume handle to the given one.
     *
     * @note The property does not take ownership of the
     *       assigned volume handle and does therefore NOT
     *       delete it on destruction or when a new handle is assigned.
     */
    void set(VolumeHandle* handle);

    /**
     * Loads the volume specified by filename.
     *
     * @note The property takes ownership of the loaded
     *       volume handle and deletes it on its own
     *       destruction or when a new handle is assigned.
     *
     * @param filename the volume to load
     */
    void loadVolume(const std::string& filename)
        throw (tgt::FileException, std::bad_alloc);

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    virtual std::string toString() const { return ""; }

protected:

    bool handleOwner_;

};

} // namespace voreen

#endif // VRN_VOLUMEHANDLEPROPERTY_H
