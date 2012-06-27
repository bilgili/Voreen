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

#ifndef ORIENTATIONPLUGIN_H
#define ORIENTATIONPLUGIN_H

#include "widgetplugin.h"

#include "tgt/quaternion.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/texture.h"

#include <QLabel>
#include <QComboBox>
#include <QBasicTimer>
#include <QPushButton>
#include <QMenu>
#include <QCheckBox>
#include <QToolButton>
#include <QPushButton>
#include <QGroupBox>

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

class OrientationPlugin;

class SchematicOverlayObject {
public:
    enum Faces { Front, Back, Top, Bottom, Left, Right };

    SchematicOverlayObject();
    ~SchematicOverlayObject();

    void renderCube();
    void renderCubeToBuffer();

    void setCubeColorFront(tgt::Color color);
    void setCubeColorBack(tgt::Color color);
    void setCubeColorLeft(tgt::Color color);
    void setCubeColorRight(tgt::Color color);
    void setCubeColorTop(tgt::Color color);
    void setCubeColorDown(tgt::Color color);
    void setCubeSize(float cubeSizeObject);

    tgt::Color getCubeColorFront();
    tgt::Color getCubeColorBack();
    tgt::Color getCubeColorLeft();
    tgt::Color getCubeColorRight();
    tgt::Color getCubeColorTop();
    tgt::Color getCubeColorDown();
    void loadTextures(Identifier set = "");
    float getCubeSize();
    void setShowTextures(bool show);
    bool getShowTextures();

private:
    tgt::Color cubeColorFrontObject_;
    tgt::Color cubeColorBackObject_;
    tgt::Color cubeColorLeftObject_;
    tgt::Color cubeColorRightObject_;
    tgt::Color cubeColorTopObject_;
    tgt::Color cubeColorDownObject_;

    float cubeSizeObject_;

    bool showTextures_;

    tgt::Texture* frontTex_;
    tgt::Texture* backTex_;
    tgt::Texture* topTex_;
    tgt::Texture* leftTex_;
    tgt::Texture* bottomTex_;
    tgt::Texture* rightTex_;
    
    std::string textureNames_[6];
};

class QtCanvasSchematicOverlay : public VoreenPainterOverlay {
public:
    QtCanvasSchematicOverlay(tgt::QtCanvas* canvas);
    ~QtCanvasSchematicOverlay();
    virtual void paint();
    virtual void resize(int width, int height);

    void setCubePosX(float x);
    void setCubePosY(float y);
    
    void setShowTextures(bool show);
    void loadTextures(Identifier set);

protected:
    SchematicOverlayObject schematicOverlayObject_;

    int isClicked(int x, int y);
    int isClicked_;

    ColorProp* cubeColorFront_;
    ColorProp* cubeColorBack_;
    ColorProp* cubeColorLeft_;
    ColorProp* cubeColorRight_;
    ColorProp* cubeColorTop_;
    ColorProp* cubeColorDown_;

    FloatProp* cubeSize_;

    FloatProp* cubePosX_;
    FloatProp* cubePosY_;
};


class OrientationPlugin : public WidgetPlugin, public MessageReceiver, public tgt::EventListener   {
    Q_OBJECT
public:
    /**
     * This enumeration is used to define which orientation features are presented to the user.
     * By default all features are enabled.
     */
    enum Features {
        ORIENTATION_AND_DISTANCE    = 1, ///< Should orientation-and-distance box be shown?
        ORIENTATION_OVERLAY         = 2, ///< Should the checkbox for enabling orientation overlay be shown?
        CONTINUOUS_MOTION           = 4, ///< Should the continous-motion box be shown?
        TRACKBALL_BOX               = 8, ///< Should the save-and-load box be shown?
        ALL_FEATURES                = 15 ///< Show everything
    };

    OrientationPlugin(QWidget* parent = 0, MessageReceiver* rec = 0, tgt::QtCanvas* canvas = 0, 
                      tgt::Trackball* track = 0, TextureContainer* tc = 0);
    ~OrientationPlugin();

    void deinit();

    /// Enable features by passing a bitstring. \see Features
    void enableFeatures(int features);
    /// Disable features by passing a bitstring. \see Features
    void disableFeatures(int features);

    void processMessage(Message* msg, const Identifier& dest = Message::all_);

    void mousePressEvent(tgt::MouseEvent* e);
    void mouseMoveEvent(tgt::MouseEvent* e);
    void mouseReleaseEvent(tgt::MouseEvent* e);
    void mouseDoubleClickEvent(tgt::MouseEvent *e);

    int isClicked(int, int);

public slots:
    void toAbove();
    void toBelow();
    void toBehind();
    void toFront();
    void toLeft();
    void toRight();
    void updateDistance();
    void orientationChanged(int);
    void distanceSliderChanged(int);
    void distanceSliderPressed();
    void distanceSliderReleased();

    void saveTrackballToDisk();
    void saveTrackballToDisk(std::string s, bool shutdown = false);
    void restoreTrackball();
    void restoreTrackball(std::string s);
        
    void setRestore(bool b);
    bool getRestore();

    void enableContSpin(bool b);

    void enableX(bool b);
    void enableY(bool b);
    void enableZ(bool b);

    void enableOrientationOverlay(bool b);

    void setShowTextures(bool show);
    void loadTextures(Identifier set);

    virtual void createWidgets();
    virtual void createConnections();

protected:       
    /// Returns wether a certain feature is enabled.
    bool isFeatureEnabled(Features feature);
    virtual void timerEvent(QTimerEvent *event);
    void checkCameraState();
    void showEvent(QShowEvent* event);

    QtCanvasSchematicOverlay schematicOverlay_;

    tgt::Trackball* track_;
    tgt::QtCanvas* canvas_;
    int isClicked_;

    const int MIN_CAM_DIST;
    const int MAX_CAM_DIST;
    const float CAM_DIST_SCALE_FACTOR;

    const tgt::vec3 AXIAL_VIEW;
    const tgt::vec3 CORONAL_VIEW;
    const tgt::vec3 SAGITTAL_VIEW;
    const tgt::vec3 AXIAL_INV_VIEW;
    const tgt::vec3 CORONAL_INV_VIEW;
    const tgt::vec3 SAGITTAL_INV_VIEW;

    /// Determines which features are presented to the user for modification.
    int features_;

private:
    void applyOrientation(const tgt::quat& q);
    void shutdownSave(bool b);
    void startupRestore();
    /// enabled or disables timer based on whether rotation is active
    void setTimerState();
        
    bool restore_;

    QGroupBox* orientationBox_;
    QGroupBox* motionBox_;
    QGroupBox* trackballBox_;
    QComboBox* comboOrientation_;
    QSlider* slDistance_;

    QCheckBox* rotateAroundX_;
    QCheckBox* rotateAroundY_;
    QCheckBox* rotateAroundZ_;
    QCheckBox* continueSpin_;
    QCheckBox* showOrientationOverlay_;
    QBasicTimer* timer_;

    float dist_;
    bool rotateX_;
    bool rotateY_;
    bool rotateZ_;

    QToolButton* buSaveTrackball_;
    QToolButton* buRestoreTrackball_;
    QCheckBox* cbRestoreOnStartup_;

    TextureContainer* tc_;
};

} // namespace voreen

#endif
