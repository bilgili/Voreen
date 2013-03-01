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

#ifndef VRN_VOLUMESUBSET_H
#define VRN_VOLUMESUBSET_H

#include "voreen/core/processors/volumeprocessor.h"

#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

/**
 * Crops the input volume by axis-aligned clipping planes.
 */
class VRN_CORE_API VolumeCrop : public CachingVolumeProcessor {
public:
    VolumeCrop();
    virtual Processor* create() const;

    virtual std::string getClassName() const      { return "VolumeCrop";        }
    virtual std::string getCategory() const       { return "Volume Processing"; }
    virtual CodeState getCodeState() const        { return CODE_STATE_STABLE;   }
    virtual bool usesExpensiveComputation() const { return true; }

protected:
    virtual void setDescriptions() {
        setDescription("Crops the input volume by axis-aligned clipping planes.");
    }

    virtual void process();

private:
    /// Crops the input volume according to the property values
    /// and writes the result to the outport.
    void crop();

    /// Ensure useful clipping planes on clip right property change.
    virtual void onClipRightChange();

    /// Ensure useful clipping planes on clip left property change.
    virtual void onClipLeftChange();

    /// Ensure useful clipping planes on clip front property change.
    virtual void onClipFrontChange();

    /// Ensure useful clipping planes on clip back property change.
    virtual void onClipBackChange();

    /// Ensure useful clipping planes on clip bottom property change.
    virtual void onClipBottomChange();

    /// Ensure useful clipping planes on clip top property change.
    virtual void onClipTopChange();

    /// Adapt ranges of clip plane properties to the input volume's dimensions.
    void adjustClipPropertiesRanges();

    VolumePort inport_;
    VolumePort outport_;

    IntProperty clipRight_;             ///< Right clipping plane position (x).
    IntProperty clipLeft_;              ///< Left clipping plane position (-x).
    IntProperty clipFront_;             ///< Front clipping plane position (y).
    IntProperty clipBack_;              ///< Back clipping plane position property (-y).
    IntProperty clipBottom_;            ///< Bottom clipping plane position property (z).
    IntProperty clipTop_;               ///< Top clipping plane position property (-z).
    BoolProperty continuousCropping_;   ///< Crop on each change of the input volume.
    ButtonProperty button_;             ///< Perform cropping.

    /// Read-only property displaying the dimensions of the cropped volume.
    IntVec3Property croppedDimensions_;
    /// Read-only property displaying the data size of the cropped volume in MB.
    IntProperty croppedSize_;

    tgt::ivec3 oldVolumeDimensions_;

    static const std::string loggerCat_; ///< category used in logging
};

}   //namespace

#endif
