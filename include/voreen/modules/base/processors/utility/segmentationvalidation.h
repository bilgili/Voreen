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

#ifndef VRN_SEGMENTATIONVALIDATION_H
#define VRN_SEGMENTATIONVALIDATION_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/volumeport.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"

namespace voreen {

/**
 * Measures the quality of a volume segmentation by comparing it against a reference segmentation.
 *
 * Several similarity measures are computed, such as Jaccard and Dice indices. The results are
 * displayed by read-only properties.
 */
class SegmentationValidation : public Processor {
public:
    SegmentationValidation();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "SegmentationValidation"; }
    virtual std::string getCategory() const  { return "Utility"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual bool isUtility() const           { return true; }
    virtual bool isEndProcessor() const      { return true; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();

private:
    void forceUpdate();
    void resetOutput();

    void computeMetrics();
    size_t getNumCommonVoxels(Volume* volumeA, Volume* volumeB);

    VolumePort inportSegmentation_;
    VolumePort inportReference_;

    ButtonProperty computeButton_;
    IntProperty sizeSegmentation_;
    IntProperty sizeReference_;
    IntProperty truePositive_;
    IntProperty falsePositive_;
    IntProperty falseNegative_;
    IntProperty trueNegative_;
    FloatProperty jaccardIndex_;
    FloatProperty diceIndex_;
    FloatProperty sensitivity_;
    FloatProperty specificity_;

    bool forceUpdate_;

private:
    static const std::string loggerCat_; ///< category used in logging
};


} // namespace

#endif // VRN_SEGMENTATIONVALIDATION_H
