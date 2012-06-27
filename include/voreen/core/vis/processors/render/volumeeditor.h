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

#ifndef VRN_VOLUMEEDITOR_H
#define VRN_VOLUMEEDITOR_H

#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "fboClass/fboclass_framebufferobject.h"

namespace voreen {

class CameraProperty;

/**
 * Performs a simple single pass raycasting with only some capabilites.
 */
class VolumeEditor : public VolumeRaycaster {
public:
    /**
     * Constructor.
     */
    VolumeEditor();

    virtual ~VolumeEditor();

    virtual std::string getCategory() const { return "Volume Processing"; }
    virtual std::string getClassName() const { return "VolumeEditor"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual void initialize() throw (VoreenException);

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
    virtual void process();

    virtual void mousePressEvent(tgt::MouseEvent* e);
    virtual void mouseMoveEvent(tgt::MouseEvent* e);
    virtual void mouseReleaseEvent(tgt::MouseEvent* e);
    virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

    bool saveSegmentationDataSet(std::string filename);

protected:
    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);
    virtual void compile(VolumeHandle* volumeHandle);

private:

    VolumeHandle* currentVolumeHandle_;

    CameraProperty camera_;
    IntProperty brushSize_;
    ColorProperty brushColor_;
    StringProperty saveDialogProp_;
    virtual void applyBrush(tgt::ivec2 mousePos);

    bool mouseDown_;

    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function

    bool firstPass_; ///< may need several rendering passes, this stores if this is the first

    FramebufferObject* fbo_;

    VolumePort volumePort_;
    RenderPort entryPort_;
    RenderPort exitPort_;
    RenderPort firstHitpointsPort_;
    RenderPort outport_;
};


} // namespace voreen

#endif // VRN_VOLUMEEDITOR_H
