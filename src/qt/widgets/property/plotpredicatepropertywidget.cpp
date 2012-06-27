/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/property/plotpredicatepropertywidget.h"

#include "voreen/core/plotting/plotpredicate.h"
#include "voreen/core/properties/plotpredicateproperty.h"

#include <QListWidget>
#include <sstream>

namespace voreen {

PlotPredicatePropertyWidget::PlotPredicatePropertyWidget(PlotPredicateProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , lWidget_(new QListWidget)
{
    updateFromProperty();
    QFontInfo fontInfo(font());
    lWidget_->setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));
    addWidget(lWidget_);
    //connect(lWidget_, SIGNAL(currentIndexChanged(int)), this, SLOT(setProperty(int)));
    //connect(lWidget_, SIGNAL(currentIndexChanged(int)), this, SIGNAL(widgetChanged()));

    addVisibilityControls();
}

void PlotPredicatePropertyWidget::updateFromProperty() {
    lWidget_->blockSignals(true);
    lWidget_->clear();

    // build combo box from descriptions
    std::vector<std::pair<int, PlotPredicate*> >::const_iterator it;
    for (it = property_->get().begin(); it < property_->get().end(); ++it) {
        std::stringstream ss;
        ss << (*it).first << ": " << (*it).second->toString();
        lWidget_->addItem(QString::fromStdString(ss.str()));
    }

    lWidget_->blockSignals(false);
}


} // namespace
