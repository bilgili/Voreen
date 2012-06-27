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

#ifdef VRN_WITH_PYTHON
// Python header must be included before other system headers
#include <Python.h>
#endif

#include "propertylinkdialog.h"

#include "linkdialogarrowgraphicsitem.h"
#include "linkdialoggraphicsview.h"
#include "linkdialogprocessorgraphicsitem.h"
#include "linkdialogpropertygraphicsitem.h"
#include "processorgraphicsitem.h"
#include "propertylistgraphicsitem.h"
#include "rootgraphicsitem.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/propertylink.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGraphicsScene>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

namespace voreen {

namespace {
    const QString leftArrowIcon(":/voreenve/icons/arrow-left.png");
    const QString biArrowIcon(":/voreenve/icons/arrow-leftright.png");
    const QString rightArrowIcon(":/voreenve/icons/arrow-right.png");

    const qreal leftColumnBasePosition = 0.0;
    const qreal centerColumnBasePosition = 200.0;
    const qreal rightColumnBasePosition = 400.0;

    qreal positionForColumnPosition(ColumnPosition position) {
        switch (position) {
        case ColumnPositionLeft:
            return leftColumnBasePosition;
        case ColumnPositionCenter:
            return centerColumnBasePosition;
        case ColumnPositionRight:
            return rightColumnBasePosition;
        default:
            tgtAssert(false, "shouldn't get here");
            return 0.0;
        }
    }
}

EnterExitPushButton::EnterExitPushButton(QWidget* parent)
    : QPushButton(parent)
{}

EnterExitPushButton::EnterExitPushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{}

EnterExitPushButton::EnterExitPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : QPushButton(icon, text, parent)
{}


void EnterExitPushButton::enterEvent(QEvent* event) {
    emit enterEventSignal();
    QPushButton::enterEvent(event);
}

void EnterExitPushButton::leaveEvent(QEvent* event) {
    emit leaveEventSignal();
    QPushButton::leaveEvent(event);
}

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, RootGraphicsItem* sourceGraphicsItem, RootGraphicsItem* destGraphicsItem,
                                       const QList<PropertyLink*>& existingLinks)
    : QDialog(parent)
    , sourceGraphicsItem_(sourceGraphicsItem)
    , destinationGraphicsItem_(destGraphicsItem)
    , isEditing_(false)
    , sourcePropertyItem_(0)
    , destinationPropertyItem_(0)
    , leftArrowButton_(0)
    , bidirectionalArrowButton_(0)
    , rightArrowButton_(0)
    , previouslyExistingLinks_(existingLinks)
{
    tgtAssert(sourceGraphicsItem, "null pointer");
    tgtAssert(destGraphicsItem, "null pointer");

    init();

    if (sourceGraphicsItem != destGraphicsItem) {
        initGraphicsItem(sourceGraphicsItem, ColumnPositionLeft);
        initGraphicsItem(destGraphicsItem, ColumnPositionRight);
    }
    else
        initGraphicsItem(sourceGraphicsItem, ColumnPositionCenter);

    foreach (PropertyLink* link, existingLinks)
        createArrowFromPropertyLink(link);

    sceneSelectionChanged();
}

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
                                       const PropertyLink* link, PropertyLinkDirection selectedButton)
    : QDialog(parent)
    , isEditing_(true)
{
    tgtAssert(sourceGraphicsItem, "null pointer");
    tgtAssert(sourceGraphicsItem->getRootGraphicsItem(), "PropertyGraphicsItem has no ProcessorGraphicsItem");
    tgtAssert(sourceGraphicsItem->getProperty(), "PropertyGraphicsItem has no Property");
    sourceGraphicsItem_ = sourceGraphicsItem->getRootGraphicsItem();

    tgtAssert(destGraphicsItem, "null pointer");
    tgtAssert(destGraphicsItem->getRootGraphicsItem(), "PropertyGraphicsItem has no ProcessorGraphicsItem");
    tgtAssert(destGraphicsItem->getProperty(), "PropertyGraphicsItem has no Property");
    destinationGraphicsItem_ = destGraphicsItem->getRootGraphicsItem();

    init();
    initPropertyItems(sourceGraphicsItem, destGraphicsItem);

    LinkDialogArrowGraphicsItem* arrow = 0;
    if ((selectedButton == PropertyLinkDirectionToRight) || (selectedButton == PropertyLinkDirectionBidirectional)) {
        arrow = new LinkDialogArrowGraphicsItem(sourcePropertyItem_);
        arrow->setDestinationItem(destinationPropertyItem_);
        view_->scene()->addItem(arrow);
        createdArrow(arrow, false);
        connectionMap_[arrow].evaluator = link->getLinkEvaluator();
    }
    if ((selectedButton == PropertyLinkDirectionToLeft) || (selectedButton == PropertyLinkDirectionBidirectional)) {
        arrow = new LinkDialogArrowGraphicsItem(destinationPropertyItem_);
        arrow->setDestinationItem(sourcePropertyItem_);
        view_->scene()->addItem(arrow);
        createdArrow(arrow, false);
        if (connectionMap_.contains(arrow))
            connectionMap_[arrow].evaluator = link->getLinkEvaluator();
    }

    sceneSelectionChanged();

    previouslyExistingLinks_.append(const_cast<PropertyLink*>(link));
    existingLinksMap_.insert(arrow, const_cast<PropertyLink*>(link));
}

void PropertyLinkDialog::init() {
    setWindowTitle(tr("Property link dialog"));

    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    // Autolinkingbuttons
    QWidget* autolinkContainer = new QWidget;
    QBoxLayout* autolinkLayout = new QHBoxLayout(autolinkContainer);
    QPushButton* autolinkName = new EnterExitPushButton(tr("Auto-link by name && type"));
    autolinkName->setFocusPolicy(Qt::NoFocus);
    connect(autolinkName, SIGNAL(clicked(bool)), this, SLOT(confirmAutoLinks()));
    connect(autolinkName, SIGNAL(enterEventSignal()), this, SLOT(showAutoLinksByName()));
    connect(autolinkName, SIGNAL(leaveEventSignal()), this, SLOT(hideAutoLinks()));
    autolinkLayout->addWidget(autolinkName);

    autolinkLayout->addSpacing(300);

    QPushButton* deleteLinks = new QPushButton(tr("Delete all links"));
    deleteLinks->setFocusPolicy(Qt::NoFocus);
    connect(deleteLinks, SIGNAL(clicked(bool)), this, SLOT(deleteAllLinks()));
    autolinkLayout->addWidget(deleteLinks);
    layout->addWidget(autolinkContainer);

    // GraphicsView
    view_ = new LinkDialogGraphicsView;
    connect(view_->scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
    connect(view_, SIGNAL(deleteActionTriggered(QGraphicsItem*)), this, SLOT(deleteArrow(QGraphicsItem*)));
    layout->addWidget(view_);

    // Combobox
    QWidget* controlContainer = new QWidget;
    QBoxLayout* controlLayout = new QHBoxLayout(controlContainer);
    functionCB_ = new QComboBox;
    functionCB_->setMinimumWidth(250);
    functionCB_->setEnabled(false);
    initCombobox();
    connect(functionCB_, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(comboBoxSelectionChanged(const QString&)));
    controlLayout->addWidget(functionCB_);

    controlLayout->addSpacing(200);

    deleteArrowButton_ = new QPushButton(QIcon(":/voreenve/icons/eraser.png"), "");
    deleteArrowButton_->setIconSize(QSize(23,23));
    deleteArrowButton_->setFocusPolicy(Qt::NoFocus);
    connect(deleteArrowButton_, SIGNAL(clicked(bool)), this, SLOT(deleteSelectedArrow()));
    controlLayout->addWidget(deleteArrowButton_);

    QWidget* buttonContainer = new QWidget;
    QBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer);
    leftArrowButton_ = new QPushButton(QIcon(":/voreenve/icons/arrow-left.png"), "");
    leftArrowButton_->setIconSize(QSize(23,23));
    leftArrowButton_->setCheckable(true);
    leftArrowButton_->setFlat(true);
    leftArrowButton_->setEnabled(false);
    buttonLayout->addWidget(leftArrowButton_);

    bidirectionalArrowButton_ = new QPushButton(QIcon(":/voreenve/icons/arrow-leftright.png"), "");
    bidirectionalArrowButton_->setIconSize(QSize(23,23));
    bidirectionalArrowButton_->setCheckable(true);
    bidirectionalArrowButton_->setFlat(true);
    bidirectionalArrowButton_->setEnabled(false);
    buttonLayout->addWidget(bidirectionalArrowButton_);

    rightArrowButton_ = new QPushButton(QIcon(":/voreenve/icons/arrow-right.png"), "");
    rightArrowButton_->setIconSize(QSize(23,23));
    rightArrowButton_->setCheckable(true);
    rightArrowButton_->setFlat(true);
    rightArrowButton_->setEnabled(false);
    buttonLayout->addWidget(rightArrowButton_);
    controlLayout->addWidget(buttonContainer);

    arrowButtonGroup_ = new QButtonGroup(controlContainer);
    arrowButtonGroup_->addButton(leftArrowButton_);
    arrowButtonGroup_->addButton(bidirectionalArrowButton_);
    arrowButtonGroup_->addButton(rightArrowButton_);
    connect(arrowButtonGroup_, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(controlButtonClicked(QAbstractButton*)));

    layout->addWidget(controlContainer);

    // Spacing
    layout->addSpacing(50);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(createPropertyLink()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttonBox);
}

void PropertyLinkDialog::initGraphicsItem(RootGraphicsItem* item, ColumnPosition position) {
    qreal xPosition = positionForColumnPosition(position);

    qreal currentHeight = 0.0;
    qreal horizontalMedian = 0.0;

    QList<Processor*> processors = item->getProcessors();
    foreach (Processor* processor, processors) {
        LinkDialogProcessorGraphicsItem* procItem = new LinkDialogProcessorGraphicsItem(QString::fromStdString(processor->getName()));
        procItem->setPos(view_->mapToScene(QPoint(xPosition, currentHeight)));
        view_->scene()->addItem(procItem);

        if (horizontalMedian == 0.0)
            horizontalMedian = procItem->boundingRect().x() + procItem->boundingRect().width() / 2.0;

        qreal thisMedian = procItem->boundingRect().x() + procItem->boundingRect().width() / 2.0;
        qreal delta = horizontalMedian - thisMedian;
        procItem->moveBy(delta, 0.0);

        currentHeight += procItem->boundingRect().height();

        foreach (Property* prop, processor->getProperties()) {
            if (dynamic_cast<EventPropertyBase*>(prop))
                continue;

            LinkDialogPropertyGraphicsItem* propItem = new LinkDialogPropertyGraphicsItem(prop, position);
            connect(propItem, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
            QPoint pos = QPoint(xPosition + procItem->boundingRect().width() / 2.0 - propItem->boundingRect().width() / 2.0 + delta, currentHeight);
            propItem->setPos(view_->mapToScene(pos));
            currentHeight += propItem->boundingRect().height();
            view_->scene()->addItem(propItem);
        }

        currentHeight += 10.0;
    }
}

void PropertyLinkDialog::initPropertyItems(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem) {
    if (sourceItem->getRootGraphicsItem() == destinationItem->getRootGraphicsItem()) {
        RootGraphicsItem* rootItem = sourceItem->getRootGraphicsItem();

        tgtAssert(rootItem->getProcessors().count() == 1, "Only processor graphics item should get here");

        Processor* processor = rootItem->getProcessors()[0];

        LinkDialogProcessorGraphicsItem* procItem = new LinkDialogProcessorGraphicsItem(QString::fromStdString(processor->getName()));
        procItem->setPos(view_->mapToScene(QPoint(centerColumnBasePosition, 0.0)));
        view_->scene()->addItem(procItem);

        sourcePropertyItem_ = new LinkDialogPropertyGraphicsItem(sourceItem->getProperty(), ColumnPositionCenter);
        connect(sourcePropertyItem_, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
        QPoint pos = QPoint(centerColumnBasePosition + procItem->boundingRect().width() / 2.0 - sourcePropertyItem_->boundingRect().width() / 2.0, procItem->boundingRect().height());
        sourcePropertyItem_->setPos(view_->mapToScene(pos));
        view_->scene()->addItem(sourcePropertyItem_);

        destinationPropertyItem_ = new LinkDialogPropertyGraphicsItem(sourceItem->getProperty(), ColumnPositionCenter);
        connect(destinationPropertyItem_, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
        pos = QPoint(centerColumnBasePosition + procItem->boundingRect().width() / 2.0 - destinationPropertyItem_->boundingRect().width() / 2.0, procItem->boundingRect().height() + sourcePropertyItem_->boundingRect().height());
        destinationPropertyItem_->setPos(view_->mapToScene(pos));
        view_->scene()->addItem(destinationPropertyItem_);

    }
    else {
        Processor* sourceProcessor = static_cast<Processor*>(sourceItem->getProperty()->getOwner());
        Processor* destinationProcessor = static_cast<Processor*>(destinationItem->getProperty()->getOwner());
        tgtAssert(sourceProcessor, "source property owner was no processor");
        tgtAssert(destinationProcessor, "destination property owner was no processor");

        LinkDialogProcessorGraphicsItem* sourceProcItem = new LinkDialogProcessorGraphicsItem(QString::fromStdString(sourceProcessor->getName()));
        sourceProcItem->setPos(view_->mapToScene(QPoint(leftColumnBasePosition, 0.0)));
        view_->scene()->addItem(sourceProcItem);

        sourcePropertyItem_ = new LinkDialogPropertyGraphicsItem(sourceItem->getProperty(), ColumnPositionLeft);
        connect(sourcePropertyItem_, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
        QPoint pos = QPoint(leftColumnBasePosition + sourceProcItem->boundingRect().width() / 2.0 - sourcePropertyItem_->boundingRect().width() / 2.0, sourceProcItem->boundingRect().height());
        sourcePropertyItem_->setPos(view_->mapToScene(pos));
        view_->scene()->addItem(sourcePropertyItem_);

        LinkDialogProcessorGraphicsItem* destinationProcItem = new LinkDialogProcessorGraphicsItem(QString::fromStdString(destinationProcessor->getName()));
        destinationProcItem->setPos(view_->mapToScene(QPoint(rightColumnBasePosition, 0.0)));
        view_->scene()->addItem(destinationProcItem);

        destinationPropertyItem_ = new LinkDialogPropertyGraphicsItem(destinationItem->getProperty(), ColumnPositionRight);
        connect(destinationPropertyItem_, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
        pos = QPoint(rightColumnBasePosition + destinationProcItem->boundingRect().width() / 2.0 - destinationPropertyItem_->boundingRect().width() / 2.0, destinationProcItem->boundingRect().height());
        destinationPropertyItem_->setPos(view_->mapToScene(pos));
        view_->scene()->addItem(destinationPropertyItem_);
    }
}

void PropertyLinkDialog::initCombobox() {
    std::vector<std::string> availableFunctions = LinkEvaluatorFactory::getInstance()->listFunctionNames();
    foreach (std::string function, availableFunctions)
        functionCB_->addItem(QString::fromStdString(function));

    int index = functionCB_->findText("id");
    functionCB_->setCurrentIndex(index);
}

LinkDialogArrowGraphicsItem* PropertyLinkDialog::getCurrentlySelectedArrow() const {
    QList<QGraphicsItem*> items = view_->scene()->selectedItems();

    if (items.count() == 0)
        return 0;

    tgtAssert(items.count() == 1, "more than one item was selected");
    tgtAssert(items[0]->type() == LinkDialogArrowGraphicsItem::Type, "wrong type was selected");

    LinkDialogArrowGraphicsItem* arrow = qgraphicsitem_cast<LinkDialogArrowGraphicsItem*>(items[0]);
    tgtAssert(connectionMap_.find(arrow) != connectionMap_.end(), "arrow did not exist in connection map");

    return arrow;
}

PropertyLinkDialog::PropertyLinkDirection PropertyLinkDialog::getDirection(LinkDialogArrowGraphicsItem* arrow) const {
    ConnectionInfo info = connectionMap_[arrow];
    if (info.bidirectional)
        return PropertyLinkDirectionBidirectional;
    else {
        if (arrow->getDestinationItem()->getPosition() == ColumnPositionLeft)
            return PropertyLinkDirectionToLeft;
        else
            return PropertyLinkDirectionToRight;
    }
}

void PropertyLinkDialog::deleteArrow(QGraphicsItem* arrow) {
    LinkDialogArrowGraphicsItem* item = qgraphicsitem_cast<LinkDialogArrowGraphicsItem*>(arrow);
    if (item) {
        if (existingLinksMap_.contains(item)) {
            QList<PropertyLink*> links = existingLinksMap_.values(item);
            tgtAssert(links.count() == 1 || links.count() == 2, "At this point only 1 or 2 links should exist for a single arrow");
            existingLinksMap_.remove(item);

            foreach (PropertyLink* link, links) {
                previouslyExistingLinks_.removeOne(link);
                emit removeLink(link);
            }
        }
        connectionMap_.remove(item);
        delete item;
        view_->scene()->invalidate();
    }
    else {
        deleteSelectedArrow();
    }
}

void PropertyLinkDialog::deleteSelectedArrow() {
    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    deleteArrow(arrow);
}

void PropertyLinkDialog::sceneSelectionChanged() {
    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();

    if (arrow == 0) {
        functionCB_->setEnabled(false);
        arrowButtonGroup_->setExclusive(false);
        leftArrowButton_->setChecked(false);
        leftArrowButton_->setEnabled(false);
        bidirectionalArrowButton_->setChecked(false);
        bidirectionalArrowButton_->setEnabled(false);
        rightArrowButton_->setChecked(false);
        rightArrowButton_->setEnabled(false);
        arrowButtonGroup_->setExclusive(true);
        return;
    }
    else {
        functionCB_->setEnabled(true);
        leftArrowButton_->setEnabled(true);
        bidirectionalArrowButton_->setEnabled(true);
        rightArrowButton_->setEnabled(true);
    }

    const ConnectionInfo& info = connectionMap_[arrow];

    std::string functionName = info.evaluator->name();
    int index = functionCB_->findText(QString::fromStdString(functionName));
    functionCB_->setCurrentIndex(index);

    switch (getDirection(arrow)) {
    case PropertyLinkDirectionToLeft:
        leftArrowButton_->setChecked(true);
        break;
    case PropertyLinkDirectionToRight:
        rightArrowButton_->setChecked(true);
        break;
    case PropertyLinkDirectionBidirectional:
        bidirectionalArrowButton_->setChecked(true);
        break;
    }
}

void PropertyLinkDialog::comboBoxSelectionChanged(const QString& text) {
    tgtAssert(text.toStdString() != "", "null string arrived");

    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    tgtAssert(arrow, "no arrow has been selected but the combobox was active");

    ConnectionInfo& info = connectionMap_[arrow];
    info.evaluator = LinkEvaluatorFactory::getInstance()->createLinkEvaluator(text.toStdString());
}

void PropertyLinkDialog::controlButtonClicked(QAbstractButton* button) {
    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    ConnectionInfo& info = connectionMap_[arrow];
    PropertyLinkDirection formerDirection = getDirection(arrow);

    if (button == leftArrowButton_) {
        if (formerDirection == PropertyLinkDirectionToRight) {
            LinkDialogPropertyGraphicsItem* tmp = info.destination;
            info.destination = info.source;
            info.source = tmp;
            arrow->swapItems();
        }
        else if (formerDirection == PropertyLinkDirectionBidirectional) {
            info.bidirectional = false;
            arrow->setBidirectional(false);

            if (!arrow->getDestinationItem()->getPosition() == ColumnPositionLeft) {
                LinkDialogPropertyGraphicsItem* tmp = info.destination;
                info.destination = info.source;
                info.source = tmp;
                arrow->swapItems();
            }
        }
    }
    else if (button == bidirectionalArrowButton_) {
        info.bidirectional = true;
        arrow->setBidirectional(true);
    }
    else if (button == rightArrowButton_) {
        if (formerDirection == PropertyLinkDirectionToLeft) {
            LinkDialogPropertyGraphicsItem* tmp = info.destination;
            info.destination = info.source;
            info.source = tmp;
            arrow->swapItems();
        }
        else if (formerDirection == PropertyLinkDirectionBidirectional) {
            info.bidirectional = false;
            arrow->setBidirectional(false);
            if (arrow->getDestinationItem()->getPosition() == ColumnPositionLeft) {
                LinkDialogPropertyGraphicsItem* tmp = info.destination;
                info.destination = info.source;
                info.source = tmp;
                arrow->swapItems();
            }
        }
    }
    else {
        tgtAssert(false, "should not get here");
    }

    view_->scene()->invalidate();
}

LinkDialogArrowGraphicsItem* PropertyLinkDialog::createdArrow(LinkDialogArrowGraphicsItem* arrow, bool bidirectional) {
    tgtAssert(arrow, "passed null pointer");

    foreach (LinkDialogArrowGraphicsItem* item, connectionMap_.keys()) {
        if ((item->getSourceItem() == arrow->getDestinationItem()) && (item->getDestinationItem() == arrow->getSourceItem())) {
            ConnectionInfo& info = connectionMap_[item];
            info.bidirectional = true;
            item->setBidirectional(true);
            //item->setSelected(true);
            view_->scene()->removeItem(arrow);
            delete arrow;
            return item;
        }
        else if ((item->getSourceItem() == arrow->getSourceItem()) && (item->getDestinationItem() == arrow->getDestinationItem())) {
            view_->scene()->removeItem(arrow);
            delete arrow;
            return 0;
        }
    }

    ConnectionInfo info;
    info.source = arrow->getSourceItem();
    tgtAssert(arrow->getDestinationItem(), "no destination item present");
    info.destination = arrow->getDestinationItem();
    info.evaluator = LinkEvaluatorFactory::getInstance()->createLinkEvaluator("id");
    info.bidirectional = bidirectional;
    connectionMap_.insert(arrow, info);

    view_->scene()->clearSelection();
    arrow->setSelected(true);
    return arrow;
}

void PropertyLinkDialog::createPropertyLink() {
    if (connectionMap_.values().count() == 0)
        emit createLink(0,0,0);
    else {
        foreach (ConnectionInfo info, connectionMap_.values()) {
            bool isLegit = true;
            foreach (PropertyLink* link, previouslyExistingLinks_) {
                if ((link->getSourceProperty() == info.source->getProperty()) && (link->getDestinationProperty() == info.destination->getProperty())) {
                    isLegit = false;
                    break;
                }
            }

            if (isLegit)
                emit createLink(info.source->getProperty(), info.destination->getProperty(), info.evaluator);

            if (info.bidirectional) {
                isLegit = true;
                foreach (PropertyLink* link, previouslyExistingLinks_) {
                    if ((link->getDestinationProperty() == info.source->getProperty()) && (link->getSourceProperty() == info.destination->getProperty())) {
                        isLegit = false;
                        break;
                    }
                }
                if (isLegit)
                    emit createLink(info.destination->getProperty(), info.source->getProperty(), info.evaluator);
            }
        }
    }
    emit accept();
}

void PropertyLinkDialog::createArrowFromPropertyLink(PropertyLink* link) {
    LinkDialogPropertyGraphicsItem* srcItem = 0;
    foreach (QGraphicsItem* item, view_->scene()->items()) {
        LinkDialogPropertyGraphicsItem* propItem = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item);
        if (propItem == 0)
            continue;

        if (propItem->getProperty() == link->getSourceProperty()) {
            srcItem = propItem;
            break;
        }
    }
    tgtAssert(srcItem, "no source property item found");

    LinkDialogPropertyGraphicsItem* destItem = 0;
    foreach (QGraphicsItem* item, view_->scene()->items()) {
        LinkDialogPropertyGraphicsItem* propItem = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item);
        if (propItem == 0)
            continue;

        ColumnPosition thisPos = srcItem->getPosition();
        ColumnPosition otherPos = propItem->getPosition();
        if ((propItem->getProperty() == link->getDestinationProperty()) && ((thisPos != otherPos) || ((thisPos == ColumnPositionCenter) && (otherPos == ColumnPositionCenter)))) {
            destItem = propItem;
            break;
        }
    }
    tgtAssert(destItem, "no destination property item found");

    LinkDialogArrowGraphicsItem* arrow = new LinkDialogArrowGraphicsItem(srcItem, destItem, false);
    if (!PropertyLink::arePropertiesLinkable(srcItem->getProperty(), destItem->getProperty()))
        arrow->setNormalColor(Qt::yellow);
    view_->scene()->addItem(arrow);
    arrow = createdArrow(arrow, false);
    existingLinksMap_.insert(arrow, link);

    if (connectionMap_.contains(arrow))
        connectionMap_[arrow].evaluator = link->getLinkEvaluator();
}

// ------------------------------------------------------------------------------------------------
// autolink methods
// ------------------------------------------------------------------------------------------------

void PropertyLinkDialog::addProbationalConnection(ConnectionInfo info) {
    LinkDialogArrowGraphicsItem* arrow = new LinkDialogArrowGraphicsItem(info.source, info.destination, info.bidirectional);
    arrow->setNormalColor(Qt::lightGray);
    view_->scene()->addItem(arrow);
    probationalConnectionMap_.insert(arrow, info);
}

void PropertyLinkDialog::showAutoLinksByName() {
    tgtAssert(probationalConnectionMap_.keys().count() == 0, "there were probational connections");

    foreach (Property* leftProp, sourceGraphicsItem_->getProcessors()[0]->getProperties()) {
        foreach (Property* rightProp, destinationGraphicsItem_->getProcessors()[0]->getProperties()) {
            if (leftProp->getGuiName() == rightProp->getGuiName() && (typeid(*leftProp) == typeid(*rightProp))) {
                LinkDialogPropertyGraphicsItem* leftItem = 0;
                LinkDialogPropertyGraphicsItem* rightItem = 0;
                foreach (QGraphicsItem* item, view_->scene()->items()) {
                    LinkDialogPropertyGraphicsItem* propItem = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item);
                    if (propItem == 0)
                        continue;

                    if (propItem->getProperty() == leftProp)
                        leftItem = propItem;
                    else if (propItem->getProperty() == rightProp)
                        rightItem = propItem;

                    if (leftItem && rightItem)
                        break;
                }

                ConnectionInfo info;
                info.source = leftItem;
                info.destination = rightItem;
                info.bidirectional = true;
                addProbationalConnection(info);
            }
        }
    }
}

void PropertyLinkDialog::deleteAllLinks() {
    tgtAssert(probationalConnectionMap_.keys().count() == 0, "there were probational connections");
    foreach (LinkDialogArrowGraphicsItem* arrow, connectionMap_.keys())
        delete arrow;

    connectionMap_.clear();
}

void PropertyLinkDialog::hideAutoLinks() {
    foreach (LinkDialogArrowGraphicsItem* arrow, probationalConnectionMap_.keys()) {
        delete arrow;
    }

    probationalConnectionMap_.clear();
}

void PropertyLinkDialog::confirmAutoLinks() {
    foreach (LinkDialogArrowGraphicsItem* arrow, probationalConnectionMap_.keys()) {
        arrow->setNormalColor(Qt::black);
        createdArrow(arrow, probationalConnectionMap_[arrow].bidirectional);
    }

    probationalConnectionMap_.clear();
}

} // namespace
