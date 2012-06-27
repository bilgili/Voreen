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

#ifndef VRN_COMPACTPROPERTYVECTORWIDGET_H
#define VRN_COMPACTPROPERTYVECTORWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/core/properties/propertyvector.h"

class QGridLayout;

namespace voreen {

class PropertyVector;
class QPropertyWidgetFactory;

class PropertyVectorWidget : public QPropertyWidget {
Q_OBJECT
public:

    enum Properties {
        BOOL,
        BUTTON,
        FLOATMAT2,
        FLOATMAT3,
        FLOATMAT4,
        FLOATVEC2,
        FLOATVEC3,
        FLOATVEC4,
        INTVEC2,
        INTVEC3,
        INTVEC4,
        /*CAMERA,*/
        COLOR,
        /*FILEDIALOG,*/
        FLOAT,
        INT,
        LIGHT,
        OPTION,
        /*SHADER,*/
        STRING
    };

    PropertyVectorWidget(PropertyVector* prop, QWidget* parent = 0);

    void updateFromProperty();

public slots:
    void setProperty(PropertyVector* value);


protected:

    /**
     * Creates the property widget for the passed property using the passed factory and
     * adds it to the property layout
     */
    void createAndAddPropertyWidget(Property* prop, QPropertyWidgetFactory* factory);
    void createAndAddPropertyWidgetByAction(QAction*);

    void initializePropertyMenu();

    /// The PropertyVector assigned to this widget
    PropertyVector* property_;

    /// Contains the property widgets.
    QGridLayout* propertiesLayout_;

    virtual void contextMenuEvent (QContextMenuEvent*);

    void propertyAdded();

    std::map<QAction*, int> propertyMap_;

    QMenu* propertyMenu_;

};

} // namespace

#endif // VRN_COMPACTBOOLPROPERTYWIDGET_H
