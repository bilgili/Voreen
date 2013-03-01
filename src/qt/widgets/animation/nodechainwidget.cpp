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

#include "voreen/qt/widgets/animation/nodechainwidget.h"

#include <QColor>
#include <QDialog>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QString>

#include <iostream>
#include <sstream>

namespace voreen{

qreal NodeChainItem::wOffset_ = 5;
qreal NodeChainItem::hOffset_ = 7;
std::vector<QString> NodeChainInterpolationItem::availableInterpolationMethods_;

// ================================================================================================

NodeChainScene::NodeChainScene()
    : locked_(false)
{}

void NodeChainScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    event->ignore();

    QGraphicsScene::contextMenuEvent(event);

    if (!event->isAccepted()){
        QMenu sceneMenu;

        sceneMenu.addAction(new QAction(QString("Select all"), &sceneMenu));

        QAction* lockAction;

        if (locked_)
            lockAction = new QAction(QString("Unlock"), &sceneMenu);
        else
            lockAction = new QAction(QString("Lock to ghostplane"), &sceneMenu);

        sceneMenu.addAction(lockAction);

        QAction* selectedSceneAction = sceneMenu.exec(event->screenPos());

        if (selectedSceneAction != 0){
            if (selectedSceneAction->text() == "Select all")
                emit selectAll();
            else if (selectedSceneAction->text() == "Unlock")
                emit setLocked(false);
            else if (selectedSceneAction->text() == "Lock to ghostplane")
                emit setLocked(true);
        }
    }
}

void NodeChainScene::setLockedToGhostplane(bool set){
    locked_ = set;
}

// ================================================================================================

NameChangeDialog::NameChangeDialog(QString oldName, QWidget* parent)
        : QDialog(parent)
        , oldName_(oldName)
{
    currentName_ = new QLineEdit(oldName, this);
    finishRenamingButton_ = new QPushButton(QString("Set Name"), this);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    mainLayout->addWidget(currentName_);
    mainLayout->addWidget(finishRenamingButton_);

    setLayout(mainLayout);

    connect(finishRenamingButton_, SIGNAL(clicked()), this, SLOT(accept()));
}

QString NameChangeDialog::getNewName() const{
    return currentName_->text();
}

// ================================================================================================

NodeChainItem::NodeChainItem(QString text, QColor color, NodeChainView* parent)
    : parent_(parent)
    , text_(text)
    , color_(color)
{
    QGraphicsItem::setCursor(Qt::ArrowCursor);
    setFlags(QGraphicsItem::ItemIsSelectable);
}

QRectF NodeChainItem::boundingRect() const{
    QRectF rect(text_.boundingRect().x() - wOffset_,
    text_.boundingRect().y() - hOffset_,
    text_.boundingRect().width() + 2*wOffset_,
    text_.boundingRect().height() + 2*hOffset_);

    return rect;
}

void NodeChainItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    if (!isSelected())
        painter->setOpacity(0.25);
    else
        painter->setOpacity(1.0);

    QColor endingColor = color_.darker(150);

    QLinearGradient gradient(0, 0, 0, boundingRect().height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, color_);
    gradient.setColorAt(0.4, endingColor);
    gradient.setColorAt(0.6, endingColor);
    gradient.setColorAt(1.0, color_);

    QColor boundingColor = endingColor.darker(150);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(boundingColor), 2.0));

    painter->drawRect(boundingRect());

    text_.paint(painter, option, widget);

    painter->setOpacity(1.0);
}

void NodeChainItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    setSelected(true);
    QMenu nodeMenu;

    nodeMenu.addAction(new QAction(QString("Insert node after"), &nodeMenu));
    nodeMenu.addAction(new QAction(QString("Delete"), &nodeMenu));
    nodeMenu.addAction(new QAction(QString("Rename"), &nodeMenu));

    QAction* selectedNodeAction = nodeMenu.exec(event->screenPos());

    if (selectedNodeAction != 0){
        if (selectedNodeAction->text() == "Insert node after")
            emit insertNodeAfter(text_.toPlainText());
        else if (selectedNodeAction->text() == "Delete")
            emit deleteNode(text_.toPlainText());
        else if (selectedNodeAction->text() == "Rename"){
            NameChangeDialog changeName(text_.toPlainText(), parent_);

            int code = changeName.exec();

            if (code == QDialog::Accepted)
                emit nameChanged(text_.toPlainText(), changeName.getNewName());
        }
    }

    event->accept();
}

void NodeChainItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /* event */){
    emit itemDoubleClicked(text_.toPlainText());
}

QString NodeChainItem::getNodeChainItemIdentifier() const{
    return text_.toPlainText();
}

// ================================================================================================

NodeChainView::NodeChainView(QWidget* parent)
    : QGraphicsView(parent)
{
    initialize();
}

void NodeChainView::initialize(){
    scene_.setObjectName(QString("NodeChainScene"));

    setObjectName(tr("NodeChainView"));
    setWindowTitle(tr("Nodechain"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&scene_);
    setDragMode(QGraphicsView::RubberBandDrag);

    setBackgroundBrush(Qt::blue);
    QLinearGradient gradient(0, 0, 0, 80);

    gradient.setSpread(QGradient::PadSpread);
    gradient.setColorAt(0, QColor(255,255,255,0));
    gradient.setColorAt(0.5, QColor(150,150,150,200));
    gradient.setColorAt(1, QColor(50,0,50,100));

    setBackgroundBrush(gradient);

    currentXPosition_ = 0;
    itemDistance_ = 20;
    nodeChainItemsColor_ = Qt::red;

    connect(&scene_, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    connect(&scene_, SIGNAL(selectAll()), this, SLOT(selectAllNodeChainItems()));

    connect(&scene_, SIGNAL(setLocked(bool)), this, SLOT(setTrackingShotLocked(bool)));
}

void NodeChainView::setInterpolationIdentifiers(const std::vector<std::string>& interpolationIdentifiers){
    NodeChainInterpolationItem::setInterpolationIdentifiers(interpolationIdentifiers);
}

void NodeChainView::clearScene(){
    noUpdateNeeded_ = true;
    scene_.clear();
    currentXPosition_ = 0;
}

bool NodeChainView::addNewNodeChainItem(const std::string& nodeChainItemIdentifier, bool isSelected,
                                        const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& up){
    NodeChainItem* nodeChainItem = new NodeChainItem(QString(nodeChainItemIdentifier.c_str()), nodeChainItemsColor_, this);

    std::stringstream toolTip;

    toolTip << "Position: " << position << std::endl << "Focus: " << focus  << std::endl << "Up: " << up;

    nodeChainItem->setToolTip(QString(toolTip.str().c_str()));

    scene_.addItem(nodeChainItem);

    nodeChainItem->setPos(currentXPosition_, 0);
    nodeChainItem->moveBy(5, 0);

    nodeChainItem->setSelected(isSelected);

    currentXPosition_ += static_cast<int>(nodeChainItem->boundingRect().width() + itemDistance_);

    if (isSelected)
        centerOn(nodeChainItem);

    connect(nodeChainItem, SIGNAL(insertNodeAfter(QString)), this, SLOT(insertNodeAfter(QString)));
    connect(nodeChainItem, SIGNAL(deleteNode(QString)), this, SLOT(deleteNode(QString)));
    connect(nodeChainItem, SIGNAL(itemDoubleClicked(QString)), this, SLOT(itemDoubleClicked(QString)));
    connect(nodeChainItem, SIGNAL(nameChanged(QString,QString)), this, SLOT(nodeChainItemNameChanged(QString,QString)));

    return true;
}

bool NodeChainView::addNewInterpolationBoxItem(const std::string& interpolationIdentifier,
                                               const std::string& foregoingNodeChainItemIdentifier){
    NodeChainInterpolationItem* nodeChainInterpolationItem = new NodeChainInterpolationItem(interpolationIdentifier,
                                                                                            nodeChainItemsColor_,
                                                                                            foregoingNodeChainItemIdentifier);

    scene_.addItem(nodeChainInterpolationItem);

    nodeChainInterpolationItem->setPos(currentXPosition_, 0);

    currentXPosition_ += static_cast<int>(nodeChainInterpolationItem->boundingRect().width() + itemDistance_);

    connect(nodeChainInterpolationItem, SIGNAL(interpolationChanged(QString,QString,QString)),
            this, SLOT(interpolationChanged(QString,QString,QString)));

    return true;
}

bool NodeChainView::trackingShotWasLocked(bool locked){
    scene_.setLockedToGhostplane(locked);

    return true;
}

void NodeChainView::setNodeChainItemsColor(QColor nodeChainItemsColor){
    nodeChainItemsColor_ = nodeChainItemsColor;
}

void NodeChainView::itemDoubleClicked(QString sender){
    emit nodeChainItemDoubleClicked(sender);
}

void NodeChainView::interpolationChanged(QString /* nodeChainInterpolationItemIdentifier */,
                                         QString foregoingNodeChaiItemIdentifier,
                                         QString newInterpolationMethod)
{
    emit interpolationChanged(foregoingNodeChaiItemIdentifier.toStdString(), newInterpolationMethod.toStdString());
}

void NodeChainView::selectionChanged(){
    if (!noUpdateNeeded_){
        std::vector<std::string> cameraNodeIdentifiers;
        QGraphicsItem* nodeItem;

        foreach (nodeItem, scene_.selectedItems()){
            if (NodeChainItem* nodeChainItem = dynamic_cast<NodeChainItem*>(nodeItem))
                cameraNodeIdentifiers.push_back(nodeChainItem->getNodeChainItemIdentifier().toStdString());
        }

       emit itemSelectionChanged(cameraNodeIdentifiers);
   }
}

void NodeChainView::insertNodeAfter(QString nodeChainItemdIdentifier){
    emit insertNodeAfter(nodeChainItemdIdentifier.toStdString());
}

void NodeChainView::deleteNode(QString nodeChainItemdIdentifier){
    emit deleteNode(nodeChainItemdIdentifier.toStdString());
}

void NodeChainView::selectAllNodeChainItems(){
    std::vector<std::string> selection;

    QGraphicsItem* item;

    foreach (item, scene_.items()){
        if (NodeChainItem* nodeChainItem = dynamic_cast<NodeChainItem*>(item))
            selection.push_back(nodeChainItem->getNodeChainItemIdentifier().toStdString());
    }

    emit itemSelectionChanged(selection);
}

void NodeChainView::setTrackingShotLocked(bool set){
    emit lockTrackingShot(set);
}

void NodeChainView::nodeChainItemNameChanged(QString nodeChainItemIdentifier, QString newName){
    emit cameraNodeNameChanged(nodeChainItemIdentifier.toStdString(), newName.toStdString());
}

void NodeChainView::sceneBuildingFinished(){
    noUpdateNeeded_ = false;
}

// ================================================================================================

NodeChainInterpolationItem::NodeChainInterpolationItem(std::string interpolationIdentifier, QColor itemColor,
                                                       std::string foregoingNodeChainItemIdentifier)
        : text_(interpolationIdentifier.c_str())
        , itemColor_(itemColor)
{
    interpolationIdentifier_ = QString(interpolationIdentifier.c_str());
    foregoingNodeChainItemIdentifier_ = QString(foregoingNodeChainItemIdentifier.c_str());

    QFont font = text_.font();
    font.setPixelSize(9);

    text_.setFont(font);
}

QRectF NodeChainInterpolationItem::boundingRect() const{
    return text_.boundingRect();
}

void NodeChainInterpolationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    painter->setOpacity(0.5);

    QColor endingColor = itemColor_.darker(150);

    QLinearGradient gradient(0, 0, 0, boundingRect().height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, endingColor);
    gradient.setColorAt(0.4, itemColor_);
    gradient.setColorAt(0.6, itemColor_);
    gradient.setColorAt(1.0, endingColor);

    QColor boundingColor = endingColor.darker(150);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(boundingColor), 2.0));

    int posX = static_cast<int>(boundingRect().x());
    int posY = static_cast<int>(boundingRect().height() / 2);

    painter->drawRect(boundingRect());
    painter->drawLine(posX-20, posY, posX, posY);

    posX += static_cast<int>(boundingRect().width());

    painter->drawLine(posX, posY, posX+20, posY);

    text_.paint(painter, option, widget);

    painter->setOpacity(1.0);
}

void NodeChainInterpolationItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    QMenu interpolationMenu;

    foreach (QString interpolation, availableInterpolationMethods_)
        interpolationMenu.addAction(new QAction(interpolation, &interpolationMenu));

    QAction* selectedInterpolation = interpolationMenu.exec(event->screenPos());

    if (selectedInterpolation != 0)
        emit interpolationChanged(text_.toPlainText(), foregoingNodeChainItemIdentifier_, selectedInterpolation->text());

    event->accept();
}

void NodeChainInterpolationItem::setInterpolationIdentifiers(const std::vector<std::string>& interpolationIdentifiers){
    availableInterpolationMethods_.clear();

    foreach (std::string interpolationIdentifier, interpolationIdentifiers)
        availableInterpolationMethods_.push_back(QString(interpolationIdentifier.c_str()));
}

void NodeChainInterpolationItem::mousePressEvent(QGraphicsSceneMouseEvent* /* event */){
}
}
