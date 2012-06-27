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

#ifndef VRN_AGGREGATIONGRAPHICSITEM_H
#define VRN_AGGREGATIONGRAPHICSITEM_H

#include "rootgraphicsitem.h"

namespace voreen {

class Processor;
class PropertyLink;

/// This type of pair contains two PropertyLinks of both directions for a single arrow
typedef std::pair<const PropertyLink*, const PropertyLink*> ArrowLinkInformation;

/**
 * An AggregationGraphicsItem is the representation for a collection of \sa ProcessorGraphicsItems
 * and other AggregationGraphicsItems. They function as a single unit in the \sa NetworkEditor
 * and are treated like ProcessorGraphicsItems in the form of the shared superclass
 * \sa RootGraphicsItem.</br>
 * In addition to the \sa Processors, this item stores all internal arrows (i.e. arrows leading
 * from one processor in this collection to another processors also in this collection. This
 * holds true for both \sa PortArrowGraphicsItems and \sa LinkArrowGraphicsItems. These lists
 * however must be filled from the class responsible for creating the AggregationGraphicsItem
 * in the first place (e.g. NetworkEditor).
 */
class AggregationGraphicsItem : public RootGraphicsItem {
Q_OBJECT
public:
    /**
     * This construction will create a new AggregationGraphicsItem containing a number of other
     * \sa RootGraphicsItems in a given \sa NetworkEditor. The name of the Aggregation will be
     * set to <code>Aggregation</code> and the ports of the new Aggregation is the sum of all
     * ports of all contained processors.</br>
     * The initial position will be set to the arithmetic mean value of all RootGraphicsItems
     * in the given list.
     * \param items The list of RootGraphicsItems which will be combined in this aggregation
     * \param networkEditor The \sa NetworkEditor which contains both the new
     * AggregationGraphicsItem and each of the \sa RootGraphicsItems in the <code>items</code>
     * list.
     */
    AggregationGraphicsItem(QList<RootGraphicsItem*> items, NetworkEditor* networkEditor);

    /// The destructor will not delete any processor or contained RootGraphicsItem on its own
    virtual ~AggregationGraphicsItem();

    /**
     * Returns a deep copy of all processors and contained RootGraphicsItem of this aggregation.
     * Note: This function does not work at the moment is not in use as for now.
     * \return A new AggregationGraphicsItem, being an exact deep copy of this item.
     */
    RootGraphicsItem* clone() const;

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    enum { Type = UserType + UserTypesAggregationGraphicsItem };

    /// The type of this QGraphicsItem subclass
    int type() const;

    /**
     * Returns whether any contained processor owns the passed \sa Property.
     * \param prop The \sa Property which should be tested
     * \return true if the property is contained in this aggregation, false otherwise
     */
    bool hasProperty(const Property* prop) const;

    /**
     * Returns whether the passed \sa RootGraphicsItem is contained (directly or indirectly via
     * another AggregationGraphicsItem) in this aggregation.
     * \param rootItem The item which should be tested.
     * \return true if the item is contained in this aggregation, false otherwise
     */
    bool contains(RootGraphicsItem* rootItem) const;

    /**
     * Returns whether the passed \sa Processor is contained (directly or indirectly via
     * another AggregationGraphicsItem) in this aggregation.
     * \param processor The \sa Processor which should be tested.
     * \return true if the processor is contained in this aggregation, false otherwise
     */
    bool contains(Processor* processor) const;

    /**
     * Returns a list of all directly contained \sa RootGraphicsItems. This method is not automatically
     * preformed recursivly.
     * \return A list of all directly contained RootGraphicsItems.
     */
    QList<RootGraphicsItem*> getRootGraphicsItems() const;

    /**
     * Adds the passed arrow to the list of internally maintained \sa PortArrowGraphicsItems. This
     * list is meant for arrows leading from one \sa Processor contained in this aggregation to
     * another \sa Processor also contained in this aggregation. The source and destination
     * \sa PortGraphicsItem of the arrow will be hidden if they do not allow multiple connections.
     * Multiple calls with the same arrow will not do anything.
     * \param arrow The PortArrowGraphicsItem which should be added to the internal list.
     */
    void addInternalPortArrow(PortArrowGraphicsItem* arrow);

    /**
     * Returns a list of all internally maintained \sa PortArrowGraphicsItems.
     * \return A list of all internally maintained PortArrowGraphicsItems
     */
    QList<PortArrowGraphicsItem*> getInternalPortArrows() const;

    /**
     * Adds the passed arrow as well as (possibly) both corresponding PropertyLinks to the internally
     * maintained list. An \sa LinkArrowGraphicsItem should be added to this list if and only if it
     * leads from a \sa Property of a \sa Processor contained in this aggregation to another \sa Property
     * of a (possibly different) \sa Processor also contained in this aggregation. This method must not
     * be called multiple times with the same arrow and PropertyLinks.
     * \param arrow The \sa LinkArrowGraphicsItem which should be added to the internal list.
     * \param linkInformation A pair of both PropertyLinks for which the arrow is responsible.
     */
    void addInternalLinkArrow(LinkArrowGraphicsItem* arrow, const ArrowLinkInformation& linkInformation);

    /**
     * Returns a list of all \sa LinkArrowGraphicsItems and their associated \sa PropertyLinks.
     * \return A list of all LinkArrowGraphicsItems and their associated PropertyLinks
     */
    QList<QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> > getInternalLinkArrows() const;

    /**
     * Returns a list of all contained \sa Processors of this aggregation. This method works recursively
     * and returns <b>all</b> \sa Processors.
     * \return A list of all contained processors
     */
    QList<Processor*> getProcessors() const;

    /**
     * This method will be called of the rename processor of the internal \sa TextGraphicsItem is finished.
     * If <code>changeChildItems</code> is set to <code>true</code>, the name of the AggregationGraphicsItem
     * will be passed as a prefix to all contained \sa RootGraphicsItems
     * \param changeChildItems If true, the name will be added as a prefix to the child items. If false,
     * nothing happens with the child items.
     */
    void renameFinished(bool changeChildItems = true);

    //void addAggregationPrefix(const QString& prefix);
    //void removeAggregationPrefix();
    //void removeAllAggregationPrefixes();

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    /// Returns a list of all directly or indirectly contained inports (\sa Port)
    QList<Port*> getInports() const;
    /// Returns a list of all directly or indirectly contained outports (\sa Port)
    QList<Port*> getOutports() const;
    /// Returns a list of all directly or indirectly contained coprocessor inports (\sa CoProcessorPort)
    QList<CoProcessorPort*> getCoProcessorInports() const;
    /// Returns a list of all directly or indirectly contained coprocessor outports (\sa CoProcessorPort)
    QList<CoProcessorPort*> getCoProcessorOutports() const;

private:
    /// the list of all contained child items
    QList<RootGraphicsItem*> childItems_;

    /// the list of all directly or indirectly contained inports
    QList<Port*> inports_;
    /// the list of all directly or indirectly contained outports
    QList<Port*> outports_;
    /// the list of all directly or indirectly contained coprocessor inports
    QList<CoProcessorPort*> coInports_;
    /// the list of all directly or indirectly contained coprocessor outports
    QList<CoProcessorPort*> coOutports_;

    /// the list of all internally maintained \sa PortArrowGraphicsItems
    QList<PortArrowGraphicsItem*> internalPortArrows_;
    /// list list of all internally maintained \sa LinkArrowGraphicsItems with their PropertyLinks
    QList<QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> > internalLinkArrows_;
};

} // namespace

#endif // VRN_AGGREGATIONGRAPHICSITEM_H
