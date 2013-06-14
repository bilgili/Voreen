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

#ifndef VRN_GIGAVOXELOCTREECREATOR_H
#define VRN_GIGAVOXELOCTREECREATOR_H

#include "voreen/core/processors/volumeprocessor.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/filedialogproperty.h"

namespace voreen {

class VolumeOctreeBase;

class OctreeCreator : public VolumeProcessor {
public:
    OctreeCreator();
    virtual ~OctreeCreator();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "OctreeCreator";         }
    virtual std::string getCategory() const   { return "Octree";                }
    virtual CodeState getCodeState() const    { return CODE_STATE_TESTING;      }

    virtual bool usesExpensiveComputation() const { return true; }

protected:
    virtual void setDescriptions() {
        setDescription("Adds an octree volume representation to the input volume. Necessary for subsequent octree processors.");
    }

    virtual void process();

    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    VolumeOctreeBase* generateOctree();

    std::string getOctreeStoragePath() const;
    std::string getConfigurationHash() const;

    void storeOctreeToCache(const VolumeOctreeBase* octree) const;
    VolumeOctreeBase* restoreOctreeFromCache() const;
    void clearCache();
    void regenerateOctree();

    void updatePropertyConfiguration();

    VolumePort volumeInport_;
    VolumePort volumeOutport_;

    IntProperty brickDimensions_;
    FloatProperty homogeneityThreshold_;

    StringOptionProperty brickPoolManager_;
    IntProperty singleBufferMemorySize_;
    IntProperty maxRamUsage_;

    BoolProperty useCaching_;
    FileDialogProperty octreeCachePath_;
    ButtonProperty clearCache_;

    ButtonProperty regenerateOctreeButton_;
    bool regenerateOctree_;

    static const std::string loggerCat_;
};

} //namespace

#endif
