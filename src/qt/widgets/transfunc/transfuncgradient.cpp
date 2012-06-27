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

#include "voreen/qt/widgets/transfunc/transfuncgradient.h"


#include <QtGui>

namespace voreen {

using tgt::Color;

TransFuncGradient::TransFuncGradient(QWidget *parent, bool clipThresholds)
    :   QWidget(parent),
        lowerThreshold_(0.f),
        upperThreshold_(1.f),
        clipThresholds_(clipThresholds)
{
    setFixedHeight(30);
    QBoxLayout *mainLayout = new QVBoxLayout(this);

    transCanvas_ = new tgt::QtCanvas("test", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, this, true);
    painter_ = new TransFuncIntensityPainter(transCanvas_);
    transCanvas_->setPainter(painter_);

    mainLayout->addWidget(transCanvas_);
}

TransFuncGradient::~TransFuncGradient() {
    delete painter_;
    delete transCanvas_;
}

void TransFuncGradient::paintEvent(QPaintEvent *) {
   transCanvas_->update();
}

void TransFuncGradient::toggleClipThresholds(bool enabled) {
    clipThresholds_ = enabled;
    update();
}

void TransFuncGradient::setThresholds(float lowerThreshold, float upperThreshold) {
    lowerThreshold_ = lowerThreshold;
    upperThreshold_ = upperThreshold;
    if (clipThresholds_)
        update();
}

void TransFuncGradient::setTextureCoords(float start, float end) {
    painter_->setTextureCoords(start, end);
}

void TransFuncGradient::setTransFunc(TransFuncIntensity* tf) {
    tf_ = tf;
    painter_->setTransFunc(tf);
    update();
}

} // namespace voreen
