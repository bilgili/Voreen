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

#ifndef VRN_IMAGESELECTOR_H
#define VRN_IMAGESELECTOR_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/interaction/mwheelnumpropinteractionhandler.h"

namespace voreen {

/**
 * Selects a single volume out of an input image sequence.
 */
class ImageSelector : public RenderProcessor {

public:
    ImageSelector();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "ImageSelector";   }
    virtual std::string getCategory() const   { return "Utility";         }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE; }
    virtual bool isUtility() const            { return true; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    virtual void invalidate(int inv = INVALID_RESULT);

    ImageSequencePort inport_;   ///< Inport for the image sequence.
    RenderPort outport_;         ///< The render port the selected image is written to.

    IntProperty imageID_;        ///< id of the selected image
    MWheelNumPropInteractionHandler<int> wheelHandler_; ///< used for cycling through images by mouse wheel

    tgt::Shader* shader_;        ///< Shader program used for rendering the selected image to the outport.

    static const std::string loggerCat_;

private:
    void adjustToImageSequence();

};

} // namespace

#endif
