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

#ifndef VRN_PROCESSORPROPERTIESWIDGET_H
#define VRN_PROCESSORPROPERTIESWIDGET_H

#include "voreen/core/properties/property.h"
#include <QWidget>
#include <QVBoxLayout>

namespace voreen {

class ExpandableHeaderButton;
class Processor;
class PropertyWidgetFactory;
class QPropertyWidget;
class VolumeContainer;
class GroupPropertyWidget;


/**
 * Widget containing each processor's property widgets. The title bar contains the name of the
 * processor and an +/- icon allowing to expand the properties.
 */
class ProcessorPropertiesWidget : public QWidget {
Q_OBJECT
public:

    enum widgetInstantiationState {
        NONE,
        ONLY_TF,
        ALL
    };

    ProcessorPropertiesWidget(QWidget* parent = 0, const Processor* processor = 0,
                              bool expanded = false, bool userExpandable = true);

    bool isExpanded() const;
    bool isUserExpandable() const;

    void setLevelOfDetail(Property::LODSetting lod);
    void setVolumeContainer(VolumeContainer*);

signals:
    void modified();

public slots:
    void setExpanded(bool expanded);
    void setUserExpandable(bool expandable);
    void toggleExpansionState();
    void updateHeaderTitle();
    void instantiateWidgets();
    void updateState();
    void showHeader(bool);

protected slots:
    void propertyModified();
    void setLODHidden();
    void setLODVisible();

protected:
    ExpandableHeaderButton* header_;
    QWidget* propertyWidget_;
    const Processor* processor_;
    std::vector<QPropertyWidget*> widgets_;
    std::map<std::string, GroupPropertyWidget*> propertyGroupsMap_;
    void showEvent(QShowEvent*);

    VolumeContainer* volumeContainer_;
    bool expanded_;
    bool userExpandable_;
    PropertyWidgetFactory* widgetFactory_;
    QVBoxLayout* mainLayout_;
    widgetInstantiationState widgetInstantiationState_;
};

} // namespace

#endif // VRN_PROCESSORPROPERTIESWIDGET_H
