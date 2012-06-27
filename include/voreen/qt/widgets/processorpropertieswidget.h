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

#ifndef VRN_PROCESSORPROPERTIESWIDGET_H
#define VRN_PROCESSORPROPERTIESWIDGET_H

#include "voreen/core/vis/properties/property.h"
#include <QWidget>

namespace voreen {

class ExpandableHeaderButton;
class Processor;
class PropertyWidgetFactory;
class QPropertyWidget;

/**
 * Widget containing each processor's property widgets. The title bar contains the name of the
 * processor and an +/- icon allowing to expand the properties.
 */
class ProcessorPropertiesWidget : public QWidget {
Q_OBJECT
public:
    ProcessorPropertiesWidget(QWidget* parent = 0, Processor* processor = 0, PropertyWidgetFactory* factory = 0,
        bool expanded = false, bool userExpandable = true);

    bool isExpanded() const;
    bool isUserExpandable() const;

    void setLevelOfDetail(Property::LODSetting lod);

signals:
    void propertyChanged();

public slots:
    void setExpanded(bool expanded);
    void setUserExpandable(bool expandable);
    void toggleExpansionState();
    void updateHeaderTitle();

protected slots:
    void updateState();
    void setLODHidden();
    void setLODVisible();

protected:
    ExpandableHeaderButton* header_;
    QWidget* propertyWidget_;
    Processor* processor_;
    std::vector<QPropertyWidget*> widgets_;
};

} // namespace

#endif // VRN_PROCESSORPROPERTIESWIDGET_H
