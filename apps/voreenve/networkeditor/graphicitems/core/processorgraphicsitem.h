/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_PROCESSORGRAPHICSITEM_H
#define VRN_PROCESSORGRAPHICSITEM_H

#include "portownergraphicsitem.h"

namespace voreen {

class Processor;

/**
 * Qt graphic item class corresponding to voreen/core/processor
 */
class ProcessorGraphicsItem : public PortOwnerGraphicsItem, public ProcessorObserver {
Q_OBJECT
public:
    //constructor + destructor
    ProcessorGraphicsItem(Processor* processor, NetworkEditor* networkEditor);
    ~ProcessorGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                              
    //---------------------------------------------------------------------------------
    int type() const {return UserTypesProcessorGraphicsItem;}

    //style
    QRectF boundingRect() const;
    QPainterPath shape() const;
protected:
    void initializePaintSettings();
    void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option);
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void layoutChildItems();
    //---------------------------------------------------------------------------------
    //      portownergraphicsitem functions                                            
    //---------------------------------------------------------------------------------
public:
    QList<Processor*> getProcessors() const;
    void saveMeta();
    void loadMeta();
    void renameFinished();
    void toggleProcessorWidget();
    void togglePropertyList();
    
protected:
    void registerPorts();

    //---------------------------------------------------------------------------------
    //      processorobserver functions                                                
    //---------------------------------------------------------------------------------
public:
    void processorWidgetCreated(const Processor* processor);
    void processorWidgetDeleted(const Processor* processor);
    void portsChanged(const Processor* processor);
    void propertiesChanged(const PropertyOwner* owner);

    //---------------------------------------------------------------------------------
    //      processor  functions                                                
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    Processor* getProcessor() const;

    //events
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
private:
    //members
    Processor* processor_; ///core item
};

} //namespace voreen

#endif // VRN_PROCESSORGRAPHICSITEM_H
