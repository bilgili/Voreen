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

#ifndef VRN_TEXTOVERLAY_H
#define VRN_TEXTOVERLAY_H

#include "voreen/core/processors/imageprocessor.h"
#include "voreen/core/ports/textport.h"
#include "voreen/core/properties/optionproperty.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/font.h"
#include "tgt/framebufferobject.h"

namespace voreen {

/**
 * Overlays text on top of the input image.
 */
class TextOverlay : public ImageProcessor {
public:
    TextOverlay();
    ~TextOverlay();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "TextOverlay";     }
    virtual std::string getCategory() const  { return "Utility";         }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual bool isUtility() const           { return true; }
    virtual std::string getProcessorInfo() const;

    /**
     * Returns true, if the processor has been initialized
     * and the RenderPors are connected.
     */
    virtual bool isReady() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    /**
     * Render text-overlay over scene
     */
    void renderOverlay();

    /**
     * Collect text to overlay from text-inports
     */
    std::string collectText(std::string key);

    int getNumberOfLines(std::string s);

    RenderPort inport_;
    RenderPort outport_;
    RenderPort privatePort_;

    TextPort text0_;
    TextPort text1_;
    TextPort text2_;
    TextPort text3_;

    StringOptionProperty layout0_;
    StringOptionProperty layout1_;
    StringOptionProperty layout2_;
    StringOptionProperty layout3_;

    tgt::Font* font_;

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace

#endif // VRN_TEXTOVERLAY_H
