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

#ifndef VRN_NODECHAINWIDGET_H
#define VRN_NODECHAINWIDGET_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include "tgt/vector.h"

#include <string>
#include <vector>

class QAction;
class QColor;
class QLineEdit;
class QPushButton;

namespace voreen {

class NodeChainScene : public QGraphicsScene {
Q_OBJECT
public:
    NodeChainScene();

    void setLockedToGhostplane(bool set);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    bool locked_;

signals:
    void selectAll();
    void setLocked(bool locked);
};

// ================================================================================================

class NodeChainView : public QGraphicsView {
Q_OBJECT
public:
    NodeChainView(QWidget* parent);

    bool addNewNodeChainItem(const std::string& nodeChainItemIdentifier, bool isSelected,
                             const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& up);
    bool addNewInterpolationBoxItem(const std::string& interpolationBoxIdentifier,
                                    const std::string& foregoingNodeChainItemIdentifier);

    bool trackingShotWasLocked(bool locked);

    void setInterpolationIdentifiers(const std::vector<std::string>& interpolationIdentifiers);

    void setNodeChainItemsColor(QColor nodeChainItemsColor);
    void clearScene();
    void sceneBuildingFinished();

signals:
    void nodeChainItemClicked(QString nodeChainItemIdentifier, bool additive);
    void nodeChainItemDoubleClicked(QString nodeChainItemIdentifier);

    void interpolationChanged(std::string foregoingNodeIdentifier, std::string interpolationIdentifier);
    void itemSelectionChanged(std::vector<std::string> selectedCameraNodeNames);

    void insertNodeAfter(std::string cameraNodeIdentifier);
    void deleteNode(std::string cameraNodeIdentifier);
    void lockTrackingShot(bool lock);
    void cameraNodeNameChanged(std::string cameraNodeIdentifier, std::string newName);

protected:
    void initialize();

    NodeChainScene scene_;
    QColor nodeChainItemsColor_;

    int currentXPosition_;
    int itemDistance_;
    bool noUpdateNeeded_;

private slots:
    void itemDoubleClicked(QString sender);

    void interpolationChanged(QString nodeChainInterpolationItemIdentifier,
                              QString foregoingNodeChainItemIdentifier,
                              QString newInterpolationMethod);

    void selectionChanged();

    void insertNodeAfter(QString nodeChainItemIdentifier);
    void deleteNode(QString nodeChainItemIdentifier);

    void selectAllNodeChainItems();
    void setTrackingShotLocked(bool set);

    void nodeChainItemNameChanged(QString nodeChainItemIdentifier, QString newName);
};

// ================================================================================================

class NameChangeDialog : public QDialog {
public:
    NameChangeDialog(QString oldName, QWidget* parent = 0);

    QString getNewName() const;

protected:
    QString oldName_;
    QLineEdit* currentName_;
    QPushButton* finishRenamingButton_;
};

// ================================================================================================

#if (QT_VERSION >= 0x040600)
class NodeChainItem : public QGraphicsObject {
#else
class NodeChainItem : public QObject, public QGraphicsItem {
#endif
Q_OBJECT

public:
    NodeChainItem(QString text, QColor color, NodeChainView* parent);

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

    QString getNodeChainItemIdentifier() const;

signals:
    void insertNodeAfter(QString nodeChainItemIdentifier);
    void deleteNode(QString nodeChainItemIdentifier);
    void itemDoubleClicked(QString nodeChainItemIdentifier);
    void nameChanged(QString nodeChainItemIdentifier, QString newName);

protected:
    static qreal wOffset_;
    static qreal hOffset_;

    NodeChainView* parent_;

    QGraphicsTextItem text_;
    QColor color_;

private slots:
};

// ================================================================================================

#if (QT_VERSION >= 0x040600)
class NodeChainInterpolationItem : public QGraphicsObject {
#else
class NodeChainInterpolationItem : public QObject, public QGraphicsItem {
#endif
Q_OBJECT
public:
    NodeChainInterpolationItem(std::string interpolationIdentifier, QColor itemColor, std::string foregoingNodeChainItemIdentifier);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

    QRectF boundingRect() const;

    static void setInterpolationIdentifiers(const std::vector<std::string>& interpolationIdentifiers);

signals:
    void interpolationChanged(QString nodeChainInterpolationItemIdentifier,
                              QString foregoingNodeChainItemIdentifier,
                              QString newInterpolationmethod);

protected:
    QGraphicsTextItem text_;
    QColor itemColor_;

    QString interpolationIdentifier_;
    QString foregoingNodeChainItemIdentifier_;

    static std::vector<QString> availableInterpolationMethods_;
};
}

#endif // VRN_NODECHAINWIDGET_H
