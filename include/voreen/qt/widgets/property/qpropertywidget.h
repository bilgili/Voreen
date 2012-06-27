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

#ifndef VRN_QPROPERTYWIDGET_H
#define VRN_QPROPERTYWIDGET_H

#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertywidget.h"

#include <QWidget>
#include <QBoxLayout>
#include <QToolButton>

class QLabel;

namespace voreen {

class QPropertyWidget : public QWidget, public PropertyWidget {
    Q_OBJECT;
public:
    QPropertyWidget(Property* prop, QWidget* parent = 0, bool showNameLabel = true);
    virtual ~QPropertyWidget();
    virtual QSize sizeHint() const;

    virtual void setEnabled(bool enabled);
    virtual void setVisible(bool state);

    virtual void disconnect();

    void hideLODControls();
    void showLODControls();
    std::string getPropertyGuiName();
    void setPropertyGuiName(std::string);
    virtual const QLabel* getNameLabel() const;

    // this is a static variable for the font size used in all propertywidgets
    static const int fontSize_;

public slots:
    virtual void setLevelOfDetail(bool value);
    virtual void setLevelOfDetail(Property::LODSetting value);
    virtual void toggleInteractionMode(bool im);
    virtual void showNameLabel(bool);

signals:
    void modified();
    void levelOfDetailChanged(Property::LODSetting);
    void mouseClicked();
    void widgetChanged();

protected:
    void addWidget(QWidget* widget);
    void addLayout(QLayout* layout);
    void addVisibilityControls();
    void mouseMoveEvent(QMouseEvent*);

    bool disconnected_;
    QBoxLayout* layout_;
    Property* prop_;
    QToolButton* lodControl_;

    mutable QLabel* nameLabel_;
    bool showNameLabel_;
};

} // namespace

#endif // VRN_QPROPERTYWIDGET_H
