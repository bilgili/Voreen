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

#include "voreen/qt/widgets/stereoplugin.h"
#include "voreen/core/vis/voreenpainter.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>


namespace voreen {

const Identifier StereoPlugin::setStereoMode_("set.stereoMode");
const Identifier StereoPlugin::setEyeDistance_("set.eyeDistance");
const Identifier StereoPlugin::setFocalDistance_("set.focalDistance");
const Identifier StereoPlugin::setCorrectWindowPos_("set.correctWindowPos");


StereoPlugin::StereoPlugin(QWidget* parent, MessageReceiver* msgReceiver) : WidgetPlugin(parent, msgReceiver) {
    setObjectName(tr("Stereoscopy"));
    icon_ = QIcon(":/icons/3d-glasses.png");
}

void StereoPlugin::createWidgets() {
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QHBoxLayout* hboxLayout;

    hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(new QLabel(tr("Rendering mode:")));
    stereoCombo_ = new QComboBox();
    stereoCombo_->addItem(tr("monoscopic"));
    stereoCombo_->addItem(tr("stereoscopic"));
    stereoCombo_->addItem(tr("autostereoscopic"));

    hboxLayout->addWidget(stereoCombo_);

    vboxLayout->addItem(hboxLayout);

    eyeDistanceSpin_ = new QSpinBox();
    eyeDistanceSpin_->setRange(0,20);
    eyeDistanceSpin_->setValue(1);

    focalSpin_ = new QSpinBox();
    focalSpin_->setRange(0,20);
    focalSpin_->setValue(1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(new QLabel(tr("Eye distance:")));
    hboxLayout->addWidget(eyeDistanceSpin_);

    hboxLayout->addWidget(new QLabel(tr("Focal distance:")));
    hboxLayout->addWidget(focalSpin_);

    vboxLayout->addItem(hboxLayout);

    hboxLayout = new QHBoxLayout();
    correctWindowPosChecker_ = new QCheckBox(tr("Switch autostereoscopic assignment on screen"));
    hboxLayout->addWidget(correctWindowPosChecker_);
    vboxLayout->addItem(hboxLayout);

    vboxLayout->addStretch();
    setLayout(vboxLayout);
}

void StereoPlugin::createConnections() {
    connect(stereoCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeStereoMode(int)));
    connect(eyeDistanceSpin_, SIGNAL(valueChanged(int)), this, SLOT(changeEyeDistance(int)));
    connect(focalSpin_, SIGNAL(valueChanged(int)), this, SLOT(changeFocalDistance(int)));
    connect(correctWindowPosChecker_, SIGNAL(toggled(bool)), this, SLOT(correctWindowPos(bool)));

}

void StereoPlugin::changeStereoMode(int stereoMode) {
    switch(stereoMode){
        case 0:
            postMessage(new IntMsg(setStereoMode_, VoreenPainter::VRN_MONOSCOPIC));
            repaintCanvases();
            break;
        case 1:
            postMessage(new IntMsg(setStereoMode_, VoreenPainter::VRN_STEREOSCOPIC));
            repaintCanvases();
            break;
        case 2:
            postMessage(new IntMsg(setStereoMode_, VoreenPainter::VRN_AUTOSTEREOSCOPIC));
            repaintCanvases();
            break;
    }
}

void StereoPlugin::changeEyeDistance(int dist) {
    postMessage(new IntMsg(setEyeDistance_, dist));
    repaintCanvases();
}

void StereoPlugin::changeFocalDistance(int dist) {
    postMessage(new IntMsg(setFocalDistance_, dist));
    repaintCanvases();
}

void StereoPlugin::correctWindowPos(bool /*on*/) {
    if (correctWindowPosChecker_->isChecked())
        postMessage(new IntMsg(setCorrectWindowPos_, 1));
    else
        postMessage(new IntMsg(setCorrectWindowPos_, 0));
    repaintCanvases();
}

} // namespace voreen

