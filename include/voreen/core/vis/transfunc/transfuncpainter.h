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

#ifndef VRN_TRANSFUNCPAINTER_H
#define VRN_TRANSFUNCPAINTER_H

#include "tgt/vector.h"
#include "tgt/painter.h"
#include "tgt/glcanvas.h"
#include "tgt/texturemanager.h"
#include "tgt/event/keyevent.h"
#include "tgt/tgt_gl.h"
/*
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/histogram.h"
#include "voreen/core/vis/transfunc/transfunceditor.h"
*/

namespace voreen {

class TransFuncIntensityGradientPrimitiveContainer;
class TransFuncPrimitive;
class Volume;
class HistogramIntensityGradient;
class TransFuncEditor;

/**
 *  Painter implementation for drawing a control widget for TransFuncIntensityGradient.
 */
class TransFuncPainter : public tgt::Painter {
    TransFuncIntensityGradientPrimitiveContainer* tf_;
    tgt::Texture* histogramTex_;
    tgt::vec2 mouseCoord_;
    TransFuncPrimitive* grabbedPrimitive_;
    TransFuncPrimitive* selectedPrimitive_;
    bool showHistogram_;
    bool showGrid_;
    Volume* curDataset_;
    HistogramIntensityGradient* curHist_;
    float thres_l, thres_u;
    TransFuncEditor* tfe_;
    bool changed_;
    bool histLog_;
    float histBright_;

public:
    TransFuncPainter(tgt::GLCanvas* cv, TransFuncEditor* tfe);
    ~TransFuncPainter();

    void paint();
    ///Render the TF in a texture
    void updateTF();
    void initialize();
    void moveMouse(tgt::vec2 m);
    void mousePress(tgt::vec2 m);
    void mouseRelease(tgt::vec2 m);
    void mouseDoubleclick(tgt::vec2 m);
    void mouseWheelUp(tgt::vec2 m);
    void mouseWheelDown(tgt::vec2 m);
    void deletePrimitive();
	void sizeChanged(const tgt::ivec2 &size);
    TransFuncIntensityGradientPrimitiveContainer* getTransFunc() { return tf_; }
    void select(TransFuncPrimitive* p);
    TransFuncPrimitive* getSelectedPrimitive() { return selectedPrimitive_; }
    void changed() { changed_ = true; }

    ///Draw Intensity-Gradient histogram in background?
    bool histogramVisible() { return showHistogram_; }
    void setHistogramVisible(bool v);
    void updateHistogramTex();
    ///Use logarithmic histogram?
    void setHistogramLog(bool l);
    void setHistogramBrightness(float b);
    
    void dataSourceChanged(Volume* newDataset);
    ///Remove all primitives from TF
    void clear();

    bool gridVisible() { return showGrid_; }
    ///Draw 10x10 grid in background?
    void setGridVisible(bool v) { showGrid_ = v; }
    
    void setThresholds(float l, float u);
    ///Check if init was completed without errors (FBO!)
    bool initOk();

    void setTransFunc(TransFuncIntensityGradientPrimitiveContainer* tf);
protected:
    TransFuncPrimitive* getPrimitiveUnderMouse(tgt::vec2 m);

    GLint currentDrawbuf_;
};

///EventListener implementation for TransferPainter
class TransFuncEditorListener : public tgt::EventListener {
    TransFuncPainter* painter_;
public:
    TransFuncEditorListener(TransFuncPainter* p) { painter_ = p; }

    void keyEvent(tgt::KeyEvent* e);
    void mouseMoveEvent(tgt::MouseEvent* e);
    void mousePressEvent(tgt::MouseEvent* e);
    void mouseReleaseEvent(tgt::MouseEvent* e);
    void mouseDoubleClickEvent(tgt::MouseEvent* e);
    void wheelEvent(tgt::MouseEvent* e);
};

}
#endif //VRN_TRANSFUNCPAINTER_H
