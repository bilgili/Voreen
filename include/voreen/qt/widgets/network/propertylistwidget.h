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

#ifndef VRN_PROPERTYLISTWIDGET_H
#define VRN_PROPERTYLISTWIDGET_H

#include "voreen/core/vis/processors/processornetwork.h"

#include <map>
#include <QScrollArea>

namespace voreen {

class Processor;
class ProcessorPropertiesWidget;

/**
 * This class displays a list of ProcessorPropertiesWidgets representing the processors' properties in a network.
 *
 * PropertyListWidget objects register themselves as observers at the assigned ProcessorNetwork.
 */
class PropertyListWidget : public QScrollArea, public ProcessorNetworkObserver {
    Q_OBJECT
public:

    /**
     * Determines the widget's gui mode.
     */
    enum WidgetMode {
        LIST,   ///< Properties of all network processors are shown in an expandable list.
        SINGLE  ///< Only the properties of the currently selected processor are shown.
    };

    /**
     * Constructor - creates an empty layout
     *
     * @param parent parent widget
     * @param network the ProcessorNetwork whose processors' properties are to be displayed.
     * @param mode the widget mode
     * @param lod the level of detail of the displayed properties
     */
    PropertyListWidget(QWidget* parent=0, ProcessorNetwork* network = 0, WidgetMode mode = SINGLE, Property::LODSetting lod = Property::DEVELOPER);

    /**
     * Destructor - deletes all created widgets
     */
    ~PropertyListWidget();

    /**
    * Changes the WidgetMode.
     */
    void setWidgetMode(WidgetMode mode);

    /**
     * Returns the WidgetMode.
     */
    WidgetMode widgetMode() const;

    /**
     * Adjusts the level of detail of all property widgets.
     */
    void setLevelOfDetail(Property::LODSetting lod);
    
    /**
     * Returns the property list's current level of detail.
     */
    Property::LODSetting getLevelOfDetail() const { return levelOfDetail_; }

    /**
     * Convenience function for setting the WidgetMode and LevelOfDetail
     * settings in combination.
     */
    void setState(WidgetMode mode, Property::LODSetting lod);

    // Implementation of the ProcessorNetworkObserver interface
    void networkChanged();
    void processorAdded(Processor* processor);
    void processorRemoved(Processor* processor);

signals:

    /**
     * Signal that is emitted when a property was been changed by the user. It is fired
     * when a repaint of the rendering is necessary.
     */
    void repaintSignal();

public slots:

    /**
     * This method passes the ProcessorNetwork whose processors' properties 
     * are to be displayed by the PropertyListWidget.
     */
    void setNetwork(ProcessorNetwork* network);

    /**
     * This method is called when a processor is selected in the NetworkEditor.
     * It will unfold the header widget of the selected processor.
     *
     * @param p processor that is selected
     */
    void processorSelected(Processor* p);

    /**
     * Should be called when a processor's name has changed in order to update
     * the corresponding property widget.
     */
    void processorNameChanged(Processor* p);
    
    /**
     * Deletes all created widgets and sets the displayed widget to an empty one.
     */
    void clear();

protected:

    /**
     * Generates the property widgets of all processors contained by the current network.
     */
    void createWidgets();    

    /**
     * Sets all header widgets invisible. 
     */
    void hideAll();

    /**
     * Sets all header widgets visible. Their expansion state is not changed. 
     */
    void showAll();

    /** 
     * Sets all header widgets to be expandable by the user.
     */
    void setAllExpandable();

    /** 
     * Sets all header widgets to be not expandable by the user.
     */
    void setAllUnexpandable();

    void updateState();

    ProcessorNetwork* processorNetwork_;
    /// mapping from processors to the property widgets
    std::map<Processor*, ProcessorPropertiesWidget*> processorWidgetMap_;

    WidgetMode widgetMode_;
    Property::LODSetting levelOfDetail_;

    QWidget* containerWidget_;
    Processor* lastSelectedProcessor_;

};

} // namespace voreen

#endif //VRN_PROPERTYLISTWIDGET_H
