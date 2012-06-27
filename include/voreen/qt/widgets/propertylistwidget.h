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

#include <map>

#include <QScrollArea>

namespace voreen {

class Processor;

/**
 * This class displays widgets for all properties of a processor in a scrollarea.
 * setProcessor() creates widgets for all properties of a given processor and dispalys them.
 * It is called whenever the user selects a processor in the graph widget.
 */
class PropertyListWidget : public QScrollArea {
    Q_OBJECT

public:
    /**
     * Constructor - creates an empty layout
     *
     * @param parent parent widget
     */
    PropertyListWidget(QWidget* parent=0);

    /**
     * Destructor - deletes all created widgets
     */
    ~PropertyListWidget();

    /**
     * This method is called when a processor is selected in the graph widget.
     * It will create and display a widget that shows all properties of the given
     * processor.
     *
     * @param p processor that is selected
     */
    void setProcessor(Processor* p);

    /**
     * This is called when the given processor should be deleted. It sets the displayed properties
     * to an empty widget and deletes the widget for the processor from the map.
     *
     * @param processor processor that should be deleted
     */
    void processorDeleted(Processor* processor);

    /**
     * Deletes all created widgets and sets the displayed widget to an empty one.
     */
    void clear();

signals:
    /**
     * Signal that is emitted when a property was changed by the user. It will cause a repaint
     * of the volume rendering. The signal is connected to the propertyChanged() signal of the
     * property widgets.
     */
    void repaintSignal();

 protected:
     /**
      * Creates a widget which layout contains the widgets of all properties of the given processor.
      *
      * @param processor processor whose property widgets will be created
      * @return widget that consists of all property widgets for the given processor
      */
    QWidget* createWidget(Processor* processor);

    QWidget* emptyWidget_; ///< dummy widget for an empty layout
    std::map<Processor*, QWidget*> processorWidgetMap_; ///< mapping from processors to the property widget

};

} // namespace voreen

#endif //VRN_PROPERTYLISTWIDGET_H
