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

#ifndef VRN_RPTPROPERTYLISTWIDGET_H
#define VRN_RPTPROPERTYLISTWIDGET_H

#include "rptnetworkserializergui.h"

#include <map>

#include <QScrollArea>

namespace voreen {

class Processor;
class ProcessorPropertiesWidget;

/**
 * This class displays a list of ProcessorPropertiesWidgets representing the processors in a network.
 * setProcessor() creates widgets for all properties of a given processor and displays them.
 */
class RptPropertyListWidget : public QScrollArea {
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
     * @param network the RptNetwork whose processors' properties are to be displayed
     */
    RptPropertyListWidget(QWidget* parent=0, RptNetwork* network = 0, WidgetMode mode = SINGLE, Property::LODSetting lod = Property::DEVELOPER);

    /**
     * Destructor - deletes all created widgets
     */
    ~RptPropertyListWidget();

    /**
     * Changes the WidgetMode.
     */
    void setWidgetMode(WidgetMode mode);

    /**
     * Returns the WidgetMode.
     */
    WidgetMode widgetMode() const;

    void setLevelOfDetail(Property::LODSetting lod);
    Property::LODSetting getLevelOfDetail() const { return lod_; }

    void setState(WidgetMode mode, Property::LODSetting lod);

    /**
     * Generates the property header widgets of all processors contained by the current network.
     */
    void createWidgets();     

signals:
    /**
     * Signal that is emitted when a property was changed by the user. It is fired
     * when a repaint of the rendering is necessary.
     */
    void repaintSignal();

public slots:
    void newNetwork(RptNetwork* network);
    void processorAdded(RptProcessorItem* procItem);
    void processorDeleted(Processor* processor);

    /**
    * This method is called when a processor is selected in the graph widget.
    * It will create and display a widget that shows all properties of the given
    * processor.
    *
    * @param p processor that is selected
    */
    void processorSelected(Processor* p);

    /**
    * Deletes all created widgets and sets the displayed widget to an empty one.
    */
    void clear();

protected:
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

    RptNetwork* rptNetwork_;

    QWidget* containerWidget_;

    WidgetMode widgetMode_;
    Property::LODSetting lod_;
    
    /// mapping from processors to the property widgets
    std::map<Processor*, ProcessorPropertiesWidget*> processorWidgetMap_;

    Processor* lastSelectedProcessor_;
};

} // namespace voreen

#endif //VRN_RPTPROPERTYLISTWIDGET_H
