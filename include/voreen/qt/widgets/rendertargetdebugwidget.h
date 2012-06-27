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

#ifndef VRN_RENDERTARGETDEBUGWIDGET_H
#define VRN_RENDERTARGETDEBUGWIDGET_H

#include <QAction>

#include "voreen/core/vis/network/networkevaluator.h"
#include "tgt/qt/qtcanvas.h"

#include <QtOpenGL/QGLWidget>

#include "tgt/shadermanager.h"
#include "tgt/qt/qtcanvas.h"

namespace voreen {

class RenderTarget;
/**
 * Show currently used RenderTargets for debugging purposes.
 */
class RenderTargetDebugWidget : public QGLWidget, public NetworkEvaluator::ProcessWrapper {
  Q_OBJECT
public:
    RenderTargetDebugWidget(const QGLWidget* shareWidget);
    ~RenderTargetDebugWidget();

    void setEvaluator(NetworkEvaluator* evaluator);

    /// Update after network has been processed.
    void afterNetworkProcess();

signals:
    void closing(bool);
    void hideSignal();

protected:
    QString internalTypeToString(GLint internalType);

    void mousePressEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);

    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();
    void paint(RenderTarget* rt, int id);
    void paintInfos(RenderTarget* rt);

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
    QAction *showInfosACT_;

    NetworkEvaluator* evaluator_;
    bool paintInfos_;
    int width_;
    int height_;
    size_t selected_;
    bool fullscreen_;

    int showType_[16];
};

} // namespace voreen

#endif //VRN_RENDERTARGETDEBUGWIDGET_H
