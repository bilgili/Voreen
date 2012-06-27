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

#ifndef VRN_COMPACTPROPERTYVECTORWIDGET_H
#define VRN_COMPACTPROPERTYVECTORWIDGET_H

#include "voreen/qt/widgets/compactproperty/compactpropertywidget.h"
#include "voreen/core/vis/properties/propertyvector.h"

class QVBoxLayout;

namespace voreen {

class PropertyVector;
class CompactPropertyWidgetFactory;

class CompactPropertyVectorWidget : public CompactPropertyWidget {
Q_OBJECT
public:
    CompactPropertyVectorWidget(PropertyVector* prop, QWidget* parent = 0);

    void update();

public slots:
    void setProperty(PropertyVector* value);
            

protected:

    /**
     * Creates the property widget for the passed property using the passed factory and
     * adds it to the property layout
     */
    void createAndAddPropertyWidget(Property* prop, CompactPropertyWidgetFactory* factory);

    /// The PropertyVector assigned to this widget
    PropertyVector* property_;

    /// Contains the property widgets.
    QVBoxLayout* propertiesLayout_;

};

} // namespace

#endif // VRN_COMPACTBOOLPROPERTYWIDGET_H
