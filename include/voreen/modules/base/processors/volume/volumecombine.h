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

#ifndef VRN_VOLUMECOMBINE_H
#define VRN_VOLUMECOMBINE_H

#include "voreen/core/processors/volumeprocessor.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"

namespace voreen {

class VolumeCombine : public VolumeProcessor {
public:
    VolumeCombine();
    ~VolumeCombine();
    virtual Processor* create() const;

    virtual std::string getClassName() const      { return "VolumeCombine";     }
    virtual std::string getCategory() const       { return "Volume Processing"; }
    virtual CodeState getCodeState() const        { return CODE_STATE_STABLE;   }
    virtual bool usesExpensiveComputation() const { return true; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void deinitialize() throw (VoreenException);

private:
    /// Voxel-wise combine operation.
    enum CombineOperation {
        OP_ADD,
        OP_A_MINUS_B,
        OP_B_MINUS_A,
        OP_AVG,
        OP_MAX,
        OP_MIN,
        OP_WEIGHTED_SUM,
        OP_WEIGHTED_SUM_2P,
        OP_BLEND,
        OP_MASK_A_BY_B,
        OP_MASK_B_BY_A,
        OP_PRIORITY_FIRST,
        OP_PRIORITY_SECOND,
        OP_TAKE_FIRST,
        OP_TAKE_SECOND
    };

    /**
     * Combines the input volumes and writes the result to combinedVolume (which is assumed to be already created),
     * by transforming the input volume's coordinates systems to the coordinates system of the combined volume.
     */
    void combineVolumes(Volume* combinedVolume, const Volume* firstVolume,
        const Volume* secondVolume, CombineOperation operation) const;

    /**
     * Combines the input volumes and writes the result to combinedVolume (which is assumed to be already created),
     * without coordinate transformation. This is much faster than the transformation-based combination,
     * but only possible for volumes that share a common grid in world space.
     */
    void combineVolumesOnCommonGrid(Volume* combinedVolume, const Volume* firstVolume,
        const Volume* secondVolume, CombineOperation operation) const;

    /// Creates a combined (empty) volume from the two input volumes in world space,
    /// or 0 in case the combined volume could not be created due to bad allocation.
    Volume* createCombinedVolume(const Volume* refVolume, const Volume* secondVolume) const;

    /// Adjust property visibilites according to selected combine function.
    void adjustPropertyVisibilities();

    VolumePort inportFirst_;
    VolumePort inportSecond_;
    VolumePort outport_;

    BoolProperty enableProcessing_;
    OptionProperty<CombineOperation> combineFunction_;
    FloatProperty factorC_;
    FloatProperty factorD_;
    StringOptionProperty referenceVolume_;

    bool volumeOwner_;
    static const std::string loggerCat_; ///< category used in logging
};


} // namespace

#endif // VRN_VOLUMECOMBINE_H
