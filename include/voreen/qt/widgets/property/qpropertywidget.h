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

#ifndef VRN_QPROPERTYWIDGET_H
#define VRN_QPROPERTYWIDGET_H

#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertywidget.h"

#include <QWidget>
#include <QBoxLayout>
#include <QToolButton>

class QLabel;

namespace voreen {

class CustomLabel;

class QPropertyWidget : public QWidget, public PropertyWidget {
    Q_OBJECT;

    friend class CustomLabel;

public:
    QPropertyWidget(Property* prop, QWidget* parent = 0, bool showNameLabel = true);
    virtual ~QPropertyWidget();
    virtual QSize sizeHint() const;

    /**
     * Public method called by the owning property. Delegates the call to updateFromPropertySlot()
     * via the signal/slot mechanism. This redirection is necessary to allow background threads
     * to call updateFromProperty().

     * @note: Do not overwrite this method, but updateFromPropertySlot() instead!
     */
    virtual void updateFromProperty();

    virtual void setEnabled(bool enabled);
    virtual void setVisible(bool state);

    virtual void disconnect();

    void hideLODControls();
    void showLODControls();
    std::string getPropertyGuiName();
    void setPropertyGuiName(std::string);
    virtual CustomLabel* getNameLabel() const;

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
    void visibilityChanged();

protected:
    void addWidget(QWidget* widget);
    void addLayout(QLayout* layout);
    void addVisibilityControls();
    void mouseMoveEvent(QMouseEvent*);

    bool disconnected_;
    QBoxLayout* layout_;
    QToolButton* lodControl_;

    mutable CustomLabel* nameLabel_;
    bool showNameLabel_;

protected slots:
    /**
     * Override this method to perform the update operations
     * that would usually be placed in updateFromProperty().
     *
     * updateFromProperty() calls are redirected to this method via a signal.
     */
    virtual void updateFromPropertySlot() = 0;

signals:
    void updateFromPropertySignal();

};

} // namespace

#endif // VRN_QPROPERTYWIDGET_H
