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

#ifndef VRN_SEGMENTATIONRAYCASTER_H
#define VRN_SEGMENTATIONRAYCASTER_H

#include "voreen/core/vis/processors/render/volumeraycaster.h"

#include <map>

namespace voreen {

/**
 * Performs a simple single pass raycasting with only some capabilites.
 */
class SegmentationRaycaster : public VolumeRaycaster {
public:
    /**
     * Constructor.
     */
    SegmentationRaycaster();

    virtual ~SegmentationRaycaster();

    virtual const Identifier getClassName() const { return "Raycaster.SegmentationRaycaster"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new SegmentationRaycaster(); }

    virtual int initializeGL();

    /**
     * Load the needed shader.
     *
     */
    virtual void loadShader();

    /**
     * Performs the raycasting.
     *
     * Initialize two texture units with the entry and exit params and renders
     * a screen aligned quad. The render destination is determined by the
     * invoking class.
     */
    virtual void process(LocalPortMapping*  portMapping);

    TransFuncProp& getTransFunc();

    PropertyVector& getSegmentationTransFuncs();

    BoolProp& getApplySegmentationProp();

protected:
    virtual std::string generateHeader();
    virtual void compile();

private:

    // callbacks for gui changes
    void applySegmentationChanged();
    void segmentationTransFuncChanged(int segment);
    void transFuncResolutionChanged();

    // writes the contents of the transferFunc_ prop into the 2D transfer function at the position
    // corresponding to the passed segment id
    void updateSegmentationTransFuncTex(int segment);

    // create the segmentation transfer function
    void createSegmentationTransFunc();
    // initializes the segmentation transfunc texture from the segmentatio transfer functions
    void initializeSegmentationTransFuncTex();

    // The transfer function property used in non-segmentation mode
    TransFuncProp transferFunc_;

    // Segmentation transfuncs used in segmentation mode
    PropertyVector* segmentTransFuncs_;

    // 2D texture that contains the segments' 1D transfer functions in row-order
    tgt::Texture* segmentationTransFuncTex_;
    // determines whether the segmentation transfunc has to be uploaded before next use
    bool segmentationTransFuncTexValid_;

    VolumeHandle* segmentationHandle_;

    BoolProp applySegmentation_;
    EnumProp* transFuncResolutionProp_;
    std::vector<std::string> transFuncResolutions_;
    int lastSegment_;

    EnumProp* compositingMode1_;    ///< What compositing mode should be applied for second outport
    EnumProp* compositingMode2_;    ///< What compositing mode should be applied for third outport

    bool destActive_[2];

};


} // namespace

#endif // VRN_SEGMENTATIONRAYCASTER_H
