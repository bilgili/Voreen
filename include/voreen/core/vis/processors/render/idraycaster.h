/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_IDRAYCASTER_H
#define VRN_IDRAYCASTER_H

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Writes color coded regions of a segmented dataset to the alpha channel of
 * the rendering target. The three color channels are filled with the
 * first-hit-positions.
 */
class IDRaycaster : public VolumeRaycaster {
public:

    IDRaycaster();

    /**
     * Destructor. Frees all allocated resources.
     */
    virtual ~IDRaycaster();

    virtual const Identifier getClassName() const {return "Raycaster.IDRaycaster";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new IDRaycaster();}

    /**
     * Reacts to the following messages:
     * - Identifier::switchCoarseness, Type: \c bool
     * - "set.penetrationDepth", Type: \c float. \see penetrationDepth_
     * - Identifier::setTransferFunction, Type: \c TransferFunc1D*.
     * - "set.useBlurring", Type: \c bool. True, if first-hit-positions are blurred.
     * - "set.blurDelta", Type: \c float. Blur filter size.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    virtual void setPropertyDestination(Identifier dest);

    virtual int initializeGL();

    virtual void loadShader();

    /**
     * If the \c IDRaycaster is not in stand alone mode, it does not free
     * the entry/exit params. This can be used in order to put the
     * \c IDRaycaster in the same pipeline as the image raycaster.
     * default: true
     */
    void setStandAlone(bool standAlone);

    virtual void process(LocalPortMapping* portMapping_);

protected:

    virtual void compile();

    std::string generateHeader() {
        return VolumeRaycaster::generateHeader();
    }

    /// The id raycasting shader.
    tgt::Shader* raycastPrg_;

    /// This shader is used to blur the first-hit-positions.
    tgt::Shader* blurShader_;

    /// Indicates if pipeline is currently in coarseness-mode.
    bool coarse_;

    /// \see setStandAlone()
    bool standAlone_;

    /// If true, the first-hit-positions are blurred.
    BoolProp useBlurring_;

    /// The blur filter size.
    FloatProp blurDelta_;

    /// Indicates how deep a ray penetrates the volume, if no segment is hit.
    FloatProp penetrationDepth_;

    /// Transfer function to use for id-raycasting.
    TransFuncProp transferFunc_;

    static const Identifier firstHitTexUnit_;
    /// Texture unit used to bind the depth channel of the first-hit-positions.
    static const Identifier firstHitDepthTexUnit_;

// !!! NOTE:
// These attributes and methods haven been moved here temporary
// from super class VolumeRenderer in order to provide compatibility
// with old VolumeContainer.
// This functionality is about to be eliminated in near future!
// (Dirk)
//
protected:    
    VolumeContainer* volumeContainer_;
    int currentDataset_;

public:
    /// Returns the dataset currently used.
    VolumeGL* getCurrentDataset() {
        return volumeContainer_ ? volumeContainer_->getVolumeGL(currentDataset_) : 0;
    }
    /// Returns the dataset currently used.
    const VolumeGL* getCurrentDataset() const {
        return volumeContainer_ ? volumeContainer_->getVolumeGL(currentDataset_) : 0;
    }
};

} // namespace voreen

#endif //__VRN_IDRAYCASTING_H__
