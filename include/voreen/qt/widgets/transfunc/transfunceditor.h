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

#ifndef VRN_TRANSFUNCEDITOR_H
#define VRN_TRANSFUNCEDITOR_H

#include "voreen/core/properties/transfuncproperty.h"

#include <QWidget>
#include <QString>

namespace voreen {

    class VolumeBase;

/**
 * Abstract base class for all transfer function widgets. It provides methods to open a Filedialog
 * for loading and saving of a transfer function and a slot for switching coarseness mode on and off.
 */
class TransFuncEditor : public QWidget , public VolumeObserver {
    Q_OBJECT

public:
    /**
     * Default constructor
     *
     * @param prop the transfer function property that belongs to this plugin
     * @param parent the parent widget
     */
    TransFuncEditor(TransFuncProperty* prop, QWidget* parent = 0);

    /**
     * Destructor
     */
    virtual ~TransFuncEditor();

    /**
     * Assigns the passed property to the editor. The
     * former property is replaced.
     */
    virtual void setTransFuncProp(TransFuncProperty* prop);

    /**
     * In this method the plugin will be layouted.
     */
    virtual void createWidgets() = 0;

    /**
     * In this method all necessary connections will be created.
     */
    virtual void createConnections() = 0;

    /**
     * Call this in order to update the editor's state to external changes (e.g. changes of the transfer function).
     */
    virtual void updateFromProperty() = 0;

    /**
     * This method is called by transfuncplugin when the user selected this editor.
     * It resets the transfer function and thresholds.
     */
    virtual void resetEditor() = 0;

    /**
     * Returns the name of the editor. This name is used in a Tabwidget to change the editors.
     *
     * @return name of the editor
     */
    const QString getTitle();

    // VolumeObserver methods:
    virtual void volumeDelete(const VolumeBase* source) = 0;
    virtual void volumeChange(const VolumeBase* source) = 0;
    virtual void derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData) = 0;
signals:
    /**
     * This signal is emitted when the transfer function has changed.
     */
    void transferFunctionChanged();

public slots:

    /**
     * Starts or stops the interaction mode.
     *
     * @param on interaction mode on or off?
     */
    void toggleInteractionMode(bool on);

protected:
    TransFuncProperty* property_; ///< the transfer function property that belongs to this plugin

    const VolumeBase* volume_; ///< TODO: remove it.

    QString title_; ///< name of the editor that is displayed in the gui
};

} //namespace voreen

#endif // VRN_TRANSFUNCEDITOR_H
