/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_PROPERTYOWNERWIDGET_H
#define VRN_PROPERTYOWNERWIDGET_H

#include "voreen/core/properties/property.h"
#include <QWidget>
#include <QVBoxLayout>

namespace voreen {

class ExpandableHeaderButton;
class QPropertyWidget;
class GroupPropertyWidget;

/**
 * Widget containing a PropertyOwner's property widgets. The title bar contains the name of the
 * PropertyOwner and an +/- icon allowing to expand the properties.
 */
class PropertyOwnerWidget : public QWidget, public PropertyOwnerObserver {
Q_OBJECT
public:

    enum widgetInstantiationState {
        NONE,
        ONLY_TF,
        ALL
    };

    PropertyOwnerWidget(QWidget* parent = 0, PropertyOwner* owner = 0, std::string title = "",
                              bool expanded = false, bool userExpandable = true, bool addResetButton = false);

    bool isExpanded() const;
    bool isUserExpandable() const;

    void setLevelOfDetail(Property::LODSetting lod);

    void updateProperties(const PropertyOwner*);
    void propertiesChanged(const PropertyOwner*);

    PropertyOwner* getOwner() const;

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
    void widgetInstantiation();
    void resetAllProperties();

protected:
    ExpandableHeaderButton* header_;
    QWidget* propertyWidget_;
    PropertyOwner* owner_;
    std::vector<QPropertyWidget*> widgets_;
    std::map<std::string, GroupPropertyWidget*> propertyGroupsMap_;
    void showEvent(QShowEvent*);

    bool expanded_;
    bool userExpandable_;
    bool addResetButton_;
    QVBoxLayout* mainLayout_;
    widgetInstantiationState widgetInstantiationState_;
};

} // namespace

#endif
