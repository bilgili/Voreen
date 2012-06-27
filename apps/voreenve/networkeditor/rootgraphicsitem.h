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

#ifndef VRN_ROOTGRAPHICSITEM_H
#define VRN_ROOTGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>

#include "networkeditor_common.h"
#include "networkeditor.h"
#include "propertylistgraphicsitem.h"
#include "openpropertylistbutton.h"
#include "textgraphicsitem.h"

namespace voreen {

class LinkArrowGraphicsItemStub;
class NetworkEditor;
class PortGraphicsItem;
class Processor;
class PropertyGraphicsItem;

/**
 * This class acts as an abstract base class for any object in the \sa NetworkEditor
 * which contains a \sa Processor. Currently this holds for \sa ProcessorGraphicsItem
 * with only one contained Processor and \sa AggregationGraphicsItem which may hold
 * multiple AggregationGraphicsItems and ProcessorGraphicsItem. Both cases should behave
 * similiarly and therefore most of the code is combined here.</br>
 * The RootGraphicsItem has only ownership over the \sa PortGraphicsItems, not the contained
 * Processors because those are owned by the \sa ProcessorNetwork.
 */
#if (QT_VERSION >= 0x040600)
class RootGraphicsItem : public QGraphicsObject {
#else
class RootGraphicsItem : public QObject, public QGraphicsItem {
#endif
Q_OBJECT

public:
    /**
     * Constructor for an instance. Will create the \sa TextGraphicsItem,
     * \sa PropertyListGraphicsItem and the \sa OpenPropertyListButton but will not call
     * the \sa createChildItems() method. This is left to the concrete subclass.
     * \param networkEditor The network editor to which this RootGraphicsItem belongs
     */
    RootGraphicsItem(NetworkEditor* networkEditor);

    /**
     * Destructor. Will delete all \sa PortGraphicsItem belonging to this RootGraphicsItem
     */
    virtual ~RootGraphicsItem();

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    virtual int type() const = 0;

    /**
     * Will layout all child items and this method should be called if the size of the
     * RootGraphicsItem changes
     */
    virtual void layoutChildItems();

    /**
     * Creates a clone of this instance and returns it. This will be a deep copy so all
     * containing items will be cloned as well.
     * \return A clone of this instance
     */
    virtual RootGraphicsItem* clone() const = 0;

    /**
     * The bounding rect of this PortGraphicsItem. \sa QGraphicsItem::boundingRect()
     * \return The bounding rect
     */
    virtual QRectF boundingRect() const;

    /**
     * Returns a list of all \sa Processors contained in this RootGraphicsItem
     * \return A list of all Processors from this RootGraphicsItem
     */
    virtual QList<Processor*> getProcessors() const = 0;

    /**
     * Sets the name of this RootGraphicsItem to the passed value.
     * \param name The new name of the RootGraphicsItem
     */
    void setName(const QString& name);

    /**
     * Returns the name of this RootGraphicsItem
     * \return The name of this RootGraphicsItem
     */
    QString getName() const;

    /**
     * Adds all passed \sa PropertyGraphicsItems to this RootGraphicsItem and assumes
     * ownership.
     * \param items The PropertyGraphicsItems which should be added to this item
     */
    void addPropertyGraphicsItems(const QList<PropertyGraphicsItem*>& items);

    /**
     * Returns a PropertyGraphicsItem representing the Property. After calling this method
     * the PropertyGraphicsItem exists and is visible in the PropertyListGraphicsItem belonging
     * to this RootGraphicsItem.
     * \param prop The Property for which a PropertyGraphicsItem is returned
     */
    PropertyGraphicsItem* getPropertyGraphicsItem(const Property* prop);

    /**
     * Returns whether any contained processor owns the passed \sa Property.
     * \param prop The \sa Property which should be tested
     * \return true if the property is contained in this aggregation, false otherwise
     */
    virtual bool hasProperty(const Property* prop) const = 0;

    /**
     * Removes the passed PropertyGraphicsItem from the list belonging to this RootGraphicsItem.
     * The item must be present in the PropertyListGraphicsItem however.
     * \param item The item which should be removed
     */
    void removePropertyGraphicsItem(PropertyGraphicsItem* item);

    /**
     * Adds a aggregation prefix to the name of the form:  [prefix] - "old name"</br>
     * Will emit the \sa renameFinished() signal afterwards.
     * \param prefix The prefix which should be added
     */
    virtual void addAggregationPrefix(const QString& prefix);

    /**
     * Removes an aggregation prefix. If no such prefix exists, nothing happens. Will emit
     * the \sa renameFinished() signal afterwards.
     */
    virtual void removeAggregationPrefix();

    /**
     * Removes all aggregation prefixes from the name and emits the \sa renameFinished() signal
     */
    virtual void removeAllAggregationPrefixes();

    /**
     * Changes the first aggregation prefix to the newly passed and emits the \sa renameFinished()
     * signal afterwards.
     * \param prefix The new aggregation prefix
     */
    void changeAggregationPrefix(const QString& prefix);

    /**
     * Informs this instance of a change in the layer. Necessary changes are done here
     * (e.g. informing the child items of the new layer).
     * \param layer The \sa NetworkEditorLayer layer
     */
    virtual void setLayer(NetworkEditorLayer layer);

    /**
     * Returns the current layer
     * \return The current layer
     */
    NetworkEditorLayer currentLayer() const;

    /**
     * Should be called if the renaming of this RootGraphicsItem should start (i.e. the
     * text item should receive the keyboard focus)
     */
    void enterRenameMode();

    /**
     * Returns the docking point for incoming \sa LinkArrowGraphicItems which are pointed
     * towards this RootGraphicsItem.
     * \return The docking point
     */
    QPointF linkArrowDockingPoint() const;

    /**
     * Connects the \sa PortGraphicsItem to the first fitting Port from the destination
     * RootGraphicsItem. If <code>testing</code> is true, the connection is not really created
     * and no side effect happens.
     * \param outport The outport which should be connected to a port from the destination
     * RootGraphicsItem
     * \param dest The destination RootGraphicsitem
     * \param testing Is this just a dry run or connect the ports for real
     */
    bool connect(PortGraphicsItem* outport, RootGraphicsItem* dest, bool testing = false);

    /**
     * Connects the two \sa PortGraphicsItems and returns if the connection was successful. If
     * <code>testing</code> is true, the connection is not really created and no side effect happens.
     * \param outport The outport which should be connected to the inport
     * \param dest The destination port which should be connected
     * \param testing Is this just a dry run or connect the ports for real
     */
    bool connect(PortGraphicsItem* outport, PortGraphicsItem* dest, bool testing = false);

    /**
     * Disconnects the two passed ports from each other
     * \param outport The outport of the connection which should be removed
     * \param inport The inport of the connection which should be removed
     */
    void disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /// Save the meta data
    virtual void saveMeta();
    /// Load and apply the stored meta data. If no meta data exists, nothing happens
    virtual void loadMeta();

    /**
     * Returns a list of all \sa Ports contained by all \sa Processors of this RootGraphicsItem
     * \return A list of all contained ports.
     */
    virtual QList<Port*> getPorts() const;

    /**
     * Returns a list of all inports contained by all \sa Processors of this RootGraphicsItem
     * \return A list of all contained inports
     */
    virtual QList<Port*> getInports() const = 0;

    /**
     * Returns a list of all outports contained by all \sa Processors of this RootGraphicsItem
     * \return A list of all contained outports
     */
    virtual QList<Port*> getOutports() const = 0;

    /**
     * Returns a list of all coprocessor inports contained by all \sa Processors of this RootGraphicsItem
     * \return A list of all contained coprocessor inports
     */
    virtual QList<CoProcessorPort*> getCoProcessorInports() const = 0;

    /**
     * Returns a list of all coprocessor outports contained by all \sa Processors of this RootGraphicsItem
     * \return A list of all contained coprocessor outports
     */
    virtual QList<CoProcessorPort*> getCoProcessorOutports() const = 0;

    /**
     * Returns a list of all \sa PortGraphicsItems owned by this RootGraphicsItem.
     * \return A list of all owned PortGraphicsItem
     */
    QList<PortGraphicsItem*> getPortGraphicsItems() const;

    /**
     * Returns the \sa PortGraphicsItem being responsible for the passed port. If no such
     * PortGraphicsItem exists in this RootGraphicsItem, the return value is 0
     * \param port The port for which the PortGraphicsItem should be returned
     * \return The PortGraphicsItem which contains the passed port
     */
    PortGraphicsItem* getPortGraphicsItem(const Port* port) const;

    /**
     * Tests if the passed RootGraphicsItem is inside this RootGraphicsItem. If <code>rootItem
     * == this</code>, the method will return true.
     * \param rootItem The RootGraphicsItem which should be tested
     * \return true if <code>rootItem == this</code> or this item contains the passed RootGraphicsItem
     */
    virtual bool contains(RootGraphicsItem* rootItem) const = 0;

    /**
     * Tests if the passed Processor is inside this RootGraphicsItem.
     * \param processor The Processor which should be tested
     * \return true if this item contains the passed processor
     */
    virtual bool contains(Processor* processor) const = 0;

public slots:
    /**
     * This slot should be called if the name of this RootGraphicsItem changes and it will induce
     * a re-layout and resize of the childItems. Will trigger an invalidate of the scene.
     */
    virtual void nameChanged();

    /**
     * This slot should be called if the renaming is finished and subclasses need to save their
     * new name to its storage location
     * \param changeChildItems Should the aggregation prefix name of the child items be changed
     * as well?
     */
    virtual void renameFinished(bool changeChildItems = true);

    /**
     * Toggles the visibility of the PropertyListGraphicsItem and changes the layout
     * accordingly.
     */
    void togglePropertyList();

signals:
    /**
     * This signal will be emitted if two \sa RootGraphicsItems are selected and a link should
     * be created between the two RootGraphicsItems
     * \param src In the default implementation, this will always be the <code>this</code> pointer
     * \param dest The destination RootGraphicsItem
     */
    void createLink(RootGraphicsItem* src, RootGraphicsItem* dest);

    /**
     * This signal is emitted if the process of creating an arrow is started no matter what kind
     * of arrow.
     */
    void startedArrow();

    /**
     * This signal is emitted if creation of an arrow is aborted. It's irrelevant whether this
     * process was successful or not
     */
    void endedArrow();

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    /**
     * Creates all necessary child items for this RootGraphicsItem. Will call \sa layoutChildItems()
     * afterwards.
     */
    void createChildItems();

    /**
     * Sets the opacity with which this RootGraphicsItem should paint itself
     * \param opacity The opacity with which this RootGraphicsItem will be painted
     */
    void setOpacity(const qreal opacity);

    /**
     * Returns the spacing this RootGraphicsItem will add horizontally. This spacing will be added
     * only once and <b>not</b> on each side of the \sa TextGraphicsItem.
     * \return The horizontal spacing
     */
    qreal getTextItemSpacing() const;

    /**
     * Returns the number of visible ports this RootGraphicsItem has. This number is calculated
     * by iterating over all ports and checking whether the corresponding PortGraphicsItem is
     * visible or not.
     * \param ports The list of ports for which the number should be calculated
     * \return The number of visible ports in the list
     */
    int getNumberOfVisiblePorts(QList<Port*> ports) const;

    /**
     * Returns the number of visible ports this RootGraphicsItem has. This number is calculated
     * by iterating over all ports and checking whether the corresponding PortGraphicsItem is
     * visible or not.
     * \param ports The list of ports for which the number should be calculated
     * \return The number of visible ports in the list
     */
    int getNumberOfVisiblePorts(QList<CoProcessorPort*> ports) const;

    /**
     * Calculates the minimum width necessary for the number of ports. Because the visibility
     * might change on the fly this width is calculated with all ports in mind, not just the
     * visible ones.
     */
    qreal getMinimumWidthForPorts() const;

    /**
     * Calculates the minimum height necessary for the number of CoProcessorPorts. Because the
     * visibility might change on the fly this width is calculated with all ports in mind, not
     * just the visible ones.
     */
    qreal getMinimumHeightForPorts() const;

    /// The NetworkEditor to which this RootHraphicsItem belongs
    NetworkEditor* networkEditor_;

    /// The complete list of all PortGraphicsItems
    QList<PortGraphicsItem*> portGraphicsItems_;

    /// The TextGraphicsItem which shows (and allows editing of) the name of the item
    TextGraphicsItem textItem_;

    /// In this list all \s PropertyGraphicsItem are collected and layouted
    PropertyListGraphicsItem propertyListItem_;

    /// This button toggles the visibility of the \sa PropertyListGraphicsItem
    OpenPropertyListButton openPropertyListButton_;

    /**
     * This variable stores the arrow that is being created while dragging the mouse,
     * so it is only != 0 between calls to \sa mousePressEvent(QGraphicsSceneMouseEvent*)
     * and \sa mouseReleaseEvent(QGraphicsSceneMouseEvent*)
     */
    LinkArrowGraphicsItemStub* currentLinkArrow_;

    /// The opacity with which this RootGraphicsItem will draw itself
    qreal opacity_;
};


} // namespace

#endif // VRN_ROOTGRAPHICSITEM_H
