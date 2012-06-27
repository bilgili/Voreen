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

#ifndef VRN_VOLUMECOLLECTIONSOURCEPROCESSOR_H
#define VRN_VOLUMECOLLECTIONSOURCEPROCESSOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/volumecollectionproperty.h"

namespace voreen {

class Volume;

/**
 * Loads multiple volumes and provides them
 * as VolumeCollection through its outport.
 */
class VolumeCollectionSource : public Processor {

public:
    VolumeCollectionSource();
    virtual ~VolumeCollectionSource();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "VolumeCollectionSource"; }
    virtual std::string getCategory() const   { return "Data Source"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Assigns a volume collection to this processor.
     *
     * @note The currently assigned collection is not freed.
     */
    void setVolumeCollection(VolumeCollection* collection);

    /**
     * Returns the currently assigned volume collection.
     */
    VolumeCollection* getVolumeCollection() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);

    /// The volume port the loaded data set is written to.
    VolumeCollectionPort outport_;

    /// Property storing the loaded volume collection.
    VolumeCollectionProperty volumeCollection_;

    static const std::string loggerCat_;
};

} // namespace

#endif
