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

#include "voreen/qt/widgets/dynamicsplugin.h"

#include <QTimer>

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

DynamicsPlugin::DynamicsPlugin(QWidget* parent, MessageReceiver* msgReceiver)
  : WidgetPlugin(parent, msgReceiver), numDatasets_(1)
{
    setObjectName("Dynamics");
    icon_ = QIcon(":/icons/dynamics.png");
    dataset_ = new SliderSpinBoxWidget();
    timer_ = new QTimer(this);
}

DynamicsPlugin::~DynamicsPlugin() {
    delete timer_;
}

void DynamicsPlugin::createWidgets() {
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QHBoxLayout* ahw = new QHBoxLayout();
    ahw->addWidget(new QLabel(tr("Dataset")));
    ahw->addWidget(dataset_);
    vboxLayout->addItem(ahw);

    QHBoxLayout* playerLayout = new QHBoxLayout();

    QWidget *playerBox = new QWidget(this);
    playerBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    rewindButton_ = new QPushButton();
    rewindButton_->setIcon(QIcon(":/icons/player_rew.png"));
    playerLayout->addWidget(rewindButton_);

    QVBoxLayout *playButtonLayout = new QVBoxLayout();
    QGroupBox *playBox = new QGroupBox();
    playButton_ = new QPushButton(playBox);
    playButton_->setIcon(QIcon(":/icons/player_play.png"));
    playButton_->setCheckable(true);
    playButtonLayout->addWidget(playButton_);

    loopCheckBox_ = new QCheckBox(tr("loop"));
    playButtonLayout->addWidget(loopCheckBox_);
    playBox->setLayout(playButtonLayout);
    playerLayout->addWidget(playBox);

    ffButton_ = new QPushButton();
    ffButton_->setIcon(QIcon(":/icons/player_fwd.png"));
    playerLayout->addWidget(ffButton_);

    stopButton_ = new QPushButton();
    stopButton_->setIcon(QIcon(":/icons/player_stop.png"));
    playerLayout->addWidget(stopButton_);

    playerBox->setLayout(playerLayout);
    vboxLayout->addWidget(playerBox);

    vboxLayout->addStretch();

    setLayout(vboxLayout);
}

void DynamicsPlugin::createConnections() {
    connect(dataset_, SIGNAL(valueChanged(int)), this, SLOT(setDataset(int)));
    connect(playButton_, SIGNAL(pressed()), this, SLOT(play()));
    connect(stopButton_, SIGNAL(pressed()), this, SLOT(stop()));
    connect(rewindButton_, SIGNAL(pressed()), this, SLOT(rewind()));
    connect(ffButton_, SIGNAL(pressed()), this, SLOT(fastForward()));
    connect(timer_, SIGNAL(timeout()), this, SLOT(run()));
}

void DynamicsPlugin::run() {
    int value = dataset_->getValue();
    ++value;
    if (value > dataset_->getMaxValue()) {
        if (isLooping()) {
            value = dataset_->getMinValue();
            dataset_->setValue(value);
        }
        else
            stop();
    }
    else
        dataset_->setValue(value);
}

void DynamicsPlugin::setNumDatasets(unsigned int num) {
    numDatasets_ = num;
    dataset_->setMaxValue(numDatasets_-1);
}

void DynamicsPlugin::setDataset(int val) {
    dataset_->setValue(val);
    dataset_->update();
    //FIXME: this won't wokr anymore
    //postMessage(new IntMsg(Processor::setCurrentDataset_, val));
    repaintCanvas();
}

void DynamicsPlugin::play() {
    if (!timer_->isActive())
        timer_->start(500);
    else
        timer_->stop();
}

void DynamicsPlugin::stop() {
    timer_->stop();
    playButton_->setChecked(false);
}

void DynamicsPlugin::rewind() {
    int curVal = dataset_->getValue();
    int minVal = dataset_->getMinValue();
    int newVal = (curVal-10 > minVal) ? curVal-10 : minVal;
    dataset_->setValue(newVal);
}

void DynamicsPlugin::fastForward() {
    int curVal = dataset_->getValue();
    int maxVal = dataset_->getMaxValue();
    int newVal = (curVal+10<maxVal) ? curVal+10 : maxVal;
    dataset_->setValue(newVal);
}

} // namespace voreen
