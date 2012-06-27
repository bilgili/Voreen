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

#include "voreen/qt/widgets/showtexcontainerwidget.h"
#include "tgt/vector.h"

#include <QMouseEvent>
#include <QString>
#include <QMenu>

namespace voreen {

ShowTexContainerWidget::ShowTexContainerWidget(const QGLWidget* shareWidget)
    : QGLWidget(0, shareWidget),
      floatProgram_(0),
      depthProgram_(0),
      tc_(0),
      paintInfos_(true),
      selected_(0),
      fullscreen_(false),
      refreshInterval_(100)
{
	contextMenuMEN_ = new QMenu(this);
	colorBufferACT_ = contextMenuMEN_->addAction("color buffer");
	colorBufferACT_->setCheckable(true);
    alphaChannelACT_ = contextMenuMEN_->addAction("alpha channel");
    alphaChannelACT_->setCheckable(true);
	depthBufferACT_ = contextMenuMEN_->addAction("depth buffer");
	depthBufferACT_->setCheckable(true);
	posXBufferACT_ = contextMenuMEN_->addAction("positive x buffer");
	posXBufferACT_->setCheckable(true);
	negXBufferACT_ = contextMenuMEN_->addAction("negative x buffer");
	negXBufferACT_->setCheckable(true);
	posYBufferACT_ = contextMenuMEN_->addAction("positive y buffer");
	posYBufferACT_->setCheckable(true);
	negYBufferACT_ = contextMenuMEN_->addAction("negative y buffer");
	negYBufferACT_->setCheckable(true);
	posZBufferACT_ = contextMenuMEN_->addAction("positive z buffer");
	posZBufferACT_->setCheckable(true);
	negZBufferACT_ = contextMenuMEN_->addAction("negative z buffer");
	negZBufferACT_->setCheckable(true);

	typeToShowACG_ = new QActionGroup(this);
	typeToShowACG_->addAction(colorBufferACT_);
    typeToShowACG_->addAction(alphaChannelACT_);
	typeToShowACG_->addAction(depthBufferACT_);
	typeToShowACG_->addAction(posXBufferACT_);
	typeToShowACG_->addAction(posXBufferACT_);
	typeToShowACG_->addAction(posYBufferACT_);
	typeToShowACG_->addAction(negYBufferACT_);
	typeToShowACG_->addAction(posZBufferACT_);
	typeToShowACG_->addAction(negZBufferACT_);

	for(unsigned int i=0; i<16; ++i) {
		showType_[i] = 0;
	}

    setWindowTitle(tr("Texture Container"));
}

ShowTexContainerWidget::~ShowTexContainerWidget() {
    killTimer(refreshInterval_);
    if (floatProgram_)
        ShdrMgr.dispose(floatProgram_);
    if (depthProgram_)
        ShdrMgr.dispose(depthProgram_);
}

void ShowTexContainerWidget::setTextureContainer(TextureContainer *tc) {
    tc_ = tc;
}

void ShowTexContainerWidget::mousePressEvent(QMouseEvent * e) {
    if (!fullscreen_) {
        int selected = e->x()*4/width_ + 4* (3-(e->y()*4/height_));
        if (selected_ != selected) {
            selected_ = selected;
            //repaint();
        }
    }
    if (e->button() == Qt::LeftButton) {
        fullscreen_ = ! fullscreen_;
        repaint();
    }
    if (e->button() == Qt::MidButton) {
        repaint();
    }
    if (e->button() == Qt::RightButton) {
        if (selected_ < tc_->getNumAvailable()) {
            int attr = tc_->getAttr(selected_);
            if (!(attr&TextureContainer::VRN_FRAMEBUFFER_CONSTS_MASK)) {
                colorBufferACT_->setEnabled(false);
                depthBufferACT_->setEnabled(false);
                posXBufferACT_->setEnabled(false);
                negXBufferACT_->setEnabled(false);
                posYBufferACT_->setEnabled(false);
                negYBufferACT_->setEnabled(false);
                posZBufferACT_->setEnabled(false);
                negZBufferACT_->setEnabled(false);


                if (attr&TextureContainer::VRN_COLOR_CONSTS_MASK) {
                    if ((attr&TextureContainer::VRN_CUBE_MAP_CONSTS_MASK)) {
                        posXBufferACT_->setEnabled(true);
                        negXBufferACT_->setEnabled(true);
                        posYBufferACT_->setEnabled(true);
                        negYBufferACT_->setEnabled(true);
                        posZBufferACT_->setEnabled(true);
                        negZBufferACT_->setEnabled(true);
                    }
                    else {
                        colorBufferACT_->setEnabled(true);
                        alphaChannelACT_->setEnabled(true);
                    }
                }
                if (attr&TextureContainer::VRN_DEPTH_CONSTS_MASK) {
                    depthBufferACT_->setEnabled(true);
                }
                switch(showType_[selected_]) {
                    case 1:
                        colorBufferACT_->setChecked(true);
                        break;
                    case 2:
                        depthBufferACT_->setChecked(true);
                        break;
                    case 3:
                        posXBufferACT_->setChecked(true);
                        break;
                    case 4:
                        negXBufferACT_->setChecked(true);
                        break;
                    case 5:
                        posYBufferACT_->setChecked(true);
                        break;
                    case 6:
                        negYBufferACT_->setChecked(true);
                        break;
                    case 7:
                        posZBufferACT_->setChecked(true);
                        break;
                    case 8:
                        negZBufferACT_->setChecked(true);
                        break;
                    case 9:
                        alphaChannelACT_->setChecked(true);
                        break;
                }
                contextMenuMEN_->exec(e->globalPos());
                if (colorBufferACT_->isChecked()) {
                    showType_[selected_] = 1;
                }
                if (depthBufferACT_->isChecked()) {
                    showType_[selected_] = 2;
                }
                if (posXBufferACT_->isChecked()) {
                    showType_[selected_] = 3;
                }
                if (negXBufferACT_->isChecked()) {
                    showType_[selected_] = 4;
                }
                if (posYBufferACT_->isChecked()) {
                    showType_[selected_] = 5;
                }
                if (negYBufferACT_->isChecked()) {
                    showType_[selected_] = 6;
                }
                if (posZBufferACT_->isChecked()) {
                    showType_[selected_] = 7;
                }
                if (negZBufferACT_->isChecked()) {
                    showType_[selected_] = 8;
                }
                if (alphaChannelACT_->isChecked()) {
                    showType_[selected_] = 9;
                }
            }
        }
    }
}

void ShowTexContainerWidget::keyPressEvent(QKeyEvent* e) {
    if ((e->key() == Qt::Key_T && (e->modifiers() & Qt::ControlModifier)) || e->key() == Qt::Key_Escape) {
        close();
    }
}

void ShowTexContainerWidget::timerEvent (QTimerEvent* /*event*/) {
    if (isVisible())
        updateGL();
}

void ShowTexContainerWidget::closeEvent(QCloseEvent *event) {
	emit closing(false);
	event->accept();
}

void ShowTexContainerWidget::hideEvent(QHideEvent* e) {
    emit hideSignal();
    QGLWidget::hideEvent(e);
}

void ShowTexContainerWidget::initializeGL() {
    floatProgram_ = ShdrMgr.loadSeparate("", "stc_showfloatcolor.frag", generateHeader(), false);
    depthProgram_ = ShdrMgr.loadSeparate("", "stc_showdepth.frag", generateHeader(), false);
    floatRectProgram_ = 0;
    depthRectProgram_ = 0;

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    startTimer(refreshInterval_);
}

void ShowTexContainerWidget::resizeGL(int width, int height) {
	glViewport(0,0, width, height);
	width_ = width;
	height_ = height;
}

void ShowTexContainerWidget::paintInfos(unsigned int id) {
	QString colorStr;
	QString depthStr;
	QString idStr = QString("%1").arg(id);
	unsigned int attr = tc_->getAttr(id);
    LGL_ERROR;
	if (attr&TextureContainer::VRN_FRAMEBUFFER_CONSTS_MASK) {
		colorStr = depthStr = tc_->getTypeAsString(attr&TextureContainer::VRN_FRAMEBUFFER_CONSTS_MASK).c_str();
	}
	else {
		if (attr&TextureContainer::VRN_COLOR_CONSTS_MASK) {
			colorStr = tc_->getTypeAsString(attr&(TextureContainer::VRN_COLOR_CONSTS_MASK|
                TextureContainer::VRN_CUBE_MAP_CONSTS_MASK)).c_str();
		}
		else
			colorStr = "unknown";
		depthStr = tc_->getTypeAsString(attr&(TextureContainer::VRN_DEPTH_CONSTS_MASK|
            TextureContainer::VRN_TEX_RECT_CONSTS_MASK|TextureContainer::VRN_DEPTH_TEX_CONSTS_MASK)).c_str();
        /*
        if (attr&VRN_DEPTH_TEX_CONSTS_MASK)
            depthStr +="(depth tex)";
        else
            depthStr +="(renderbuffer)";
        */
	}
    glColor4f(0.5f, 0.5f, 0.5f, 1.f);
	LGL_ERROR;
    renderText(-1.0, -0.4, -1.0, QString(tc_->getDebugLabel(id).c_str()));
	renderText(-1.0, -0.6, -1.0, idStr);
	renderText(-1.0, -0.8, -1.0, colorStr);
	renderText(-1.0, -1.0, -1.0, depthStr);
    LGL_ERROR;
}

std::string ShowTexContainerWidget::generateHeader() {
    std::string header = "";
 //   header += "#version 110 \n";
    if (tc_) {
        if ( tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_2D ){
            header += "#define VRN_TEXTURE_2D\n";
        } 
        else if ( tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE ) {
            header += "#define VRN_TEXTURE_RECTANGLE\n";
        }
        if ( tc_->getTextureContainerType() == TextureContainer::VRN_TEXTURE_CONTAINER_FBO) {
            header += "#define VRN_TEXTURE_CONTAINER_FBO\n";
        }
        else if (tc_->getTextureContainerType() == TextureContainer::VRN_TEXTURE_CONTAINER_RTT) {
            header += "#define VRN_TEXTURE_CONTAINER_RTT\n";
        }

    }
    return header;
}

void ShowTexContainerWidget::paint(unsigned int id) {
	glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    if (id>15) return;

	unsigned int attr = tc_->getAttr(id);
	if (showType_[id] == 0) {
		if (attr&TextureContainer::VRN_COLOR_CONSTS_MASK) {
            if ((attr&TextureContainer::VRN_CUBE_MAP_CONSTS_MASK))
                showType_[id] = 3;
            else
                showType_[id] = 1;
		}
		else if (attr&TextureContainer::VRN_DEPTH_CONSTS_MASK) {
			showType_[id] = 2;
		}
	}
	switch(showType_[id]) {
		case 1: // color buffer
        case 9: {
            GLenum texTarget = tc_->getGLTexTarget(id);
			glBindTexture(texTarget, tc_->getGLTexID(id));

            //FIXME: set consistent texture filtering mode here, but restore old state afterwards

            tgt::Shader* prg = floatProgram_;

            if (!prg)
                return;
		    prg->activate();
		    prg->setUniform("tex_", 0);
            LGL_ERROR;

            GLint location = prg->getUniformLocation("screenDim_", true);
            if (location != -1)
                prg->setUniform(location, tgt::vec2(tc_->getSize()));
            location = prg->getUniformLocation("screenDimRCP_", true);
            if (location != -1)
                prg->setUniform(location, 1.f / tgt::vec2(tc_->getSize()));

            LGL_ERROR;
			if (showType_[id] == 9)
                prg->setUniform("showAlpha_", true);
            else
                prg->setUniform("showAlpha_", false);
            LGL_ERROR;

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
			glTexCoord3f(0, 0, 0); glVertex3f(-1, -1, -0.5f);
			glTexCoord3f(1, 0, 0); glVertex3f(1, -1, -0.5f);
			glTexCoord3f(1, 1, 0); glVertex3f(1,  1, -0.5f);
			glTexCoord3f(0, 1, 0); glVertex3f(-1,  1, -0.5f);
			glEnd();
			prg->deactivate();
			break;
        }
        case 2: {// depth buffer
            GLenum texTarget = tc_->getGLTexTarget(id);
			glBindTexture(texTarget,tc_->getGLDepthTexID(id));
            tgt::Shader* prg = depthProgram_;

			prg->activate();
			prg->setUniform("tex_", 0);
            prg->setUniform("screenDim_", static_cast<tgt::vec2>(tc_->getSize()));
            LGL_ERROR;
			prg->setUniform("screenDimRCP_", 1.f/static_cast<tgt::vec2>(tc_->getSize()));
            LGL_ERROR;
            
            glDisable(GL_DEPTH);
            glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
			glTexCoord3f(0, 0, 0); glVertex3f(-1, -1, -0.5f);
			glTexCoord3f(1, 0, 0); glVertex3f(1, -1, -0.5f);
			glTexCoord3f(1, 1, 0); glVertex3f(1,  1, -0.5f);
			glTexCoord3f(0, 1, 0); glVertex3f(-1,  1, -0.5f);
			glEnd();
			prg->deactivate();
			break;
        }
		case 3:; // cube map pos x
		case 4:; // cube map neg x
		case 5:; // cube map pos y
		case 6:; // cube map neg y
		case 7:; // cube map pos z
		case 8: // cube map neg z
			glBindTexture(tc_->getGLTexTarget(id), tc_->getGLTexID(id));
			break;
		default:; // do nothing
	}
    LGL_ERROR;

    glDisable(GL_TEXTURE_2D);

    paintInfos(id);
    LGL_ERROR;
}

void ShowTexContainerWidget::paintGL() {
    glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if ( !tc_ )
        return;

    /*
    tc_->setActiveTarget(0);
        //glViewport(0,0,400,400);
        glClearColor(0,1,0,0);
        glClearDepth(1.0);
        glEnable(GL_DEPTH);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glColor3f(1,0,0);
        glBegin(GL_QUADS);
		    glVertex3f(-0.8, -0.8, -1.0f);
		    glVertex3f(0.8, -0.8, -1.0f);
		    glVertex3f(0.8,  0.8, 1.0f);
		    glVertex3f(-0.8,  0.8, 1.0);
		glEnd();
    tc_->setActiveTarget(20);
    */

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	/*
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	*/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

    if (fullscreen_) {
        paint(selected_);
    } else {
		for(int i=0; i<tc_->getNumAvailable(); ++i) {
			glPushMatrix();
			glTranslatef(-0.75+0.5*(i%4), -0.75+0.5*(i/4), 0.0);
			glScalef(0.25, 0.25, 1.0);
            paint(i);
			glPopMatrix();
		}
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	/*
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	*/
	glMatrixMode(GL_MODELVIEW);
}

//---------------------------------------------------------------------------

ShowTexture::ShowTexture(QWidget *parent, bool shareContext) :
tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, parent, shareContext), tex_(0) {
    fragProgram_ = ShdrMgr.load("stc_showtexture", generateHeader(), false);
    startTimer(500);
}

void ShowTexture::setTexture(GLuint tex, tgt::ivec2 size/*=tgt::ivec2(0,0)*/) {
	tex_ = tex;
    size_ = size;
}

void ShowTexture::timerEvent(QTimerEvent* /*event*/) {
	updateGL();
}

void ShowTexture::initializeGL() {
}

void ShowTexture::resizeGL(int width, int height) {
	glViewport(0,0, width, height);
}

void ShowTexture::paintGL() {
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_);
    fragProgram_->activate();
    fragProgram_->setUniform("tex_", 0);
    fragProgram_->setUniform("texSize_", size_);
	glBegin(GL_QUADS);
	glTexCoord3f(0, 0, 0); glVertex3f(-1, -1, -0.5f);
	glTexCoord3f(1, 0, 0); glVertex3f(1, -1, -0.5f);
	glTexCoord3f(1, 1, 0); glVertex3f(1,  1, -0.5f);
	glTexCoord3f(0, 1, 0); glVertex3f(-1,  1, -0.5f);
	glEnd();
    fragProgram_->deactivate();
    glDisable(GL_TEXTURE_2D);
}

std::string ShowTexture::generateHeader() {
    std::string header = "";
 //   header += "#version 110 \n";
    /*if (tc_) {
        if ( tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_2D ){
            header += "#define VRN_TEXTURE_2D\n";
        } else if ( tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE ) {
            header += "#define VRN_TEXTURE_RECTANGLE\n";
        }
    } */
    return header;
}

} // namespace voreen
