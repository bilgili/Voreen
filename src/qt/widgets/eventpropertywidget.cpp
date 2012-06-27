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

#include "voreen/qt/widgets/eventpropertywidget.h"
#include "voreen/core/vis/properties/eventproperty.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/qt/widgets/keydetectorwidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

namespace voreen {

EventPropertyWidget::EventPropertyWidget(EventProperty* property, QWidget* parent)
    : QWidget(parent)
    , property_(property)
{
    layout_ = new QHBoxLayout(this);

    QLabel* name = new QLabel(QString::fromStdString(property->getOwner()->getName() + "." + property->getGuiText()));
    layout_->addWidget(name);
    layout_->addSpacing(20);

    if (property->isMouseEvent())
        createMouseWidgets();
    if (property->isKeyEvent())
        createKeyWidgets();
}

void EventPropertyWidget::createMouseWidgets() {
    ModifierDetectorWidget* modifierWidget = new ModifierDetectorWidget;
    modifierWidget->setText(ModifierDetectorWidget::getStringForModifier(ModifierDetectorWidget::getQtModifierFromTGT(property_->getModifier())));
    modifierWidget->setMinimumWidth(50);
    connect(modifierWidget, SIGNAL(modifierChanged(Qt::KeyboardModifiers)), this, SLOT(modifierChanged(Qt::KeyboardModifiers)));
    layout_->addWidget(modifierWidget);

    QComboBox* buttonBox = new QComboBox;
    buttonBox->addItem("Left mouse button");
    buttonBox->addItem("Middle mouse button");
    buttonBox->addItem("Right mouse button");
    buttonBox->addItem("Mouse wheel up");
    buttonBox->addItem("Mouse wheel down");
    buttonBox->addItem("All buttons");
    switch (property_->getMouseButtons()) {
        case tgt::MouseEvent::MOUSE_BUTTON_LEFT:
            buttonBox->setCurrentIndex(0);
            break;
        case tgt::MouseEvent::MOUSE_BUTTON_MIDDLE:
            buttonBox->setCurrentIndex(1);
            break;
        case tgt::MouseEvent::MOUSE_BUTTON_RIGHT:
            buttonBox->setCurrentIndex(2);
            break;
        case tgt::MouseEvent::MOUSE_WHEEL_UP:
            buttonBox->setCurrentIndex(3);
            break;
        case tgt::MouseEvent::MOUSE_WHEEL_DOWN:
            buttonBox->setCurrentIndex(4);
            break;
        case tgt::MouseEvent::MOUSE_ALL:
        default:
            buttonBox->setCurrentIndex(5);
            break;
    }
    connect(buttonBox, SIGNAL(currentIndexChanged(int)), this, SLOT(buttonChanged(int)));
    layout_->addWidget(buttonBox);
}

void EventPropertyWidget::createKeyWidgets() {
    KeyDetectorWidget* keyWidget = new KeyDetectorWidget;
    keyWidget->setText(KeyDetectorWidget::getStringForKey(KeyDetectorWidget::getQtKeyFromTGT(property_->getKeyCode())));
    connect(keyWidget, SIGNAL(key(int)), this, SLOT(keyChanged(int)));
    layout_->addWidget(keyWidget);
}

void EventPropertyWidget::modifierChanged(Qt::KeyboardModifiers modifier) {
    tgt::Event::Modifier m = ModifierDetectorWidget::getTGTModifierFromQt(modifier);
    property_->setModifier(m);
}

void EventPropertyWidget::keyChanged(int key) {
    property_->setKeyCode(KeyDetectorWidget::getTGTKeyFromQt(key));
}

void EventPropertyWidget::buttonChanged(int button) {
    switch (button) {
    case 0:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_BUTTON_LEFT);
        break;
    case 1:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_BUTTON_MIDDLE);
        break;
    case 2:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_BUTTON_RIGHT);
        break;
    case 3:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_WHEEL_UP);
        break;
    case 4:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_WHEEL_DOWN);
        break;
    case 5:
        property_->setMouseButtons(tgt::MouseEvent::MOUSE_ALL);
        break;
    }
}

} // namespace
