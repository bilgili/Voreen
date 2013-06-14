/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_RENDERTARGETVIEWER_H
#define VRN_RENDERTARGETVIEWER_H

#include <vector>

#include "tgt/qt/qtcanvas.h"
#include "tgt/shadermanager.h"
#include "tgt/font.h"
#include "tgt/framebufferobject.h"

#include <QAction>
#include <QtOpenGL/QGLWidget>

#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/qt/voreenqtapi.h"

namespace voreen {

class RenderTarget;

/**
 * Show currently used RenderTargets for debugging purposes.
 */
class VRN_QT_API RenderTargetViewer : public QGLWidget, public NetworkEvaluatorObserver {
Q_OBJECT
public:
    enum ShowType {
        R                   = 0x0001,
        G                   = 0x0002,
        B                   = 0x0004,
        A                   = 0x0008,
        Color               = 0x0010,
        Alpha               = 0x0020,
        Depth               = 0x0040,
        CheckerboardPattern = 0x0080,
        BackgroundWhite     = 0x0100,
        BackgroundBlack     = 0x0200,
        H                   = 0x0400,
        S                   = 0x0800,
        V                   = 0x1000
    };

    RenderTargetViewer(const QGLWidget* sharedContext);
public:
    ~RenderTargetViewer();

    void setEvaluator(NetworkEvaluator* evaluator);

    /// Update after network has been processed.
    void afterNetworkProcess();

    // frees OGL resources, called by mainwindow before destruction
    void deinit();

public slots:
    void processorsSelected(const QList<Processor*>& processors);

signals:
    void closing(bool);
    void hideSignal();

protected:

    struct CompProcID {
        inline bool operator() (const Port* p1, const Port* p2) {
            return (p1->getProcessor()->getID() < p2->getProcessor()->getID());
        }
    };

    QString internalTypeToString(GLint internalType);

    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);

    void updateMousePosition(QMouseEvent* e);

    virtual void initializeGL();
    void initializeFbo();

    virtual void resizeGL(int width, int height);
    virtual void paintGL();
    void updateSelected();
    void takeScreenshot(bool withOverlay);
    void paintPort(RenderPort* rp, int index);
    void renderTargetToTexture(RenderTarget* rt, unsigned int showType, tgt::Texture* texture);
    void renderInfosToFontTexture(RenderTarget* rt);
    void renderFont(float x, float y, QString text);
    void renderQuad();
    void paintCombinedTextures();
    void paintOutline();
    void openSubWindow();
    void setSubWidget(int subWidgetFullscreenIndex, int showType);
    tgt::vec3 rgbToHsv(tgt::vec3 rgb);
    void showHelp();
    std::vector<RenderPort*> collectRenderPorts();

    tgt::Shader* colorProgram_;
    tgt::Shader* inversecolorProgram_;

    QMenu* contextMenuMEN_;
    QActionGroup* typeToShowACG_;
    QActionGroup* backgroundToShowACG_;
    QAction* backgroundBlackACT_;
    QAction* backgroundWhiteACT_;
    QAction* backgroundCheckerboardPatternACT_;
    QAction* colorRACT_;
    QAction* colorGACT_;
    QAction* colorBACT_;
    QAction* colorAACT_;
    QAction* colorRGBAACT_;
    QAction* alphaOnlyACT_;
    QAction* depthOnlyACT_;
    QAction* hOnlyACT_;
    QAction* sOnlyACT_;
    QAction* vOnlyACT_;
    QAction* keepAspectRatioACT_;
    QAction* showInfosACT_;
    QAction* showInfosDetailsACT_;
    QAction* saveScreenshotACT_;
    QAction* saveScreenshotWithOverlayACT_;
    QAction* openInNewWindowACT_;
    QAction* showHelpACT_;
    QAction* filterPortyBySelectedProcessorsACT_;

    NetworkEvaluator* evaluator_;
    bool paintInfos_;
    int selected_;
    bool fullscreen_;

    int mouseX_;
    int mouseY_;

    int dimX_;
    int dimY_;

    int scaledWidth_;
    int scaledHeight_;

    int currentWidth_;
    int currentHeight_;

    std::map<int, unsigned int> showType_;

    bool initialized_;

    GLfloat currentDepthBuffer_;
    tgt::vec4 currentTexel_;
    float currentValueScale_;

    float zoomScale_;
    float zoomTranslateX_;
    float zoomTranslateY_;
    float zoomMouseX_;
    float zoomMouseY_;
    float zoomOffsetX_;
    float zoomOffsetY_;
    bool isPanning_;

    bool freezeInfos_;
    bool mouseIsInside_;

    tgt::FramebufferObject* fbo_;
    tgt::Texture* colorTex_;
    tgt::Texture* fontTex_;

    const QGLWidget* sharedContext_;

    bool isSubWidget_;

    GLfloat minDepth_;
    GLfloat maxDepth_;

    QList<Processor*> selectedProcessors_;

#ifdef VRN_MODULE_FONTRENDERING
    tgt::Font* font_;
#else
    QFont font_;
#endif

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_RENDERTARGETVIEWER_H
