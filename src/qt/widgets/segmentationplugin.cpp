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

#include "voreen/qt/widgets/segmentationplugin.h"

#include <QCheckBox>
#include <QSpinBox>

#include "voreen/core/vis/processors/render/volumeraycaster.h"

namespace voreen {

SegmentationPlugin::SegmentationPlugin(QWidget* parent, MessageReceiver* msgReceiver)
  : WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Segmentation"));
    icon_ = QIcon(":/icons/segmentation.png");
}

void SegmentationPlugin::createWidgets() {
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    segmentationSwitch_ = new QCheckBox(tr("Use segmentation"));
    segmentationSelect_ = new QSpinBox;
    segmentationSelect_->setRange(0, 255);
    vboxLayout->addWidget(segmentationSwitch_);

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    vboxLayout->addItem(hboxLayout);

    hboxLayout->addWidget(new QLabel(tr("Visible segment:")));
    hboxLayout->addWidget(segmentationSelect_);

    vboxLayout->addStretch();

    setLayout(vboxLayout);
}

void SegmentationPlugin::createConnections() {
    connect(segmentationSwitch_, SIGNAL(toggled(bool)), this, SLOT(toggleUseSegmentation(bool)));
    connect(segmentationSelect_, SIGNAL(valueChanged(int)), this, SLOT(setSegmentation(int)));
}

void SegmentationPlugin::toggleUseSegmentation(bool useSegmentation) {
	postMessage(new BoolMsg(VolumeRaycaster::switchSegmentation_, useSegmentation));
    repaintCanvases();
}

void SegmentationPlugin::setSegmentation(int segment) {
	postMessage(new IntMsg(VolumeRaycaster::setSegment_, segment));
    repaintCanvases();
}
} // namespace voreen

