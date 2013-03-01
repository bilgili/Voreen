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

#ifndef VRN_QPROPERTYWIDGETWITHEDITORWINDOW_H
#define VRN_QPROPERTYWIDGETWITHEDITORWINDOW_H

#include "voreen/qt/widgets/property/qpropertywidget.h"

namespace voreen {

class VoreenToolWindow;

class QPropertyWidgetWithEditorWindow : public QPropertyWidget {
public:
    QPropertyWidgetWithEditorWindow(Property* prop, QWidget* parent = 0, bool showNameLabel = true, bool isEditorWindowResizable = true);
    virtual ~QPropertyWidgetWithEditorWindow();

    /**
     * Stores the state of the editor window.
     */
    virtual MetaDataBase* getWidgetMetaData() const;

protected:
    /**
     * Creates the editor window.
     */
    void createEditorWindow(Qt::DockWidgetArea area, const QString& titlePostfix = "", const int& initialWidth = -1, const int& initialHeight = -1);

    /**
     * Sets custom editor window properties.
     */
    virtual void customizeEditorWindow() = 0;

    /**
     * Creates the widget for the editor window
     */
    virtual QWidget* createEditorWindowWidget() = 0;

    virtual Property* getProperty() = 0;

    /**
     * Queries the property's meta data for the tool window's visibility state.
     */
    virtual bool editorVisibleOnStartup() const;

    bool isEditorWindowResizable_;
    VoreenToolWindow* editorWindow_;
};

} // namespace

#endif // VRN_QPROPERTYWIDGETWITHEDITORWINDOW_H
