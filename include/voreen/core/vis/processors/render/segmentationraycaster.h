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

    /**
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - setTransferFunction, which sets the current transfer-function. Msg-Type: TransferFunc*
     *
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

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

    void setCurrentSegment(int id);
    int getCurrentSegment() const;

    void setSegmentVisible(int segment, bool visible);
    bool isSegmentVisible(int segment) const;

    BoolProp& getApplySegmentationProp();
    IntProp& getCurrentSegmentProp();
    BoolProp& getSegmentVisibleProp();

protected:
    virtual std::string generateHeader();
    virtual void compile();

private:

    // callbacks for gui changes
    void currentSegmentChanged();
    void segmentVisibilityChanged();
    void applySegmentationChanged();
    void transFuncChanged();
    void transFuncResolutionChanged();

    // writes the contents of the transferFunc_ prop into the 2D transfer function at the position
    // corresponding to the passed segment id
    void updateSegmentationTransFuncTex(int segment);

    // Saves/restores the segment's TransFuncKeys/visibility-flag to/from the maps (see below)
    void saveSegmentTransFunc(int segment);
    void saveStandardTransFunc();
    void restoreSegmentTransFunc(int segment);
    void restoreStandardTransFunc();
    // Returns wether there are TransFuncKeys/visibility-flag available for the passed segment id
    bool hasStoredSegmentTransFunc(int segment);
    // Resets the transferFunc_ prop to a standard ramp
    void generateStandardTransFunc();
    // clears the cached segment transfunc keys
    void clearSegmentTransFuncs();

    void generateSegmentationTransFunc();

    // The transfer function property used for the manipulation of the segment transfer functions
    TransFuncProp transferFunc_;

    // 2D texture that contains the segments' 1D transfer functions in row-order
    tgt::Texture* segmentationTransFuncTex_;
    bool segmentationTransFuncTexValid_;

    VolumeHandle* segmentationHandle_;

    BoolProp applySegmentation_;
    IntProp currentSegment_;
    BoolProp segmentVisible_;
    EnumProp* transFuncResolutionProp_;
    std::vector<std::string> transFuncResolutions_;
    int lastSegment_;

    // these maps map from the segment id to the stored TransFuncKeys/visibility flag of a segment
    std::map< int, std::vector<TransFuncMappingKey*> > segmentTransFuncKeys_;
    std::map< int, tgt::vec2 > segmentThresholds_;
    std::map< int, bool > segmentVisibilities_;

    // TransFuncKeys of the 1D TF applied when no segmentation is applied
    std::vector<TransFuncMappingKey*> standardTransFuncKeys_;
    tgt::vec2 standardThresholds_;

    EnumProp* compositingMode1_;    ///< What compositing mode should be applied for second outport
    EnumProp* compositingMode2_;    ///< What compositing mode should be applied for third outport

    bool destActive_[2];

};


} // namespace

#endif // VRN_SEGMENTATIONRAYCASTER_H
