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

#ifndef VRN_TRANSFUNCPLUGIN_H
#define VRN_TRANSFUNCPLUGIN_H

#include "voreen/qt/widgets/qpropertywidget.h"

#include <QShowEvent>

class QStackedWidget;
class QTabWidget;

namespace voreen {

class Processor;
class TransFuncEditor;
class TransFuncProp;

/**
 * Container class for transfer function editors. Actually 3 editors for intensity
 * transfer functions are possible. They are arrenged in a QTabWidget.
 */
class TransFuncPlugin : public QPropertyWidget {
Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param proc editors for the transfer function property in this processor are created
     * @param parent the parent widget
     * @param orientation should the editors layouted vertically or horizontally?
     */
    TransFuncPlugin(Processor* proc, QWidget* parent = 0,
                    Qt::Orientation orientation = Qt::Horizontal);

    /**
     * Constructor
     *
     * @param prop the transfer function property that belongs to this plugin
     * @param parent the parent widget
     * @param orientation should the editors layouted vertically or horizontally?
     */
    TransFuncPlugin(TransFuncProp* prop, QWidget* parent = 0,
                    Qt::Orientation orientation = Qt::Horizontal);

    /**
     * Destructor
     */
    ~TransFuncPlugin();

    /**
     * Creates the editors
     *
     * @param parent the parent widget
     * @param orientation should the editors layouted vertically or horizontally?
     */
    void createEditors(int index, QWidget* parent, Qt::Orientation orientation);

    /**
     * Creates the widgets for all editors and puts them into a QTabWidget.
     */
    void createWidgets();

    /**
     * Creates the connections for all editors and the QTabWidget.
     * Additionally the first editor is set active by calling editorChanged()
     */
    void createConnections();

    /**
     * This method is called by the transfer function property when the rendered volume changes.
     * It calls the volumeChanged() method of the current active editor.
     */
    void update();

    /**
     * Sets disconnected_ to true. This indicates that the widget is already removed from the
     * property or not.
     */
    void disconnect();

public slots:
    /**
     * Slot that is called when the user selects another editor in the tab widget.
     * It will switch to the new editor when the user agrees with the reset of the
     * transfer function. Otherwise the old editor remains.
     *
     * @param index index of the selected tab
     */
    void editorChanged(int index);

    /**
     * Slot that is called when the user selects another transfer function in the combobox.
     * It changes the displayed editors to the editors for the selected transfer function.
     *
     * @param index index of selected transfer function
     */
    void comboBoxChanged(int index);

signals:
    /**
     * This signal is emitted whenever the transfer function has changed. It is used to force a
     * repaint of the volume rendering
     */
    void transferFunctionChanged();

protected:
    void showEvent(QShowEvent* event);

    // The following method stubs are necessary, because transfuncplugin inherits from qpropertywidget
    void addVisibilityControls() {}
    void showLODControls() {}
    void hideLODControls() {}
    void setLevelOfDetail(bool /*value*/) {}
    
private:
    std::vector<std::vector<TransFuncEditor*> > editors_; ///< vector with all editors. The user can choose between all.
    std::vector<TransFuncProp*> properties_; ///< vector with all transfer function properties that belong to this plugin
    std::vector<QTabWidget*> tabWidgets_;    ///< vector with all created QTabWidgets
    QStackedWidget* stackedWidget_;          ///< stacked widget in which all editors are displayed
    bool disconnected_;                      ///< indicates whether the widget was already disconnected from the property
    int oldIndex_;                           ///< index of the last active tab
};

} // namespace voreen

#endif // VRN_TRANSFUNCPLUGIN_H
