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

#ifndef VRN_QPROPERTYWIDGET_H
#define VRN_QPROPERTYWIDGET_H

#include "voreen/core/vis/properties/propertywidget.h"

#include <QWidget>
#include <QBoxLayout>
#include <QToolButton>

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

public slots:
    virtual void setLevelOfDetail(bool value);
    virtual void setLevelOfDetail(Property::LODSetting value);
    virtual void toggleInteractionMode(bool im);

signals:
    void modified();
    void levelOfDetailChanged(Property::LODSetting);

protected:
    void addWidget(QWidget* widget);
    void addLayout(QLayout* layout);
    void addVisibilityControls();

    bool disconnected_;
    QBoxLayout* layout_;
    Property* prop_;
    QToolButton* lodControl_;
};

} // namespace

#endif // VRN_QPROPERTYWIDGET_H
