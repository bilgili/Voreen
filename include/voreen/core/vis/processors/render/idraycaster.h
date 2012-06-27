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

#ifndef VRN_IDRAYCASTER_H
#define VRN_IDRAYCASTER_H

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"

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
    virtual Processor* create() const {return new IDRaycaster();}

    virtual int initializeGL();

    virtual void loadShader();

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

    /// Indicates how deep a ray penetrates the volume, if no segment is hit.
    FloatProp penetrationDepth_;

    static const Identifier firstHitPointsTexUnit_;
    static const Identifier firstHitPointsDepthTexUnit_;

};

} // namespace voreen

#endif //__VRN_IDRAYCASTING_H__
