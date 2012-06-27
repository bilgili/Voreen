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

#ifndef FLYBYPLUGIN_H
#define FLYBYPLUGIN_H

#include "widgetplugin.h"
#include <vector>
#include <string>

#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QBasicTimer>
#include <QFileDialog>
#include <QToolBox>
#include <QMenu>
#include <QCheckBox>
#include <QGroupBox>

#include "tgt/quaternion.h"
#include "tgt/vector.h"
#include "tgt/spline.h"
#include "tgt/camera.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/navigation/trackball.h"
#include "voreen/core/vis/processors/processor.h"

#ifdef VRN_WITH_DEVIL
#include "IL/il.h"
#endif

struct QExtAction : public QAction {

        QExtAction(const QString& text, QObject* parent, int num)
            : QAction(text, parent), num_(num) {}

        int num_;
};

namespace voreen {

struct Orientation {
    tgt::quat tripod_;
    tgt::vec3 position_;
    tgt::vec3 lightposition_;
    double timeToNextFrame_;
    QExtAction* action_;
    QPixmap pic_;
};

struct FlybyInfo {
    int width_, height_;
    bool record_;
    float total_, t_;
    int piccounter_;
    tgt::Spline sp_;
    tgt::Spline splight_;
    std::string filename_;
    bool pause_;
};

//class FlybyThread;

class FlybyPlugin : public WidgetPlugin {
    Q_OBJECT

    public:
    FlybyPlugin(QWidget* parent, MessageReceiver* msgreceiver, Processor* processor, tgt::Trackball* track, WId canvasID);

        virtual ~FlybyPlugin() {
            delete flybyTimer_;
        }
        std::vector<Orientation> getCurrentFlyby() const {
            return currentFlyby_;
        }

    protected:
        virtual void createWidgets();
        virtual void createConnections();
        virtual void timerEvent(QTimerEvent *event);

        void refreshFrameMenu();
        void gotoFrame(int num);

        void setWidgetState();

    public slots:

        void addKeyframe();
        void replaceKeyframe();
        void startFlyby(bool record = false, std::string dirname = std::string(""));
        void stopFlyby();
        void pauseFlyby();
        void clearFlyby();
        void recordFlyby();
        void saveFlyby();
        void loadFlyby();
        void flybyStep();
        void selectFrame(QAction* action, bool jumptoFrame);
        void selectFrame(QAction* action);
        void deleteFrame();
        void deleteLastFrame();
        void updateFrameTime(double val);

    private:
        tgt::QtCanvas* canvas_;
        std::vector<Orientation> currentFlyby_;
        int currentFrame_;
        bool goOnSelectEnabled_;

        QLabel* laFlyby_;
        QLabel* laNumberKeyframes_;

        QGroupBox* playerBox_;
        QGroupBox* createBox_;
        QGroupBox* editBox_;
        QGroupBox* recordBox_;
        QPushButton *addKeyframe_;
        QPushButton *deleteLast_;
        QPushButton *startFlyby_;
        QPushButton *stopFlyby_;
        QPushButton *pauseFlyby_;
        QPushButton *clearFlyby_;
        QPushButton *recordFlyby_;
        QPushButton *saveFlyby_;
        QPushButton *loadFlyby_;
        QSpinBox *spWidth_;
        QSpinBox *spHeight_;
        QCheckBox *checkLoop_;
        QBasicTimer *flybyTimer_;

        QPushButton *deleteFrame_;
        QPushButton *selectFrame_;
        QPushButton *replaceFrame_;
        QDoubleSpinBox *selectFrameTimeCreate_;
        QDoubleSpinBox *selectFrameTimeEdit_;
        QMenu *frameMenu_;

        WId canvasID_;
//         FlybyThread* fbThread_;
        FlybyInfo *fbInfo_;
        QWidget* parent_;

        tgt::Trackball* track_;
        Processor* processor_;
        tgt::vec4 lightPos_;
        float timeOffset_;
};

} // namespace voreen

#endif
