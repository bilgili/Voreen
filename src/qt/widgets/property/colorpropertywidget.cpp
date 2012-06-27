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

#include "voreen/qt/widgets/property/colorpropertywidget.h"

#include "voreen/core/vis/properties/colorproperty.h"

#include <QColorDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

namespace voreen {

ClickableColorLabel::ClickableColorLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{}

void ClickableColorLabel::setColor(const QColor& color) {
    color_ = color;
    update();
}

void ClickableColorLabel::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton)
        emit(clicked());
}

void ClickableColorLabel::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setBrush(color_);
    painter.drawRect(1, 1, rect().width() - 2, rect().height() - 2);
}

ColorPropertyWidget::ColorPropertyWidget(ColorProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , colorLbl_(new ClickableColorLabel(""))
    , currentColor_(toQColor(prop->get()))
{
    updateColorLabel();
    addWidget(colorLbl_);
    connect(colorLbl_, SIGNAL(clicked(void)), this, SLOT(setProperty(void)));

    addVisibilityControls();
}

void ColorPropertyWidget::updateFromProperty() {
    currentColor_ = toQColor(property_->get());
    updateColorLabel();
}

void ColorPropertyWidget::setProperty() {
    if (!disconnected_) {
        colorDialog();
        updateColorLabel();
        tgt::Color value = toTgtColor(currentColor_);
        property_->set(value);
        emit modified();
    }
}

void ColorPropertyWidget::colorDialog() {
    currentColor_.setRgba(QColorDialog::getRgba(currentColor_.rgba()));
}

void ColorPropertyWidget::updateColorLabel() {
    colorLbl_->setColor(currentColor_);
}

tgt::Color ColorPropertyWidget::toTgtColor(QColor color) {
    return tgt::Color(color.redF(),color.greenF(),color.blueF(), color.alphaF());
}

QColor ColorPropertyWidget::toQColor(tgt::Color color) {
    return QColor(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
                  static_cast<int>(color.b * 255), static_cast<int>(color.a * 255));
}

} // namespace
