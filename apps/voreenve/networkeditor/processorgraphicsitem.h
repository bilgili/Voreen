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

#ifndef VRN_PROCESSORGRAPHICSITEM_H
#define VRN_PROCESSORGRAPHICSITEM_H

#include "rootgraphicsitem.h"
#include "networkeditor_common.h"

#include "voreen/core/processors/processor.h"

#include <QProgressDialog>
#include <QTime>

namespace voreen {

/**
 * Representation of a Processor in the scene. The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class ProcessorGraphicsItem : public RootGraphicsItem, public ProcessorObserver {
Q_OBJECT
public:
    ProcessorGraphicsItem(Processor* processor, NetworkEditor* networkEditor);
    RootGraphicsItem* clone() const;

    enum { Type = UserType + UserTypesProcessorGraphicsItem };
    int type() const;

    void layoutChildItems();
    bool hasProperty(const Property* prop) const;
    bool contains(RootGraphicsItem* rootItem) const;
    bool contains(Processor* processor) const;
    Processor* getProcessor() const;
    QList<Processor*> getProcessors() const;

    void saveMeta();
    void loadMeta();

    void initializePorts();
    QList<Port*> getInports() const;
    QList<Port*> getOutports() const;
    QList<CoProcessorPort*> getCoProcessorInports() const;
    QList<CoProcessorPort*> getCoProcessorOutports() const;

    void processorWidgetCreated(const Processor* processor);
    void processorWidgetDeleted(const Processor* processor);
    void portsAndPropertiesChanged(const Processor*);

    QList<QAction*> getProcessorWidgetContextMenuActions();

protected:
    //virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

    void toggleProcessorWidget();

public slots:
    void renameFinished(bool changeChildItems = true);

private:
    Processor* processor_;

    QList<Port*> inports_;
    QList<Port*> outports_;
    QList<CoProcessorPort*> coInports_;
    QList<CoProcessorPort*> coOutports_;
};

} //namespace voreen

#endif // VRN_PROCESSORGRAPHICSITEM_H
