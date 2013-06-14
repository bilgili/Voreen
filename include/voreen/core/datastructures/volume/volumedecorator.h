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

#ifndef VRN_VOLUMEHANDLEDECORATOR_H
#define VRN_VOLUMEHANDLEDECORATOR_H

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"

#include "tgt/matrix.h"
#include "tgt/vector.h"

namespace voreen {

///Basic decorator:
class VRN_CORE_API VolumeDecoratorIdentity : public VolumeBase, public VolumeObserver {
public:
    VolumeDecoratorIdentity(const VolumeBase* vhb);
    ~VolumeDecoratorIdentity() {}

    virtual const VolumeBase* getDecorated() const {
        return base_;
    }

    virtual size_t getNumRepresentations() const {
        return base_->getNumRepresentations();
    }

    virtual const VolumeRepresentation* getRepresentation(size_t i) const {
        return base_->getRepresentation(i);
    }

    virtual const VolumeRepresentation* useConverter(const RepresentationConverterBase* converter) const {
        return base_->useConverter(converter);
    }

    virtual void addRepresentation(VolumeRepresentation* rep) {
        const_cast<VolumeBase*>(base_)->addRepresentation(rep);
    }

    virtual void removeRepresentation(size_t i) {
        const_cast<VolumeBase*>(base_)->removeRepresentation(i);
    }

    virtual std::vector<std::string> getMetaDataKeys() const {
        return base_->getMetaDataKeys();
    }

    virtual const MetaDataBase* getMetaData(const std::string& key) const {
        return base_->getMetaData(key);
    }

    virtual bool hasMetaData(const std::string& key) const {
        return base_->hasMetaData(key);
    }

    virtual Modality getModality() const {
        return base_->getModality();
    }

    //VolumeObserver implementation:
    virtual void volumeDelete(const VolumeBase* /*source*/) {
        // not ideal, but the best we can do here:
        base_ = 0;
        notifyDelete();
    }

    virtual void volumeChange(const VolumeBase* /*source*/) {
        notifyReload();
    }

protected:
    const VolumeBase* base_;
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing a metadata item.
class VRN_CORE_API VolumeDecoratorReplace : public VolumeDecoratorIdentity {
public:
    /**
     * Decorates a volumehandle, replacing a metadata item.
     *
     * @param vhb The Volume to decorate
     * @param key Key of the MetaData item to replace.
     * @param value New Value. The decorator takes ownership.
     */
    VolumeDecoratorReplace(const VolumeBase* vhb, const std::string& key, MetaDataBase* value);
    ~VolumeDecoratorReplace() { delete value_; }

    virtual std::vector<std::string> getMetaDataKeys() const;
    virtual const MetaDataBase* getMetaData(const std::string& key) const;
    virtual bool hasMetaData(const std::string& key) const;

    MetaDataBase* getValue() const;
    void setValue(MetaDataBase* value);

protected:
    std::string key_;
    MetaDataBase* value_;
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing its physical-to-world transformation matrix.
class VRN_CORE_API VolumeDecoratorReplaceTransformation : public VolumeDecoratorReplace {
public:
    VolumeDecoratorReplaceTransformation(const VolumeBase* vhb, tgt::mat4 matrix) :
      VolumeDecoratorReplace(vhb, "Transformation", new Mat4MetaData(matrix)) {}
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing its voxel spacing.
class VRN_CORE_API VolumeDecoratorReplaceSpacing : public VolumeDecoratorReplace {
public:
    VolumeDecoratorReplaceSpacing(const VolumeBase* vhb, tgt::vec3 spacing) :
      VolumeDecoratorReplace(vhb, "Spacing", new Vec3MetaData(spacing)) {}
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing its offset in physical coordinates.
class VRN_CORE_API VolumeDecoratorReplaceOffset : public VolumeDecoratorReplace {
public:
    VolumeDecoratorReplaceOffset(const VolumeBase* vhb, tgt::vec3 offset) :
      VolumeDecoratorReplace(vhb, "Offset", new Vec3MetaData(offset)) {}
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing its real-world mapping.
class VRN_CORE_API VolumeDecoratorReplaceRealWorldMapping : public VolumeDecoratorReplace {
public:
    VolumeDecoratorReplaceRealWorldMapping(const VolumeBase* vhb, RealWorldMapping rwm) :
      VolumeDecoratorReplace(vhb, "RealWorldMapping", new RealWorldMappingMetaData(rwm)) {}
};

//-------------------------------------------------------------------------------------------------

/// Decorates a Volume, replacing its timestep.
class VRN_CORE_API VolumeDecoratorReplaceTimestep : public VolumeDecoratorReplace {
public:
    VolumeDecoratorReplaceTimestep(const VolumeBase* vhb, float timestep) :
      VolumeDecoratorReplace(vhb, "Timestep", new FloatMetaData(timestep)) {}
};

} // namespace

#endif
