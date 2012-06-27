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

#ifndef VRN_SHOWTEXCONTAINERWIDGET_H
#define VRN_SHOWTEXCONTAINERWIDGET_H

#include <QAction>

#include "voreen/core/opengl/texturecontainer.h"
#include "tgt/qt/qtcanvas.h"

#include <QtOpenGL/QGLWidget>

#include "tgt/shadermanager.h"
#include "tgt/qt/qtcanvas.h"

namespace voreen {

/**
 * Show the contents of a TextureContainer.
 *
 * FIXME: Use of texture unit 0 is hard coded.
 * FIXME: only 16 textures can be shown.
 */
class ShowTexContainerWidget : public QGLWidget {
  Q_OBJECT
public:
	ShowTexContainerWidget(const QGLWidget* shareWidget);
	~ShowTexContainerWidget();
    
	void setTextureContainer(TextureContainer *tc);
signals:
    void closing(bool);
    void hideSignal();

protected:
	void mousePressEvent(QMouseEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void timerEvent(QTimerEvent* event);
    void closeEvent(QCloseEvent* event);
    void hideEvent(QHideEvent* e);

    virtual void initializeGL (); 
	virtual void resizeGL(int width, int height); 
	virtual void paintGL(); 
	void paint(unsigned int id);
	void paintInfos(unsigned int id);
	
    /**
     * Generates the appropriate header for the used shaders.
     */
    virtual std::string generateHeader();

	tgt::Shader* floatProgram_;
	tgt::Shader* depthProgram_;
    tgt::Shader* floatRectProgram_;
	tgt::Shader* depthRectProgram_;

	QMenu *contextMenuMEN_;
	QActionGroup *typeToShowACG_;
	QAction *colorBufferACT_;
    QAction *alphaChannelACT_;
	QAction *depthBufferACT_;
	QAction *posXBufferACT_;
	QAction *negXBufferACT_;
	QAction *posYBufferACT_;
	QAction *negYBufferACT_;
	QAction *posZBufferACT_;
	QAction *negZBufferACT_;

	TextureContainer *tc_;
	bool paintInfos_;
	int width_;
	int height_;
	int selected_;
	bool fullscreen_;
	int refreshInterval_;

	int showType_[16];
};

//---------------------------------------------------------------------------

class ShowTexture : public tgt::QtCanvas {
public:
	ShowTexture(QWidget *parent=0, bool shareContext = true);
    void setTexture(GLuint tex, tgt::ivec2 size=tgt::ivec2(0,0));
protected:
	void timerEvent ( QTimerEvent * event );

	virtual void initializeGL (); 
	virtual void resizeGL ( int width, int height ); 
	virtual void paintGL (); 
    /**
     * Generates the appropriate header for the used shaders.
     */
    virtual std::string generateHeader();

	tgt::Shader* fragProgram_;
	GLuint tex_;
    tgt::ivec2 size_;
};

} // namespace voreen

#endif //VRN_SHOWTEXCONTAINERWIDGET_H
