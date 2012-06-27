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

#ifndef VRN_CONNECTED_COMPONENTS_3D_H
#define VRN_CONNECTED_COMPONENTS_3D_H

#include "voreen/modules/base/processors/volume/volumeprocessor.h"
#include "voreen/core/properties/allproperties.h"

namespace voreen {

/**
 * Detects connected components in a volume data set and intensity-codes the assigned labels
 * in a 16 bit output volume of the same dimensions.
 *
 * @see ConnectedComponents2D
 */
class ConnectedComponents3D : public VolumeProcessor {
public:
    ConnectedComponents3D();
    ~ConnectedComponents3D();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "ConnectedComponents3D"; }
    virtual std::string getCategory() const     { return "Volume Processing"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void deinitialize() throw (VoreenException);

private:
    VolumePort inport_;     ///< Volume to analyze.
    VolumePort outport_;    ///< Output volume storing the components' labels.

    BoolProperty enableProcessing_;         ///< If set to false, the input volume is passed through.
    IntOptionProperty connectivity_;        ///< Voxel neighborhood to consider for the analysis.
    IntProperty minComponentSize_;          ///< Components consisting of less voxels are discarded.
    IntProperty maxComponents_;             ///< The maximal number of connected components that are put out.
    StringOptionProperty componentSorting_; ///< Determines in which order the component labels are assigned.
    BoolProperty binarizeOutput_;           ///< If enabled, all connected components are assigned the same label (max intensity).
    BoolProperty stretchLabels_;            ///< If enabled, component labels are stretched over the whole intensity range
                                            ///  of the output volume in order to maximize contrast.

    bool volumeOwner_;
    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_CONNECTED_COMPONENTS_3D_H
