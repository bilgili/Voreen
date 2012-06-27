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

#ifndef VRN_PORTGRAPHICSITEM_H
#define VRN_PORTGRAPHICSITEM_H

#include "networkeditor_common.h"
#include "hastooltip.h"
#include "networkeditor.h"

#include <QGraphicsItem>

namespace voreen {

class PortArrowGraphicsItem;
class Port;
class RenderTarget;
class RootGraphicsItem;

/**
 * A PortGraphicsItem is a graphical representation of a \sa Port. It belongs to a specific
 * \sa RootGraphicsItem which needs to layout the PortGraphicsItem by itself. This class reacts
 * to MouseEvents and creates new \sa PortArrowGraphicsItems. It also tests if the connection
 * is allowed and modifies the current arrow accordingly (e.g. color).
 * Instances of this class store the outgoing \sa PortArrowGraphicsItems and maintain a list
 * of connected ports (both incoming and outgoing).
 * If a valid (new) connection is created, it calls the \sa connect method of the RootGraphicsItem.
*/
#if (QT_VERSION >= 0x040600)
class PortGraphicsItem : public QGraphicsObject, public HasToolTip {
#else
class PortGraphicsItem : public QObject, public QGraphicsItem, public HasToolTip {
#endif
Q_OBJECT
    friend class PortArrowGraphicsItem;

public:
    /**
     * Constructor for a PortGraphicsItem instance. Will set this item to accept hover events
     * and makes this item selectable.
     * \param port The port for which a PortGraphicsItem should be created. Must be != 0.
     * \param parent The parent which owns this PortGraphicsItem. Must be != 0.
     */
    PortGraphicsItem(Port* port, RootGraphicsItem* parent);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesPortGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * Returns the port which is represented by the PortGraphicsItem
     * \return The port which is represented by the PortGraphicsItem. This cannot be 0.
     */
    Port* getPort() const;

    /**
     * Return the parent which owns this PortGraphicsItem
     * \return The parent which owns this PortGraphicsItem. This cannot be 0.
     */
    RootGraphicsItem* getParent() const;

    /**
     * Returns a list of all connected PortGraphicsItems.
     * \return A list of all connected PortGraphicsItems. Is always initialized but may contain
     * zero elements.
     */
    const QList<PortGraphicsItem*>& getConnectedPorts();

    /**
     * Add the port to the connected ports list. Multiple calls with the same port
     * will result in multiple occurances of the port.
     * \param port The port which should be added.
     */
    void addConnection(PortGraphicsItem* port);

    /**
     * Removes the port from the connected ports list
     * \param port The port which should be removed
     * \return true if the port was present in the list, false otherwise
     */
    bool removeConnection(PortGraphicsItem* port);

    /**
     * Checks if the stored port is an outport or not. Is equal to calling the same method
     * on the port itself.
     * \return true if the port is an outport, false otherwise
     */
    bool isOutport() const;

    /**
     * Adds an arrow to the internally maintained arrow list. Multiple calls to this methods
     * will result in multiple appearances. The ownership however is not transferred
     * \param arrow The arrow which should be added to this PortGraphicsItem
     */
    void addArrow(PortArrowGraphicsItem* arrow);

    /**
     * Remove the arrow from the internally maintained arrow list. As the ownership does not
     * lie with the PortGraphicsItem, the arrow is not deleted after removing it.
     * \param arrow The PortArrowGraphicsItem which should be removed from the list
     * \return true if the arrow existed in the list and was successfully deleted, false otherwise
     */
    bool removeArrow(PortArrowGraphicsItem* arrow);

    /**
     * Returns a reference to the list of outgoing arrows of this PortGraphicsItem
     * \return A constant reference of the arrow list maintained by this PortGraphicsitem
     */
    const QList<PortArrowGraphicsItem*>& getArrowList();

    /**
     * \sa QGraphicsItem::update(const QRectF& rect)
     * \param rect The area which should be repainted
     */
    void update(const QRectF& rect = QRectF());

    /**
     * The bounding rect of this PortGraphicsItem. \sa QGraphicsItem::boundingRect()
     * \return The bounding rect
     */
    QRectF boundingRect() const;

    /**
     * Informs this instance of a change in the layer. Necessary changes are done here
     * (e.g. disabling the creation of new arrows, if the dataflow layer is not
     * selected).
     * \param layer The \sa NetworkEditorLayer layer
     */
    void setLayer(NetworkEditorLayer layer);

    /**
     * Returns the tooltip containing information about the contained port. \sa HasToolTip::tooltip()
     * \return The tooltip which is ready to be added to a QGraphicsScene
     */
    QGraphicsItem* tooltip() const;

    /**
     * Sets the currently active \sa PortArrowGraphicsItem. Used by the PortArrowGraphicsItem itself
     * to set itself as the current arrow if the destination is changed
     * \param arrow The new PortArrowGraphicsItem
     */
    void setCurrentArrow(PortArrowGraphicsItem* arrow);

signals:
    /**
     * This signal is emitted if a new arrow is created. This signal only appears
     * once for each arrow.
     */
    void startedArrow();

    /**
     * This signal is emitted if the creation of a new arrow is finished. This signal
     * is triggered whether the operation was successful or not.
     */
    void endedArrow();

protected:
    /**
     * Returns the color for the internal port type.</br>
     * <ul>
     * <li>VolumePort = Red</li>
     * <li>CoProcessorPort = Green</li>
     * <li>RenderPort = Blue</li>
     * <li>TextPort = Cyan</li>
     * <li>VolumeCollectionPort = Magenta</li>
     * <li>GeometryPort = Yellow</li>
     * <li>Unknown Port type = gray</li>
     * </ul>
     * \return The color for the internal stored port
     */
    QColor getColor() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
    /**
     * Graphical tooltip for texture containers in network editor
     */
    class TCTooltip : public QGraphicsRectItem {
    public:
        TCTooltip(QGraphicsItem* parent = 0);
        TCTooltip(const QRectF& rect, QGraphicsItem* parent = 0);
        TCTooltip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = 0);
        ~TCTooltip();

        void initialize(RenderTarget* rt);

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        QImage* image_;
    };

    /**
     * Contains all outgoing arrows which belong to this PortGraphicsItem. This list is modified
     * via \sa addArrow(PortArrowGraphicsItem*) , \sa removeArrow(PortArrowGraphicsItem*) and
     * \sa getArrowList()
     */
    QList<PortArrowGraphicsItem*> arrowList_;

    /**
     * Contains all connected ports. This includes ports both at the end of outgoing arrows
     * and at the end of incoming arrows. This list is modified via \sa addConnection(PortGraphicsItem*),
     * \sa removeConnection(PortGraphicsItem*) and \sa getConnectedPorts()
     */
    QList<PortGraphicsItem*> connectedPorts_;

    /**
     * The parent which has ownership of this PortGraphicsItem and takes care of layouting.
     */
    RootGraphicsItem* parent_;

    /**
     * The port represented by this PortGraphicsItem.
     */
    Port* port_;

    /**
     * This variable stores the arrow that is being created while dragging the mouse,
     * so it is only != 0 between calls to \sa mousePressEvent(QGraphicsSceneMouseEvent*)
     * and \sa mouseReleaseEvent(QGraphicsSceneMouseEvent*)
     */
    PortArrowGraphicsItem* currentArrow_;
};

} // namespace

#endif // VRN_PORTGRAPHICSITEM_H
