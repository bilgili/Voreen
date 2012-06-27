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

#ifndef VRN_REGIONGROWINGPROCESSOR_H
#define VRN_REGIONGROWINGPROCESSOR_H

#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/vis/processors/geometry/geometryprocessor.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/vis/processors/image/renderstore.h"

namespace voreen {

class RenderStore;

/**
 * Interactive volume segmentation by region growing.
 *
 * Input:
 *  - volume to be segmented
 *  - co-processor input connected to a RenderStore that has to store
 *    the raycaster's first-hit-points
 * Output: segmented volume
 */
class RegionGrowingProcessor : public VolumeRenderer {
public:
    RegionGrowingProcessor();

    virtual ~RegionGrowingProcessor();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "RegionGrowing"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new RegionGrowingProcessor(); }

    virtual void process();

    VolumeHandle* getSegmentation() const { return segmentationHandle_; }

    FloatVec2Property& getThresholdProp();
    BoolProperty& getThresholdFillingProp();
    FloatProperty& getStrictnessProp();
    BoolProperty& getAdaptiveProp();
    IntProperty& getMaxSeedDistanceProp();
    StringOptionProperty& getCostFunctionProp();

    void clearSegmentation();
    void clearSegment(int segmentID);
    void undoLastGrowing();
    void startGrowing(tgt::ivec2 seedPos, int segmentID);
    void saveSegmentation(std::string filename) const;
    void loadSegmentation(std::string filename);
    void saveSegment(int segmentID, std::string filename);

protected:
    void mark(const tgt::ivec3& seedpos, int segment);
    void volumeModified(VolumeHandle* v);

    VolumeHandle* segmentationHandle_;

    FloatProperty strictness_;         ///< influences cost function for filling
    BoolProperty thresholdFilling_;    ///< use threshold as additional restriction for filling?
    FloatVec2Property thresholds_;     ///< intensity thresholds which are applied when thresholdFilling_ is true
    StringOptionProperty fillCostFunction_;    ///< cost function to use
    BoolProperty adaptive_;            ///< adjust criteria during growing
    IntProperty maxSeedDistance_;

    VolumeUInt8* lastSegmentation_; ///< used for undo functionality

    VolumePort inport_;
    VolumePort outport_;
    GenericCoProcessorPort<RenderStore> cpp_;

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace

#endif // VRN_REGIONGROWINGPROCESSOR_H
