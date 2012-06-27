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

#include "voreen/core/vis/transfunc/transfuncpainter.h"
#include "tgt/glmath.h"
#include "tgt/shadermanager.h"
#include "voreen/core/volume/gradient.h"
#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

using namespace tgt;

namespace voreen {

TransFuncPainter::TransFuncPainter(tgt::GLCanvas* cv, TransFuncEditor* tfe)
    : tgt::Painter(cv)
    , histogramTex_(0)
    , selectedPrimitive_(0)
    , showHistogram_(false)
    , showGrid_(false)
    , curDataset_(0)
    , curHist_(0)
    , tfe_(tfe)
    , histLog_(true)
    , histBright_(1.0)
{
    tf_ = 0;
    thres_l = 0.0;
    thres_u = 1.0;
    
    grabbedPrimitive_ = 0;
    changed_ = true;
}

TransFuncPainter::~TransFuncPainter() {
    if (tf_)
        delete tf_;
}

void TransFuncPainter::paint() {
	tgt::Shader::deactivate();
// 	for(int i=0; i<16; i++) {
// 			glActiveTexture(GL_TEXTURE0+i);
// 			glDisable(GL_TEXTURE_1D);
// 			glDisable(GL_TEXTURE_2D);
// 			glDisable(GL_TEXTURE_3D);
// 		}
	glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    if(changed_)
        updateTF();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

//     glTranslatef(-0.5f,-0.5f,-2.0f);
    if((showHistogram_) && (histogramTex_)) {
        histogramTex_->enable();
        histogramTex_->bind();
        glBegin(GL_QUADS);
            // Front Face
            glColor3f(1.0,1.0,1.0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.0f, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,  1.0f,  -0.5f);  // Top Left Of The Texture and Quad
        glEnd();
        histogramTex_->disable();
    }
    if(showGrid_) {
        glBegin(GL_LINES);
        glColor3f(0.7,0.7,0.7);
        for(int i=0; i<10; ++i) {
            glVertex3f( 0.0f, i*0.1f,  -0.5f);
            glVertex3f( 1.0f, i*0.1f,  -0.5f);

            glVertex3f( i*0.1f,  0.0f,  -0.5f);
            glVertex3f( i*0.1f,  1.0f,  -0.5f);
        }
        glEnd();
    }

    //lower threshold:
    glBegin(GL_QUADS);
            glColor4f(0.7,0.7,0.7,0.5);

            glVertex3f( 0.0f, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glVertex3f( thres_l, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glVertex3f( thres_l,  1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glVertex3f(0.0f,  1.0f,  -0.5f);  // Top Left Of The Texture and Quad
    glEnd();

    //upper threshold:
    glBegin(GL_QUADS);
            glColor4f(0.7,0.7,0.7,0.5);

            glVertex3f( thres_u, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glVertex3f( 1.0, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glVertex3f( 1.0,  1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glVertex3f(thres_u,  1.0f,  -0.5f);  // Top Left Of The Texture and Quad
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glDisable(GL_TEXTURE_2D);
    tf_->paintInEditor();

    getCanvas()->swap();
    glDisable(GL_BLEND);

    if(changed_)
        tfe_->transFuncChanged();
    changed_ = false;
}

void TransFuncPainter::initialize() {
	getCanvas()->getGLFocus();
    glGetIntegerv(GL_DRAW_BUFFER, &currentDrawbuf_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,1.0,0.0,1.0f,-2.0f,1.0f);
//     gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glewInit();

    tf_ = new TransFuncIntensityGradientPrimitiveContainer(256);
    if ( !tf_->initOk() ) {
        delete tf_;
        tf_ = 0;
    }
    else {
        glDrawBuffer(currentDrawbuf_);
    }
}

void TransFuncPainter::updateTF() {
    glGetIntegerv(GL_DRAW_BUFFER, &currentDrawbuf_); // Save the current Draw buffer
  
    tf_->updateTexture();
    
    glDrawBuffer(currentDrawbuf_);
    glViewport(0, 0, getCanvas()->getWidth(), getCanvas()->getHeight());
}

void TransFuncPainter::sizeChanged(const tgt::ivec2& size) {
    glViewport(0, 0, size.x, size.y);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,1.0f,0.0f,1.0f,-2.0f,1.0f);
//     gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TransFuncPainter::mousePress(tgt::vec2 m) {
    grabbedPrimitive_ = getPrimitiveUnderMouse(m);
    select(grabbedPrimitive_);
    paint();
    mouseCoord_ = m;
	if(grabbedPrimitive_)
		tfe_->setInteractionCoarseness(true);
}

void TransFuncPainter::mouseRelease(tgt::vec2 m) {
    if(grabbedPrimitive_) {
        grabbedPrimitive_->mouseRelease(m);
        grabbedPrimitive_ = 0;
        changed();
    }
	tfe_->setInteractionCoarseness(false);
}

void TransFuncPainter::moveMouse(tgt::vec2 m) {
    tgt::vec2 offset = m - mouseCoord_;
    mouseCoord_ = m;
    if(grabbedPrimitive_) {
        grabbedPrimitive_->mouseDrag(offset);
        changed();
    }
}

void TransFuncPainter::mouseDoubleclick(tgt::vec2 m) {
    grabbedPrimitive_ = getPrimitiveUnderMouse(m);
    if(grabbedPrimitive_) {
//        bool ch;
        select(grabbedPrimitive_);
        paint();
//        tgt::col4 newcol = tfe_->colorChooser(grabbedPrimitive_->getColor(), ch);
//        if(ch)
//            selectedPrimitive_->setColor(newcol);
        changed();
		grabbedPrimitive_ = 0;
    }
}

void TransFuncPainter::deletePrimitive() {
    if(selectedPrimitive_) {
        TransFuncPrimitive* p = selectedPrimitive_;
        select(0);
        tf_->removePrimitive(p);
        grabbedPrimitive_ = 0;
    }
    changed();
}

void TransFuncPainter::mouseWheelUp(tgt::vec2 m) {
    TransFuncPrimitive* p = getPrimitiveUnderMouse(m);
    if(p) {
        tgt::col4 c = p->getColor();
        if((c.a + 10) > 255)
            c.a = 255;
        else
            c.a += 10;
        p->setColor(c);
        changed();
    }
}

void TransFuncPainter::mouseWheelDown(tgt::vec2 m) {
    TransFuncPrimitive* p = getPrimitiveUnderMouse(m);
    if(p) {
        tgt::col4 c = p->getColor();
        if((c.a - 10) < 0)
            c.a = 0;
        else
            c.a -= 10;
        p->setColor(c);
        changed();
    }
}

TransFuncPrimitive* TransFuncPainter::getPrimitiveUnderMouse(tgt::vec2 m) {
    TransFuncPrimitive* um = tf_->getControlPointUnderMouse(mouseCoord_);
    if((um) && (!um->mousePress(m))) {
        um = 0;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glDisable(GL_BLEND);
        tf_->paintSelection();
        glEnable(GL_BLEND);
        GLubyte pixel[3];
        glReadPixels((int)(getCanvas()->getWidth()*m.x), (int)(getCanvas()->getHeight()*m.y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel );

//         LWARNINGC("test", "pixel: " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2]);
        if((pixel[1] == 123) && (pixel[2] == 123))
            um = tf_->getPrimitive(pixel[0]);
    }
    return um;
}

void TransFuncPainter::select(TransFuncPrimitive* p) {
    if(selectedPrimitive_)
        selectedPrimitive_->setSelected(false);
    if(p) {
        p->setSelected(true);
    }
    selectedPrimitive_ = p;

    if(p)
        tfe_->selected();
    else
        tfe_->deselected();
}


void TransFuncPainter::dataSourceChanged(Volume* newDataset) {
    curDataset_ = newDataset;
    showHistogram_ = false;
    if(histogramTex_)
        delete histogramTex_;
    histogramTex_ = 0;
    if(curHist_)
        delete curHist_;
    curHist_ = 0;
    changed();
}

void TransFuncPainter::setHistogramVisible(bool v) {
    showHistogram_ = v;
    if( v && (!histogramTex_) && (curDataset_) ) {
        //histogram is enabled but no texture is available
        //=> calculate one from current dataset.
        Volume* sourceDataset = 0;
        Volume* intensityDataset = 0;

        if(curDataset_->getBitsStored() != 32) {
//              sourceDataset = calcGradients<col3>(curDataset_);
            sourceDataset = calcGradientsSobel<col3>(curDataset_, false);
            intensityDataset = curDataset_;
        } else
            sourceDataset = (Volume3xUInt8*) curDataset_;

        int bucketsi = 256;
//         if(curDataset_->getBitsStored() == 16)
//             bucketsi = 512;
        int bucketsg = 256;
        histogramTex_ = new Texture(tgt::ivec3(bucketsi,bucketsg,1), GL_LUMINANCE, GL_LUMINANCE,
                                    GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);

        if(curHist_)
            delete curHist_;
        curHist_ = new HistogramIntensityGradient(sourceDataset, intensityDataset, bucketsi, bucketsg);
        updateHistogramTex();

        if(curDataset_->getBitsStored() != 32)
            delete sourceDataset;
    }
}

void TransFuncPainter::updateHistogramTex() {
    if(!histogramTex_)
        return;
    for(int i=0; i<histogramTex_->getHeight(); i++) {
        for(int j=0; j<histogramTex_->getWidth(); j++) {
            int o;
            if(histLog_)
                o = (int)(curHist_->getLogNormalized(j,i)*255.0/**2550000.0*/*histBright_);
            else
                o = (int)(curHist_->getNormalized(j,i)*2550000.0*histBright_);
            if(o>255)
                o = 255;
            else if (o<0)
                o = 0;
            histogramTex_->texel<uint8_t>(j,i) = (GLubyte)o;
        }
    }
    histogramTex_->uploadTexture();
}


void TransFuncPainter::setThresholds(float l, float u) {
    thres_l = l;
    thres_u = u;
}

void TransFuncPainter::clear() {
    grabbedPrimitive_ = 0;
    selectedPrimitive_ = 0;
    tf_->clear();
	changed();
}

void TransFuncPainter::setTransFunc(TransFuncIntensityGradientPrimitiveContainer* tf) {
    grabbedPrimitive_ = 0;
    selectedPrimitive_ = 0;
    tf_ = tf;
}

void TransFuncPainter::setHistogramLog(bool l) {
    histLog_ = l;
}

void TransFuncPainter::setHistogramBrightness(float b) {
    histBright_ = b;
}

//
// TransferEditorListener
//

void TransFuncEditorListener::keyEvent(tgt::KeyEvent* e) {
    if(e->pressed()) {
//             switch(e->keyCode())                      // which key have we got
//             {
//                 case tgt::KeyEvent::K_F1 :                      // if it is F1
//     //                 getCanvas()->toggleFullScreen();    // toggle between fullscreen
//                     break;
//                     default:                                           // any other key
//                         break;                                         // nothing to do
//             }
    }
    else {
        switch(e->keyCode())                      // which key have we got
        {
//             case tgt::KeyEvent::K_S :
//                 painter_->getTransFunc()->save("test2.xml");
//                 break;
            case tgt::KeyEvent::K_Q :
                painter_->getTransFunc()->addPrimitive(new TransFuncQuad(tgt::vec2(0.6,0.6), 0.3, tgt::col4(255,0,0,128)));
                break;
            case tgt::KeyEvent::K_B :
                painter_->getTransFunc()->addPrimitive(new TransFuncBanana(tgt::vec2(0.0,0.0), tgt::vec2(0.5,0.6), tgt::vec2(0.34,0.4), tgt::vec2(0.45,0.0),  tgt::col4(0,255,0,128)));
                break;
            case tgt::KeyEvent::K_DELETE :
                painter_->deletePrimitive();
                break;
            default:                                           // any other key
                break;                                        // nothing to do
        }
    }
}

void TransFuncEditorListener::mouseMoveEvent(tgt::MouseEvent* e) {
    painter_->moveMouse(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-((float)e->y()/painter_->getCanvas()->getHeight())));
    painter_->getCanvas()->repaint();
}

void TransFuncEditorListener::mousePressEvent(tgt::MouseEvent* e) {
    painter_->getCanvas()->getGLFocus();
    painter_->mousePress(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-((float)e->y()/painter_->getCanvas()->getHeight())));
    painter_->getCanvas()->repaint();
}

void TransFuncEditorListener::mouseReleaseEvent(tgt::MouseEvent* e) {
    painter_->mouseRelease(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-((float)e->y()/painter_->getCanvas()->getHeight())));
}

void TransFuncEditorListener::mouseDoubleClickEvent(tgt::MouseEvent* e) {
    painter_->getCanvas()->getGLFocus();
    painter_->mouseDoubleclick(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-((float)e->y()/painter_->getCanvas()->getHeight())));
    painter_->getCanvas()->repaint();
}

void TransFuncEditorListener::wheelEvent(tgt::MouseEvent* e) {
    painter_->getCanvas()->getGLFocus();
    if(e->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN)
        painter_->mouseWheelDown(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-(static_cast<float>(e->y())/painter_->getCanvas()->getHeight())));
    else
        painter_->mouseWheelUp(tgt::vec2((float)e->x()/painter_->getCanvas()->getWidth(), 1.0f-(static_cast<float>(e->y())/painter_->getCanvas()->getHeight())));
    painter_->getCanvas()->repaint();
}

} // namespace
