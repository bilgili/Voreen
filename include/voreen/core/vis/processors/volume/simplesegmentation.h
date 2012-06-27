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

#ifndef VRN_SIMPLESEGMENTATIONPROCESSOR_H
#define VRN_SIMPLESEGMENTATIONPROCESSOR_H

#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/volume/volumeatomic.h"

namespace voreen {

/**
 * Renders geometry hints related to the SimpleSegmentationProcessor
 */
class GeomSimpleSegmentation : public GeometryRenderer {
public:
    GeomSimpleSegmentation();

    virtual const Identifier getClassName() const { return "GeometryRenderer.GeomSimpleSegmentation"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new GeomSimpleSegmentation(); }

protected:
    virtual void render(LocalPortMapping*  portMapping);

    BoolProp showSkelPoints_;
    BoolProp showCritPoints_;
    BoolProp showBoundingBox_;
    BoolProp spherePoints_;
    FloatProp sphereSize_;
    IntProp sphereModulo_;
    BoolProp colorSegments_;
};

/**
 * Simple interactive volume segmentation.
 *
 * Input: volume, its segmentation volume, first-hit point image from raycaster (for picking)
 * Coprocessor-Output: Connect to GeomSimpleSegmentation
 * Output: Connect to NullRenderer
 */
class SimpleSegmentationProcessor : public VolumeRenderer {
public:
    SimpleSegmentationProcessor();

    virtual ~SimpleSegmentationProcessor();

    virtual const Identifier getClassName() const { return "Miscellaneous.SimpleSegmentation"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new SimpleSegmentationProcessor(); }

    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
    virtual void process(LocalPortMapping*  portMapping);
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

    VolumeHandle* getSegmentation() const { return segmentation_; }

protected:
    void mark(const tgt::ivec3& seedpos, int segment);
    void volumeModified(VolumeHandle* v);

    VolumeHandle* volume_;
    VolumeHandle* segmentation_;
    int fhp_;

    std::vector<tgt::vec3> skelPoints_;
    std::vector<int> skelSegCounts_;
    std::vector<tgt::vec3> critPoints_;

    FloatProp strictness_;         ///< influences cost function for filling
    BoolProp thresholdFilling_;    ///< use threshold as additional restriction for filling?
    EnumProp* fillCostFunction_;   ///< cost function to use
    BoolProp adaptive_;            ///< adjust criteria during growing
    IntProp maxSeedDistance_;

    VolumeUInt8* lastSegmentation_; ///< used for undo functionality

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace

#endif // VRN_SIMPLESEGMENTATIONPROCESSOR_H
