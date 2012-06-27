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

#ifndef VRN_PLOTDATAPROPERTYWIDGET_H
#define VRN_PLOTDATAPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h"

class QPushButton;

namespace voreen {

// some forward declarations
class PlotData;
class PlotDataProperty;
class VoreenToolWindow;
class PlotDataWidget;

class PlotDataPropertyWidget : public QPropertyWidgetWithEditorWindow {
    Q_OBJECT
public:
    PlotDataPropertyWidget(PlotDataProperty* prop, QWidget* parent = 0);

    void updateFromProperty();

private slots:
    void toggleWidgetVisibility();

protected:
    virtual QWidget* createEditorWindowWidget();
    virtual void customizeEditorWindow();
    virtual Property* getProperty();

private:
    QPushButton* editBt_;

    PlotDataProperty* property_;
    PlotDataWidget* plotDataWidget_;

    //lazy evaluation, it is expensive to update the widget, so we do it only if necessary
    bool updateFromPropertyFlag_;
};

} // namespace

#endif // VRN_PLOTDATAPROPERTYWIDGET_H
