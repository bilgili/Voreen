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

#ifndef VRN_VOLUMECOLLECTIONSOURCEPROCESSOR_H
#define VRN_VOLUMECOLLECTIONSOURCEPROCESSOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"

namespace voreen {

class Volume;

/**
 * Loads multiple volumes and provides them
 * as VolumeCollection through its outport.
 */
class VRN_CORE_API VolumeCollectionSource : public Processor {

public:
    VolumeCollectionSource();
    virtual ~VolumeCollectionSource();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "VolumeCollectionSource"; }
    virtual std::string getCategory() const   { return "Input";                  }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;        }

    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Assigns a volume collection to this processor.
     *
     * @param owner if true, the processor takes ownership of the passed volumes
     */
    void setVolumeCollection(VolumeCollection* collection, bool owner = false);

    /**
     * Returns the currently assigned volume collection.
     */
    VolumeCollection* getVolumeCollection() const;

protected:
    virtual void setDescriptions() {
        setDescription("Loads multiple volumes and provides them as VolumeCollection.");
    }

    virtual void process();
    virtual void initialize() throw (tgt::Exception);

    /// The volume port the loaded data set is written to.
    VolumeCollectionPort outport_;

    /// Property storing the loaded volume collection.
    VolumeURLListProperty volumeURLList_;

    static const std::string loggerCat_;
};

} // namespace

#endif
