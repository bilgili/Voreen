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

#include "propertylinkdialog.h"

#include "linkdialogarrowgraphicsitem.h"
#include "linkdialoggraphicsview.h"
#include "linkdialogprocessorgraphicsitem.h"
#include "linkdialogpropertygraphicsitem.h"
#include "processorgraphicsitem.h"
#include "propertylistgraphicsitem.h"
#include "rootgraphicsitem.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/propertyvector.h"
#include "voreen/core/properties/volumeurlproperty.h"
#include "voreen/core/properties/link/dependencylinkevaluator.h"
#include "voreen/core/properties/link/dependencylinkevaluators.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/link/propertylink.h"
#include "voreen/qt/widgets/enterexitpushbutton.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGraphicsScene>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>


namespace {

/*
 * Helper function: Collects compatible link evaluators for the passed property pair
 * by iterating over all linkevaluatorfactories of all modules.
 */
std::vector<std::pair<std::string, std::string> > getCompatibleEvaluators(
    const voreen::Property* src, const voreen::Property* dest) {

    std::vector<std::pair<std::string, std::string> > result;

    if (!voreen::VoreenApplication::app()) {
        LERRORC("voreenve.PropertyLinkDialog", "VoreenApplication not instantiated");
        return result;
    }
    const std::vector<voreen::VoreenModule*>& modules = voreen::VoreenApplication::app()->getModules();

    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<voreen::LinkEvaluatorFactory*>& factories = modules.at(m)->getLinkEvaluatorFactories();
        for (size_t i=0; i<factories.size(); i++) {
            std::vector<std::pair<std::string, std::string> > evaluators =
                factories.at(i)->getCompatibleLinkEvaluators(src, dest);
            result.insert(result.end(), evaluators.begin(), evaluators.end());
        }
    }

    return result;
}

/*
 * Helper function: Checks if the passed properties are linkable
 * by iterating over all linkevaluatorfactories of all modules.
 */
bool arePropertiesLinkable(const voreen::Property* src, const voreen::Property* dest) {

    if (!voreen::VoreenApplication::app()) {
        LERRORC("voreenve.PropertyLinkDialog", "VoreenApplication not instantiated");
        return false;
    }
    const std::vector<voreen::VoreenModule*>& modules = voreen::VoreenApplication::app()->getModules();

    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<voreen::LinkEvaluatorFactory*>& factories = modules.at(m)->getLinkEvaluatorFactories();
        for (size_t i=0; i<factories.size(); i++) {
            if (factories.at(i)->arePropertiesLinkable(src, dest))
                return true;
        }
    }

    return false;
}

/*
 * Helper function: Creates a link evaluator for the passed type string
 * by iterating over all linkevaluatorfactories of all modules.
 */
voreen::LinkEvaluatorBase* createLinkEvaluator(const std::string& typeString) {

    if (!voreen::VoreenApplication::app()) {
        LERRORC("voreenve.PropertyLinkDialog", "VoreenApplication not instantiated");
        return 0;
    }
    const std::vector<voreen::VoreenModule*>& modules = voreen::VoreenApplication::app()->getModules();

    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<voreen::LinkEvaluatorFactory*>& factories = modules.at(m)->getLinkEvaluatorFactories();
        for (size_t i=0; i<factories.size(); i++) {
            voreen::LinkEvaluatorBase* evaluator = factories.at(i)->createEvaluator(typeString);
            if (evaluator)
                return evaluator;
        }
    }

    return 0;
}

} // namespace anonymous


namespace voreen {

namespace {
    const QString leftArrowIcon(":/qt/icons/arrow-left.png");
    const QString biArrowIcon(":/qt/icons/arrow-leftright.png");
    const QString rightArrowIcon(":/qt/icons/arrow-right.png");

    const qreal leftColumnBasePosition = 0.0;
    const qreal centerColumnBasePosition = 200.0;
    const qreal rightColumnBasePosition = 400.0;

    const qreal scrollMargin = 0.1;  // = a 10% area around the edges is used for scrolling
    const int scrollStepSize = 15;

    const QSize buttonSize = QSize(23, 23);

    const QColor arrowColorNormal = Qt::black;
    const QColor arrowColorDependency = Qt::cyan;

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

    bool evaluatorIsDependencyLinkEvaluator(LinkEvaluatorBase* evaluator) {
        DependencyLinkEvaluator* depLinkEva = dynamic_cast<DependencyLinkEvaluator*>(evaluator);
        return (depLinkEva  != 0);
    }
}

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, RootGraphicsItem* sourceGraphicsItem, RootGraphicsItem* destGraphicsItem,
                                       const QList<PropertyLink*>& existingLinks)
    : QDialog(parent)
    , sourceGraphicsItem_(sourceGraphicsItem)
    , destinationGraphicsItem_(destGraphicsItem)
    , sourcePropertyItem_(0)
    , destinationPropertyItem_(0)
    , propertyLinkModeButton_(0)
    , dependencyLinkModeButton_(0)
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

    propertyLinkModeButton_->click();

    foreach (PropertyLink* link, existingLinks)
        createArrowFromPropertyLink(link);

    sceneSelectionChanged();
    view_->scene()->clearSelection();
}

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
                                       const PropertyLink* link, PropertyLinkDirection selectedButton)
    : QDialog(parent)
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

        if (evaluatorIsDependencyLinkEvaluator(link->getLinkEvaluator()))
            arrow->setNormalColor(arrowColorDependency);
    }
    if ((selectedButton == PropertyLinkDirectionToLeft) || (selectedButton == PropertyLinkDirectionBidirectional)) {
        arrow = new LinkDialogArrowGraphicsItem(destinationPropertyItem_);
        arrow->setDestinationItem(sourcePropertyItem_);
        view_->scene()->addItem(arrow);
        createdArrow(arrow, false);
        if (connectionMap_.contains(arrow))
            connectionMap_[arrow].evaluator = link->getLinkEvaluator();

        if (evaluatorIsDependencyLinkEvaluator(link->getLinkEvaluator()))
            arrow->setNormalColor(arrowColorDependency);
    }

    sceneSelectionChanged();
    previouslyExistingLinks_.append(const_cast<PropertyLink*>(link));
    existingLinksMap_.insert(arrow, const_cast<PropertyLink*>(link));
}

void PropertyLinkDialog::init() {
    scrollTimerUp_.setInterval(25);
    scrollTimerDown_.setInterval(25);
    connect(&scrollTimerUp_, SIGNAL(timeout()), this, SLOT(scrollUpSlot()));
    connect(&scrollTimerDown_, SIGNAL(timeout()), this, SLOT(scrollDownSlot()));

    setWindowTitle(tr("Edit Links"));

    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    // Mode buttons
    QGroupBox* modeContainer = new QGroupBox(this);
    QBoxLayout* modeLayout = new QHBoxLayout(modeContainer);
    propertyLinkModeButton_ = new QPushButton(tr("Property Link Mode"));
    propertyLinkModeButton_->setCheckable(true);
    propertyLinkModeButton_->setFlat(true);
    propertyLinkModeButton_->setFocusPolicy(Qt::NoFocus);
    modeLayout->addWidget(propertyLinkModeButton_);

    dependencyLinkModeButton_ = new QPushButton(tr("Dependency Link Mode"));
    dependencyLinkModeButton_->setCheckable(true);
    dependencyLinkModeButton_->setFlat(true);
    dependencyLinkModeButton_->setFocusPolicy(Qt::NoFocus);
    modeLayout->addWidget(dependencyLinkModeButton_);
    layout->addWidget(modeContainer);

    QButtonGroup* modeGroup = new QButtonGroup(modeContainer);
    modeGroup->addButton(propertyLinkModeButton_);
    modeGroup->addButton(dependencyLinkModeButton_);
    connect(modeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(modeButtonClicked(QAbstractButton*)));

    // GraphicsView
    view_ = new LinkDialogGraphicsView;
    connect(view_->scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
    connect(view_, SIGNAL(deleteActionTriggered(QGraphicsItem*)), this, SLOT(deleteArrow(QGraphicsItem*)));
    layout->addWidget(view_);

    // Link controls
    QWidget* controlContainer = new QWidget;
    QBoxLayout* controlLayout = new QHBoxLayout(controlContainer);
    functionCB_ = new QComboBox;
    functionCB_->setMinimumWidth(250);
    functionCB_->setEnabled(false);
    initCombobox();
    connect(functionCB_, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(comboBoxSelectionChanged(const QString&)));
    controlLayout->addWidget(functionCB_);

    dependencyLinkHistoryContainer_ = new QWidget;
    QBoxLayout* dependencyHistoryLayout = new QHBoxLayout(dependencyLinkHistoryContainer_);
    QLabel* dependencyTextLabel = new QLabel(tr("History Size:"));
    dependencyHistoryLayout->addWidget(dependencyTextLabel);
    dependencyHistoryLengthLabel_ = new QLabel;
    dependencyHistoryLayout->addWidget(dependencyHistoryLengthLabel_);
    controlLayout->addSpacing(5);
    dependencyHistoryLengthSlider_ = new QSlider(Qt::Horizontal, this);
    dependencyHistoryLengthSlider_->setMinimumWidth(140);
    dependencyHistoryLengthSlider_->setMinimum(-1);
    dependencyHistoryLengthSlider_->setMaximum(50);
    dependencyHistoryLayout->addWidget(dependencyHistoryLengthSlider_);
    connect(dependencyHistoryLengthSlider_, SIGNAL(valueChanged(int)), this, SLOT(setDependencyHistoryLengthLabel(int)));
    controlLayout->addWidget(dependencyLinkHistoryContainer_);

    controlLayout->addStretch(1);

    deleteArrowButton_ = new QPushButton(QIcon(":/qt/icons/eraser.png"), "");
    deleteArrowButton_->setIconSize(buttonSize);
    connect(deleteArrowButton_, SIGNAL(clicked(bool)), this, SLOT(deleteSelectedArrow()));
    deleteArrowButton_->setFocusPolicy(Qt::NoFocus);
    deleteArrowButton_->setToolTip(tr("Delete Link"));
    controlLayout->addWidget(deleteArrowButton_);

    controlLayout->addSpacing(15);
    //controlLayout->addStretch(1);

    leftArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-left.png"), "");
    leftArrowButton_->setIconSize(buttonSize);
    leftArrowButton_->setCheckable(true);
    leftArrowButton_->setEnabled(false);
    leftArrowButton_->setToolTip(tr("Direction: Right to Left"));
    controlLayout->addWidget(leftArrowButton_);

    bidirectionalArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-leftright.png"), "");
    bidirectionalArrowButton_->setIconSize(buttonSize);
    bidirectionalArrowButton_->setCheckable(true);
    bidirectionalArrowButton_->setEnabled(false);
    bidirectionalArrowButton_->setToolTip(tr("Direction: Bidirectional"));
    controlLayout->addWidget(bidirectionalArrowButton_);

    rightArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-right.png"), "");
    rightArrowButton_->setIconSize(buttonSize);
    rightArrowButton_->setCheckable(true);
    rightArrowButton_->setEnabled(false);
    rightArrowButton_->setToolTip(tr("Direction: Left to Right"));
    controlLayout->addWidget(rightArrowButton_);

    arrowButtonGroup_ = new QButtonGroup(controlContainer);
    arrowButtonGroup_->addButton(leftArrowButton_);
    arrowButtonGroup_->addButton(bidirectionalArrowButton_);
    arrowButtonGroup_->addButton(rightArrowButton_);
    connect(arrowButtonGroup_, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(controlButtonClicked(QAbstractButton*)));

    layout->addWidget(controlContainer);

    // Second control line
    QWidget* autolinkContainer = new QWidget;
    QBoxLayout* autolinkLayout = new QHBoxLayout(autolinkContainer);
    autolinkName_ = new EnterExitPushButton(tr("Auto-link by name && type"));
    autolinkName_->setFocusPolicy(Qt::NoFocus);
    connect(autolinkName_, SIGNAL(clicked(bool)), this, SLOT(confirmAutoLinks()));
    connect(autolinkName_, SIGNAL(enterEventSignal()), this, SLOT(showAutoLinksByName()));
    connect(autolinkName_, SIGNAL(leaveEventSignal()), this, SLOT(hideAutoLinks()));
    autolinkLayout->addWidget(autolinkName_);

    autolinkLayout->addSpacing(300);

    QPushButton* deleteLinks = new QPushButton(tr("Delete all links"));
    deleteLinks->setFocusPolicy(Qt::NoFocus);
    connect(deleteLinks, SIGNAL(clicked(bool)), this, SLOT(deleteAllLinks()));
    autolinkLayout->addWidget(deleteLinks);
    layout->addWidget(autolinkContainer);

    // Spacing
    //layout->addSpacing(50);

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

            PropertyVector* propVector = dynamic_cast<PropertyVector*>(prop);
            if (propVector) {
                foreach (Property* propVectorProp, propVector->getProperties()) {
                    LinkDialogPropertyGraphicsItem* propItem = new LinkDialogPropertyGraphicsItem(propVectorProp, position);
                    propItem->setDelegate(this);
                    propItem->addPrefix(QString::fromStdString(propVector->getGuiName()));
                    connect(propItem, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
                    connect(propItem, SIGNAL(movedArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(movedArrow(LinkDialogArrowGraphicsItem*)));
                    connect(propItem, SIGNAL(endedArrow()), this, SLOT(endedArrow()));
                    connect(propItem, SIGNAL(deleteArrow(QGraphicsItem*)), this, SLOT(deleteArrow(QGraphicsItem*)));
                    QPoint pos = QPoint(xPosition + procItem->boundingRect().width() / 2.0 - propItem->boundingRect().width() / 2.0 + delta, currentHeight);
                    propItem->setPos(view_->mapToScene(pos));
                    currentHeight += propItem->boundingRect().height();
                    view_->scene()->addItem(propItem);
                }
            }
            else {
                LinkDialogPropertyGraphicsItem* propItem = new LinkDialogPropertyGraphicsItem(prop, position);
                propItem->setDelegate(this);
                connect(propItem, SIGNAL(createdArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(createdArrow(LinkDialogArrowGraphicsItem*)));
                connect(propItem, SIGNAL(movedArrow(LinkDialogArrowGraphicsItem*)), this, SLOT(movedArrow(LinkDialogArrowGraphicsItem*)));
                connect(propItem, SIGNAL(endedArrow()), this, SLOT(endedArrow()));
                connect(propItem, SIGNAL(deleteArrow(QGraphicsItem*)), this, SLOT(deleteArrow(QGraphicsItem*)));
                QPoint pos = QPoint(xPosition + procItem->boundingRect().width() / 2.0 - propItem->boundingRect().width() / 2.0 + delta, currentHeight);
                propItem->setPos(view_->mapToScene(pos));
                currentHeight += propItem->boundingRect().height();
                view_->scene()->addItem(propItem);
            }
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
    //std::vector<std::string> availableFunctions = LinkEvaluatorFactory::getInstance()->listFunctionNames();
    //foreach (std::string function, availableFunctions) {
        ////if (function != "DependencyLink")
            //functionCB_->addItem(QString::fromStdString(function));
    //}

    //int index = functionCB_->findText("id");
    //functionCB_->setCurrentIndex(index);
}

void PropertyLinkDialog::updateCombobox(LinkDialogPropertyGraphicsItem* source, LinkDialogPropertyGraphicsItem* destination) {

    functionCB_->clear();
    std::vector<std::pair<std::string, std::string> > availableFunctions =
        getCompatibleEvaluators(source->getProperty(), destination->getProperty());
    for (std::vector<std::pair<std::string, std::string> >::iterator i = availableFunctions.begin();
        i != availableFunctions.end(); ++i)
    {
        size_t pos0 = i->first.find("Dependency");
        size_t pos1 = i->first.find("dependency");
        if (pos0 == std::string::npos && pos1 == std::string::npos) {
            functionCB_->addItem(QString::fromStdString(i->first));
        }
    }
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
    if (arrow)
        deleteArrow(arrow);
}

void PropertyLinkDialog::sceneSelectionChanged() {
    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();

    if (arrow == 0) {
        arrowButtonGroup_->setExclusive(false);
        functionCB_->setEnabled(false);
        functionCB_->clear();
        dependencyHistoryLengthSlider_->setEnabled(false);
        dependencyHistoryLengthSlider_->setValue(-1);
        dependencyHistoryLengthLabel_->setHidden(true);
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
        leftArrowButton_->setEnabled(true);
        rightArrowButton_->setEnabled(true);

        tgtAssert(connectionMap_.contains(arrow), "arrow not present in the connection map");
        const ConnectionInfo& info = connectionMap_[arrow];

        if (info.evaluator) {
            if (evaluatorIsDependencyLinkEvaluator(info.evaluator)) {
                if (!dependencyLinkModeButton_->isChecked())
                    dependencyLinkModeButton_->click();

                dependencyHistoryLengthSlider_->setEnabled(true);

                DependencyLinkEvaluator* depEval = dynamic_cast<DependencyLinkEvaluator*>(info.evaluator);
                int length = depEval->getHistoryLength();
                dependencyHistoryLengthSlider_->setValue(length);
                setDependencyHistoryLengthLabel(length);
            } else {
                if (!propertyLinkModeButton_->isChecked())
                    propertyLinkModeButton_->click();
                functionCB_->setEnabled(true);
                bidirectionalArrowButton_->setEnabled(true);
                updateCombobox(info.source, info.destination);

                std::string functionName = info.evaluator->getClassName();
                int index = functionCB_->findText(QString::fromStdString(functionName));
                functionCB_->setCurrentIndex(index);
            }
        }

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
}

void PropertyLinkDialog::comboBoxSelectionChanged(const QString& text) {
    //tgtAssert(text.toStdString() != "", "null string arrived");

    LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    //tgtAssert(arrow, "no arrow has been selected but the combobox was active");

    if (arrow) {
        ConnectionInfo& info = connectionMap_[arrow];
        info.evaluator = createLinkEvaluator(text.toStdString());
    }
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

void PropertyLinkDialog::modeButtonClicked(QAbstractButton* button) {
    if (button == propertyLinkModeButton_) {
        dependencyLinkHistoryContainer_->setVisible(false);
        functionCB_->setVisible(true);
        autolinkName_->setEnabled(true);

        //LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
        //if (arrow) {
            //bidirectionalArrowButton_->setEnabled(true);
            //functionCB_->setEnabled(true);
            //ConnectionInfo& info = connectionMap_[arrow];
            //if (evaluatorIsDependencyLinkEvaluator(info.evaluator)) {
                //info.evaluator = LinkEvaluatorFactory::getInstance()->create("LinkEvaluatorId");
                //arrow->setNormalColor(Qt::black);
            //}
        //}
    }
    else {
        functionCB_->setVisible(false);
        dependencyLinkHistoryContainer_->setVisible(true);
        bidirectionalArrowButton_->setEnabled(false);
        autolinkName_->setEnabled(false);

        //LinkDialogArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
        //if (arrow) {
            //dependencyHistoryLengthSlider_->setEnabled(true);
            //ConnectionInfo& info = connectionMap_[arrow];
            //if (!evaluatorIsDependencyLinkEvaluator(info.evaluator)) {
                //info.evaluator = LinkEvaluatorFactory::getInstance()->create("DependencyLink");
            //}
            //int length = dynamic_cast<DependencyLinkEvaluatorBase*>(info.evaluator)->getHistoryLength();
            //dependencyHistoryLengthSlider_->setValue(length);
            //setDependencyHistoryLengthLabel(length);
            //arrow->setNormalColor(Qt::cyan);
            //rightArrowButton_->click();
        //}
    }
}

void PropertyLinkDialog::setDependencyHistoryLengthLabel(int newValue) {
    LinkDialogArrowGraphicsItem* currentArrow = getCurrentlySelectedArrow();
    if (currentArrow) {
        tgtAssert(evaluatorIsDependencyLinkEvaluator(connectionMap_[currentArrow].evaluator), "this method should not be called if the selected arrow is no DependencyLink");
        DependencyLinkEvaluator* depEval = dynamic_cast<DependencyLinkEvaluator*>(connectionMap_[currentArrow].evaluator);
        depEval->setHistoryLength(newValue);
    }

    dependencyHistoryLengthLabel_->setVisible(true);
    if (newValue >= 0)
        dependencyHistoryLengthLabel_->setNum(newValue);
    else
        dependencyHistoryLengthLabel_->setText("inf");
}

LinkDialogArrowGraphicsItem* PropertyLinkDialog::createdArrow(LinkDialogArrowGraphicsItem* arrow, bool bidirectional) {
    tgtAssert(arrow, "passed null pointer");

    foreach (LinkDialogArrowGraphicsItem* item, connectionMap_.keys()) {
        if ((item->getSourceItem() == arrow->getDestinationItem()) && (item->getDestinationItem() == arrow->getSourceItem())) {
            // a reverse arrow already exists
            // => make that arrow bidirectional as long as it isn't a DependencyLink
            ConnectionInfo& info = connectionMap_[item];

            if (evaluatorIsDependencyLinkEvaluator(info.evaluator)) {
                // no bidirectional arrows with DependencyLinks
                view_->scene()->removeItem(arrow);
                delete arrow;
                return 0;
            }
            else {
                info.bidirectional = true;
                item->setBidirectional(true);
                //item->setSelected(true);
                view_->scene()->removeItem(arrow);
                delete arrow;
                return item;
            }
        }
        else if ((item->getSourceItem() == arrow->getSourceItem()) && (item->getDestinationItem() == arrow->getDestinationItem())) {
            // exactly this arrow already exists
            view_->scene()->removeItem(arrow);
            delete arrow;
            return 0;
        }
    }

    ConnectionInfo info;
    info.source = arrow->getSourceItem();
    tgtAssert(arrow->getDestinationItem(), "no destination item present");
    info.destination = arrow->getDestinationItem();

    if (propertyLinkModeButton_->isChecked()) {
        arrow->setNormalColor(Qt::black);
        std::vector<std::pair<std::string, std::string> > availableFunctions =
            getCompatibleEvaluators(info.source->getProperty(), info.destination->getProperty());
        std::string evalType;
        for(std::vector<std::pair<std::string, std::string> >::iterator i=availableFunctions.begin(); i!=availableFunctions.end(); i++) {
            if(evalType == "")
                evalType = i->first;
            else {
                if(i->second == "id")
                    evalType = i->first;

                //prefer other LEs over dep. LEs
                size_t pos0 = evalType.find("Dependency");
                size_t pos1 = evalType.find("dependency");
                if (pos0 != std::string::npos || pos1 != std::string::npos) {
                    evalType = i->first;
                }
            }
        }
        info.evaluator = createLinkEvaluator(evalType);
        if(info.evaluator->arePropertiesLinkable(info.destination->getProperty(), info.source->getProperty()))
            bidirectional = true;
        else
            bidirectional = false;

    }
    else if (dependencyLinkModeButton_->isChecked()) {
        arrow->setNormalColor(arrowColorDependency);
        //std::vector<std::pair<std::string, std::string> > availableFunctions =
        //    getCompatibleEvaluators(info.source->getProperty(), info.destination->getProperty());
        //std::string evalType;
        //for(std::vector<std::pair<std::string, std::string> >::iterator i=availableFunctions.begin(); i!=availableFunctions.end(); i++) {
        //    if(i->second == "DependencyLink")
        //        evalType = i->first;
        //}
        //info.evaluator = createLinkEvaluator(evalType);

        // TODO revise or remove
/*        if (dynamic_cast<const VolumeHandleProperty*>(info.source->getProperty()))
            info.evaluator = new DependencyLinkEvaluatorVolumeHandle;
        else */
            info.evaluator = new DependencyLinkEvaluator;
        bidirectional = false;
    }
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
        QList<ConnectionInfo> currentConnections = connectionMap_.values();
        foreach (const ConnectionInfo& info, currentConnections) {
            bool evaluatorChanged = false;
            bool alreadyExists = false;
            PropertyLink* linkToRemove = 0;
            foreach (PropertyLink* link, previouslyExistingLinks_) {
                bool source = ((link->getSourceProperty() == info.source->getProperty()) || (link->getSourceProperty() == info.destination->getProperty()));
                bool destination = ((link->getDestinationProperty() == info.destination->getProperty()) || (link->getDestinationProperty() == info.source->getProperty()));
                bool evaluator = (link->getLinkEvaluator() == info.evaluator);

                if (source && destination && evaluator)
                    alreadyExists = true;

                if (source && destination && !evaluator) {
                    evaluatorChanged = true;
                    linkToRemove = link;
                }

                if (alreadyExists || evaluatorChanged)
                    break;
            }

            if (evaluatorChanged) {
                emit removeLink(linkToRemove);
                alreadyExists = false;
            }

            if (!alreadyExists) {
                emit createLink(info.source->getProperty(), info.destination->getProperty(), info.evaluator);

                if (info.bidirectional)
                    emit createLink(info.destination->getProperty(), info.source->getProperty(), info.evaluator->create());
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
    if (!arePropertiesLinkable(srcItem->getProperty(), destItem->getProperty()))
        arrow->setNormalColor(Qt::yellow);
    view_->scene()->addItem(arrow);
    arrow = createdArrow(arrow, false);
    existingLinksMap_.insert(arrow, link);

    if (connectionMap_.contains(arrow))
        connectionMap_[arrow].evaluator = link->getLinkEvaluator();

    if (evaluatorIsDependencyLinkEvaluator(link->getLinkEvaluator()))
        arrow->setNormalColor(arrowColorDependency);
    else
        arrow->setNormalColor(Qt::black);

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

                if (leftItem && rightItem) {
                    ConnectionInfo info;
                    if(arePropertiesLinkable(leftProp, rightProp) && arePropertiesLinkable(rightProp, leftProp)) {
                        info.source = leftItem;
                        info.destination = rightItem;
                        info.bidirectional = true;
                    }
                    else if(arePropertiesLinkable(leftProp, rightProp)) {
                        info.source = leftItem;
                        info.destination = rightItem;
                        info.bidirectional = false;
                    }
                    else if(arePropertiesLinkable(rightProp, leftProp)) {
                        info.source = rightItem;
                        info.destination = leftItem;
                        info.bidirectional = false;
                    }

                    addProbationalConnection(info);
                }
            }
        }
    }
}

void PropertyLinkDialog::deleteAllLinks() {
    tgtAssert(probationalConnectionMap_.keys().count() == 0, "there were probational connections");
    foreach (LinkDialogArrowGraphicsItem* arrow, connectionMap_.keys())
        deleteArrow(arrow);

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

bool PropertyLinkDialog::getNewArrowIsBirectional() const {
    if (propertyLinkModeButton_->isChecked())
        return true;
    else
        return false;
}

bool PropertyLinkDialog::allowConnectionBetweenProperties(const Property* p1, const Property* p2) const {
    if (propertyLinkModeButton_->isChecked())
        return arePropertiesLinkable(p1, p2);
    else
        return true;
}

void PropertyLinkDialog::movedArrow(LinkDialogArrowGraphicsItem*) {
    int cursorPos = view_->mapFromGlobal(QCursor::pos()).y();
    int viewHeight = view_->size().height();
    int scrollAreaHeight = static_cast<int>(viewHeight * scrollMargin);

    if (cursorPos <= scrollAreaHeight) {
        if (!scrollTimerUp_.isActive())
            scrollTimerUp_.start();
    }
    else if (cursorPos >= viewHeight - scrollAreaHeight) {
        if (!scrollTimerDown_.isActive())
            scrollTimerDown_.start();
    }
    else {
        if (scrollTimerDown_.isActive())
            scrollTimerDown_.stop();

        if (scrollTimerUp_.isActive())
            scrollTimerUp_.stop();
    }
}

void PropertyLinkDialog::endedArrow() {
    if (scrollTimerDown_.isActive())
        scrollTimerDown_.stop();

    if (scrollTimerUp_.isActive())
        scrollTimerUp_.stop();
}

void PropertyLinkDialog::scrollUpSlot() {
    QScrollBar* scrollBar = view_->verticalScrollBar();
    int oldValue = scrollBar->value();
    int newValue = oldValue - scrollStepSize;
    if (newValue < scrollBar->minimum())
        newValue = scrollBar->minimum();
    scrollBar->setValue(newValue);
}

void PropertyLinkDialog::scrollDownSlot() {
    QScrollBar* scrollBar = view_->verticalScrollBar();
    int oldValue = scrollBar->value();
    int newValue = oldValue + scrollStepSize;
    if (newValue > scrollBar->maximum())
        newValue = scrollBar->maximum();
    scrollBar->setValue(newValue);
}

} // namespace
