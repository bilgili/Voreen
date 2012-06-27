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

#ifndef VRN_THRESHOLD_H
#define VRN_THRESHOLD_H

//TODO: Check if all of this is needed
#include <vector>
#include <string>

#include "tgt/shadermanager.h"
#include "tgt/quadric.h"
#include "tgt/types.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/genericfragment.h"

namespace voreen {

/**
 * Performs a thresholding.
 *
 * The pixel color is used, when the surrounding pixel exceed a defined
 * threshold is exceeded and black otherwise.
 *
 * It's probably a slow filter because an if instruction is used internally.
 */
class Threshold : public GenericFragment {
public:
    /**
     * The Constructor.
     *
     * @param camera The camera from wich we will get information about the current modelview-matrix.
     * @param tc The TextureContainer that will be used to manage TextureUnits for all render-to-texture work done by the PostProcessing.
     */
    Threshold();
    virtual const Identifier getClassName() const {return "PostProcessor.Threshold";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new Threshold();}

    void process(LocalPortMapping* portmapping);

    /**
     * Sets the threshold to be used.
     *
     * @param threshold The threshold.
     */
    void setThreshold(float threshold);

    /**
     * Sets the delta parameter
     *
     * @param delta
     */
    void setDelta(float delta);

    /**
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - set.thresholdPPdelta, which is used to set the parameter delta, Msg-Type: float
     *      - set.thresholdPPthreshold, which is used to set \a threshold_ , Msg-Type: float
     *
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

protected:
    FloatProp threshold_;
    FloatProp delta_;
};


} // namespace voreen

#endif //VRN_THRESHOLD_H
