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

#include "../networkeditor.h"
#include "../networkeditor_settings.h"

#include "tgt/assert.h"

//core
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/link/propertylink.h"
#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/voreenapplication.h"

//gi
#include "../graphicitems/core/propertygraphicsitem.h"
#include "../graphicitems/core/portownergraphicsitem.h"
#include "../graphicitems/core/portgraphicsitem.h"
#include "../graphicitems/connections/propertylinkarrowgraphicsitem.h"
#include "utils/splitgraphicsview.h"

#include "voreen/qt/widgets/enterexitpushbutton.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QButtonGroup>
#include <QLabel>
#include <QKeyEvent>
#include <QAbstractButton>
#include <QComboBox>
#include <QMessageBox>

namespace voreen {

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, PortOwnerGraphicsItem* source, PortOwnerGraphicsItem* destination)
    : QDialog(parent)
    , sourceView_(0)
    , destinationView_(0)
    , lastPressedDirectionButton_(0)
    , areEvaluatorsChained_(true)
    , sourceItem_(source)
    , destinationItem_(destination)
{
    tgtAssert(source, "null pointer");
    tgtAssert(destination, "null pointer");

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (source != destination) {
        init(false);
        initPropertyList(sourceView_, source->getPropertyList());
        initPropertyList(destinationView_, destination->getPropertyList());
        connect(sourceView_,SIGNAL(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)),this,SLOT(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)));
        connect(destinationView_,SIGNAL(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)),this,SLOT(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)));
        source->getPropertyList()->setVisible(true);
        destination->getPropertyList()->setVisible(true);
    }
    else {
        init(true);
        initPropertyList(sourceView_, source->getPropertyList());
        connect(sourceView_,SIGNAL(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)),this,SLOT(linkProperties(PropertyGraphicsItem*, PropertyGraphicsItem*)));
        source->getPropertyList()->setVisible(true);
    }

    sceneSelectionChanged();
    sourceView_->scene()->clearSelection();
    if(destinationView_) destinationView_->scene()->clearSelection();
}

PropertyLinkDialog::~PropertyLinkDialog() {
    undoAllModifications();
    if(sourceItem_)
        sourceItem_->getPropertyList()->setScene(dynamic_cast<NetworkEditor*>(parent())->scene());
    if(sourceItem_ != destinationItem_)
        destinationItem_->getPropertyList()->setScene(dynamic_cast<NetworkEditor*>(parent())->scene());
}

//---------------------------------------------------------------------------------------------------------------
//                  view interaction                                                                             
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkDialog::sceneSelectionChanged() {
    PropertyLinkArrowGraphicsItem* arrow = getCurrentlySelectedArrow();

    if (arrow == 0) {
        arrowButtonGroup_->setExclusive(false);
        arrowButtonGroup_->blockSignals(true);
        evaluatorSrcDstBox_->blockSignals(true);
        evaluatorDstSrcBox_->blockSignals(true);
        evaluatorSrcDstBox_->setEnabled(false);
        evaluatorDstSrcBox_->setEnabled(false);
        evaluatorSrcDstBox_->clear();
        evaluatorDstSrcBox_->clear();
        leftArrowButton_->setChecked(false);
        leftArrowButton_->setEnabled(false);
        bidirectionalArrowButton_->setChecked(true);
        bidirectionalArrowButton_->setEnabled(false);
        rightArrowButton_->setChecked(false);
        rightArrowButton_->setEnabled(false);
        evaluatorSrcDstBox_->blockSignals(false);
        evaluatorDstSrcBox_->blockSignals(false);
        arrowButtonGroup_->blockSignals(false);
        arrowButtonGroup_->setExclusive(true);
        chainButton_->blockSignals(true);
        chainButton_->setEnabled(false);
        toggleChainButton(true);
        chainButton_->blockSignals(false);
        return;
    }
    else {
        updateEvaluatorBoxes(arrow);
        updateArrowButtonGroup(arrow);
    }
}

PropertyLinkArrowGraphicsItem* PropertyLinkDialog::getCurrentlySelectedArrow() const {
    QList<QGraphicsItem*> items = sourceView_->scene()->selectedItems();

    if (items.count() == 0)
        return 0;

    if(items.count() == 1 && items[0]->type() == UserTypesPropertyLinkArrowGraphicsItem)
        return qgraphicsitem_cast<PropertyLinkArrowGraphicsItem*>(items[0]);
    
    return 0;
}

void PropertyLinkDialog::setCurrentlySelectedArrow(PropertyGraphicsItem* src, PropertyGraphicsItem* dst) {
    //src item has to be in sourceView_
    tgtAssert(sourceView_->getPropertyList()->getAllPropertyGraphicsItems().contains(src),"src is not from sourceView");
    //clear selection
    sourceView_->scene()->blockSignals(true);
    sourceView_->scene()->clearSelection();
    sourceView_->scene()->blockSignals(false);
    //find arrow
    foreach(PropertyLinkArrowGraphicsItem* item, src->getDestinationLinkList())
        if(item->getSourceItem() == dst){
            item->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
            item->setSelected(true);
            return;
    }
    foreach(PropertyLinkArrowGraphicsItem* item, src->getSourceLinkList())
        if(item->getDestinationItem() == dst){
            item->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
            item->setSelected(true);
            return;
        }
    //should not get here
    sourceView_->scene()->clearSelection();
}


//---------------------------------------------------------------------------------------------------------------
//                  layout                                                                                       
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkDialog::init(bool oneView) {

    setWindowTitle(tr("Edit Links"));

    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    // GraphicsViews
    QWidget* viewContainer = new QWidget(this);
    QBoxLayout* viewLayout = new QHBoxLayout(viewContainer);
    viewLayout->setSpacing(0);
    if(oneView){
        sourceView_ = new SplitGraphicsView(this);
        sourceView_->setSplitPosition(SplitGraphicsView::CENTER);
        viewLayout->addWidget(sourceView_);
        connect(sourceView_->scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
    } else {
        sourceView_ = new SplitGraphicsView(this);
        sourceView_->setSplitPosition(SplitGraphicsView::LEFT);
        destinationView_ = new SplitGraphicsView(this);
        destinationView_->setSplitPosition(SplitGraphicsView::RIGHT);
        sourceView_->setBuddyView(destinationView_);
        destinationView_->setBuddyView(sourceView_);
        viewLayout->addWidget(sourceView_);
        viewLayout->addWidget(destinationView_);
        connect(sourceView_->scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
        connect(destinationView_->scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
    }
    layout->addWidget(viewContainer);


    //QAction* deleteAction = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    //connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteAction()));
    //contextMenu_.addAction(deleteAction);
    //connect(view_, SIGNAL(deleteActionTriggered(QGraphicsItem*)), this, SLOT(deleteArrow(QGraphicsItem*)));

    //layout under the view
    QWidget* controlContainer = new QWidget(this);
    QBoxLayout* controlLayout = new QHBoxLayout(controlContainer);
    
    // Link controls
    QWidget* selectedLinkContainer = new QWidget;
    QBoxLayout* selectedLinkLayout = new QVBoxLayout(selectedLinkContainer);

    QLabel* evalLabel = new QLabel("Selected Link Evaluator:");
    selectedLinkLayout->addWidget(evalLabel);

    QWidget* evaluatorContainer = new QWidget(this);
    QGridLayout* evaluatorLayout = new QGridLayout(evaluatorContainer);

    QLabel* evaluatorSrcDstLabel = new QLabel(QString("Evaluator -> :"),this);
    evaluatorLayout->addWidget(evaluatorSrcDstLabel,0,0);

    evaluatorSrcDstBox_ = new QComboBox(this);
    evaluatorSrcDstBox_->setMinimumWidth(200);
    evaluatorSrcDstBox_->setEnabled(false);
    connect(evaluatorSrcDstBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(srcDstEvaluatorBoxSelectionChanged(int)));
    evaluatorLayout->addWidget(evaluatorSrcDstBox_,0,1);
    //selectedLinkLayout->addWidget(evaluatorSrcDstContainer);

    //QWidget* evaluatorDstSrcContainer = new QWidget(this);
    //QBoxLayout* evaluatorDstSrcLayout = new QHBoxLayout(evaluatorDstSrcContainer);

    QLabel* evaluatorDstSrcLabel = new QLabel(QString("Evaluator <- :"),this);
    evaluatorLayout->addWidget(evaluatorDstSrcLabel,1,0);

    evaluatorDstSrcBox_ = new QComboBox(this);
    evaluatorDstSrcBox_->setMinimumWidth(200);
    evaluatorDstSrcBox_->setEnabled(false);
    connect(evaluatorDstSrcBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(dstSrcEvaluatorBoxSelectionChanged(int)));
    evaluatorLayout->addWidget(evaluatorDstSrcBox_,1,1);
    
    chainButton_ = new QPushButton(QIcon(":/voreenve/icons/chain.png"), "");
    chainButton_->setIconSize(QSize(40,40));
    chainButton_->setEnabled(false);
    connect(chainButton_,SIGNAL(clicked()),this,SLOT(toggleChainButton()));
    chainButton_->setToolTip(tr("Evaluators are Connected"));
    evaluatorLayout->addWidget(chainButton_,0,2,2,1);

    selectedLinkLayout->addWidget(evaluatorContainer);

    QLabel* direLabel = new QLabel("Direction of Selected Link:");
    selectedLinkLayout->addWidget(direLabel);

    QWidget* directionButtonContainer = new QWidget;
    QBoxLayout* directionButtonLayout = new QHBoxLayout(directionButtonContainer);
    
    leftArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-left.png"), "");
    leftArrowButton_->setIconSize(QSize(40,40));
    leftArrowButton_->setCheckable(true);
    leftArrowButton_->setToolTip(tr("Direction: Right to Left"));
    directionButtonLayout->addWidget(leftArrowButton_);

    bidirectionalArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-leftright.png"), "");
    bidirectionalArrowButton_->setIconSize(QSize(40,40));
    bidirectionalArrowButton_->setCheckable(true);
    bidirectionalArrowButton_->setToolTip(tr("Direction: Bidirectional"));
    directionButtonLayout->addWidget(bidirectionalArrowButton_);

    rightArrowButton_ = new QPushButton(QIcon(":/qt/icons/arrow-right.png"), "");
    rightArrowButton_->setIconSize(QSize(40,40));
    rightArrowButton_->setCheckable(true);
    rightArrowButton_->setToolTip(tr("Direction: Left to Right"));
    directionButtonLayout->addWidget(rightArrowButton_);

    arrowButtonGroup_ = new QButtonGroup(controlContainer);
    arrowButtonGroup_->addButton(leftArrowButton_);
    arrowButtonGroup_->addButton(bidirectionalArrowButton_);
    arrowButtonGroup_->addButton(rightArrowButton_);
    connect(arrowButtonGroup_, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(directionButtonClicked(QAbstractButton*)));

    selectedLinkLayout->addWidget(directionButtonContainer);

    /*deleteArrowButton_ = new QPushButton(QIcon(":/qt/icons/eraser.png"), "");
    deleteArrowButton_->setIconSize(buttonSize);
    connect(deleteArrowButton_, SIGNAL(clicked(bool)), this, SLOT(deleteSelectedArrow()));
    deleteArrowButton_->setFocusPolicy(Qt::NoFocus);
    deleteArrowButton_->setToolTip(tr("Delete Link"));
    controlLayout->addWidget(deleteArrowButton_);*/

    controlLayout->addWidget(selectedLinkContainer);

    controlLayout->addStretch(1);

    // Second control line
    QWidget* allLinksContainer = new QWidget;
    QBoxLayout* allLinksLayout = new QVBoxLayout(allLinksContainer);

        //create all
    QLabel* createAllLabel = new QLabel(QString("Auto-Link by Name & Type:"),this);
    allLinksLayout->addWidget(createAllLabel);

    QWidget* createAllContainer = new QWidget;
    QBoxLayout* createAllLayout = new QHBoxLayout(createAllContainer);

    EnterExitPushButton* autolinkProcessor = new EnterExitPushButton(tr("Processor Links"), this);
    autolinkProcessor->setFocusPolicy(Qt::NoFocus);
    connect(autolinkProcessor, SIGNAL(clicked(bool)), this, SLOT(confirmAutoLinks()));
    connect(autolinkProcessor, SIGNAL(enterEventSignal()), this, SLOT(showProcessorAutoLinks()));
    connect(autolinkProcessor, SIGNAL(leaveEventSignal()), this, SLOT(hideAutoLinks()));
    createAllLayout->addWidget(autolinkProcessor);
    EnterExitPushButton* autolinkPort = new EnterExitPushButton(tr("Port Links"), this);
    autolinkPort->setFocusPolicy(Qt::NoFocus);
    connect(autolinkPort, SIGNAL(clicked(bool)), this, SLOT(confirmAutoLinks()));
    connect(autolinkPort, SIGNAL(enterEventSignal()), this, SLOT(showPortAutoLinks()));
    connect(autolinkPort, SIGNAL(leaveEventSignal()), this, SLOT(hideAutoLinks()));
    createAllLayout->addWidget(autolinkPort);

    allLinksLayout->addWidget(createAllContainer);

        //delete all
    QLabel* deleteAllLabel = new QLabel(QString("Delete all Links:"),this);
    allLinksLayout->addWidget(deleteAllLabel);

    QWidget* deleteAllContainer = new QWidget;
    QBoxLayout* deleteAllLayout = new QHBoxLayout(deleteAllContainer);

    QPushButton* deleteProcessorLinks = new QPushButton(tr("Processor Links"));
    deleteProcessorLinks->setFocusPolicy(Qt::NoFocus);
    connect(deleteProcessorLinks, SIGNAL(clicked(bool)), this, SLOT(deleteAllProcessorLinks()));
    deleteAllLayout->addWidget(deleteProcessorLinks);
    QPushButton* deletePortLinks = new QPushButton(tr("Port Links"));
    deletePortLinks->setFocusPolicy(Qt::NoFocus);
    connect(deletePortLinks, SIGNAL(clicked(bool)), this, SLOT(deleteAllPortLinks()));
    deleteAllLayout->addWidget(deletePortLinks);
    allLinksLayout->addWidget(deleteAllContainer);

    controlLayout->addWidget(allLinksContainer);

    layout->addWidget(controlContainer);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(applyAllModifications()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttonBox);
}

void PropertyLinkDialog::initPropertyList(SplitGraphicsView* view, PropertyListGraphicsItem* list) {

    view->setPropertyList(list);
    list->setHeaderMode(PropertyListGraphicsItem::BIG_HEADER);
    if(list->currentLayer() == NetworkEditorLayerCameraLinking) {
        list->setPropertyVisibleMode(PropertyListGraphicsItem::SHOW_CAMERA_PROPERTIES);
    } else {
        list->setPropertyVisibleMode(PropertyListGraphicsItem::SHOW_ALL_PROPERTIES);
    }
    list->setPropertyVisibleModifier(PropertyListGraphicsItem::HIDE_NO_PROPERTIES);

    if(list->getParent() == sourceItem_)
        list->setLinkArrowMode(PropertyListGraphicsItem::SHOW_FRIEND_LIST_ARROWS, destinationItem_->getPropertyList());
    else
        list->setLinkArrowMode(PropertyListGraphicsItem::SHOW_FRIEND_LIST_ARROWS, sourceItem_->getPropertyList());

    //setPosition
    switch(view->getSplitPosition()){
    case SplitGraphicsView::CENTER:
        view->centerOn(0.0, 0.0);
        list->setPos(-list->boundingRect().width()/2.0, view->mapToScene(0.0,view->rect().top()).y());
        break;
    case SplitGraphicsView::LEFT:
        view->centerOn(-175.0, 0.0);
        list->setPos(-175.0 - list->boundingRect().width()/2.0, view->mapToScene(0.0,view->rect().top()).y());
        break;
    case SplitGraphicsView::RIGHT:
        view->centerOn(175.0, 0.0);
        list->setPos(175.0 - list->boundingRect().width()/2.0, view->mapToScene(0.0,view->rect().top()).y());
        break;
    default:
        tgtAssert(false, "Should not get here");
        break;
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  evaluator functions and slots                                                                
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkDialog::updateEvaluatorBoxes(PropertyLinkArrowGraphicsItem* arrow) {
    tgtAssert(arrow,"PropertyLinkDialog::updateCombobox: null pointre passed");
    //pre settings
    evaluatorSrcDstBox_->blockSignals(true);
    evaluatorDstSrcBox_->blockSignals(true);
    evaluatorSrcDstBox_->setEnabled(true);
    evaluatorDstSrcBox_->setEnabled(true);
    evaluatorSrcDstBox_->clear();
    evaluatorDstSrcBox_->clear();

    PropertyGraphicsItem* src = 0;
    PropertyGraphicsItem* dst = 0;
    bool isLeftArrow = false;


    //setting src as "left" item
    if(sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getSourceItem())){
        src = arrow->getSourceItem();
        dst = arrow->getDestinationItem();
    } else {
        dst = arrow->getSourceItem();
        src = arrow->getDestinationItem();
        isLeftArrow = true;
    }

    if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {//bi
        updateSrcDstEvaluatorBox(src,dst);
        updateDstSrcEvaluatorBox(dst,src);
        chainButton_->setEnabled(true);
        if(getEvaluator(src,dst)->getClassName() == getEvaluator(dst,src)->getClassName()) {
            toggleChainButton(true);
            chainButton_->setEnabled(true);
        } else {
            toggleChainButton(false);
            chainButton_->setEnabled(false);
        }
    } else { //uni
        if(isLeftArrow) {
            updateSrcDstEvaluatorBox(0,0);
            updateDstSrcEvaluatorBox(dst,src);
        } else {
            updateSrcDstEvaluatorBox(src,dst);
            updateDstSrcEvaluatorBox(0,0);
        }
        chainButton_->setEnabled(false);
        toggleChainButton(false);
    }

    //post settings
    evaluatorSrcDstBox_->blockSignals(false);
    evaluatorDstSrcBox_->blockSignals(false);
}

void PropertyLinkDialog::updateSrcDstEvaluatorBox(PropertyGraphicsItem* src,PropertyGraphicsItem* dst) {
    //set ->
    evaluatorSrcDstBox_->blockSignals(true);
    chainButton_->blockSignals(true);
    evaluatorSrcDstBox_->clear();
    evaluatorSrcDstBox_->setEnabled(true);

    //is empty?
    if(!src || !dst){
        evaluatorSrcDstBox_->setEnabled(false);
        toggleChainButton(false);
        chainButton_->setEnabled(false);
        chainButton_->blockSignals(false);
        evaluatorSrcDstBox_->blockSignals(false);
        return;
    }

    //get Evaluators
    std::vector<std::pair<std::string, std::string> > availableEvaluators =
            src->getProperty()->getCompatibleEvaluators(dst->getProperty());

    if(availableEvaluators.empty()){
        evaluatorSrcDstBox_->setEnabled(false);
        toggleChainButton(false);
        chainButton_->setEnabled(false);
        chainButton_->blockSignals(false);
        evaluatorSrcDstBox_->blockSignals(false);
        return;
    }

    //set Box
    for (std::vector<std::pair<std::string, std::string> >::iterator i = availableEvaluators.begin();
            i != availableEvaluators.end(); ++i) {
                evaluatorSrcDstBox_->addItem(QString::fromStdString(i->second),QVariant(QString::fromStdString(i->first)));
    }

    //set Evaluator
    LinkEvaluatorBase* sdEval = 0;
    LinkEvaluatorBase* dsEval = 0;
    if(!(sdEval = getEvaluator(src,dst))){
        bool hasOppEval = false;
        if(dsEval = getEvaluator(dst,src)) {
            for (std::vector<std::pair<std::string, std::string> >::iterator i = availableEvaluators.begin(); i != availableEvaluators.end(); ++i) {
                if(i->first == dsEval->getClassName()) {
                    evaluatorSrcDstBox_->setCurrentIndex(evaluatorSrcDstBox_->findText(QString::fromStdString(i->second)));
                    addEvaluatorModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(i->first)));
                    hasOppEval = true;
                    break;
                }
            }
        }
        if(!hasOppEval) {
            evaluatorSrcDstBox_->setCurrentIndex(evaluatorSrcDstBox_->findText(QString::fromStdString(availableEvaluators[0].second)));
            addEvaluatorModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(availableEvaluators[0].first)));
        }
    } else {
        evaluatorSrcDstBox_->setCurrentIndex(evaluatorSrcDstBox_->findText(QString::fromStdString(
            sdEval->getGuiName())));
    }

    //are evaluators chained?
    if(evaluatorDstSrcBox_->currentIndex() == -1) {
        toggleChainButton(true);
        chainButton_->setEnabled(false);
    } else
    if(qVariantValue<QString>(evaluatorSrcDstBox_->itemData(evaluatorSrcDstBox_->currentIndex())) == 
       qVariantValue<QString>(evaluatorDstSrcBox_->itemData(evaluatorDstSrcBox_->currentIndex()))) {
        toggleChainButton(true);
        chainButton_->setEnabled(true);
    } else {
        toggleChainButton(false);
        chainButton_->setEnabled(true);
    }
    chainButton_->blockSignals(false);
    evaluatorSrcDstBox_->blockSignals(false);
}

void PropertyLinkDialog::updateDstSrcEvaluatorBox(PropertyGraphicsItem* src,PropertyGraphicsItem* dst) {
    //set <-
    evaluatorDstSrcBox_->blockSignals(true);
    chainButton_->blockSignals(true);
    evaluatorDstSrcBox_->clear();
    evaluatorDstSrcBox_->setEnabled(true);

    //is empty?
    if(!src || !dst){
        evaluatorDstSrcBox_->setEnabled(false);
        toggleChainButton(false);
        chainButton_->setEnabled(false);
        chainButton_->blockSignals(false);
        evaluatorDstSrcBox_->blockSignals(false);
        return;
    }

    //get Evaluators
    std::vector<std::pair<std::string, std::string> > availableEvaluators =
            src->getProperty()->getCompatibleEvaluators(dst->getProperty());

    if(availableEvaluators.empty()){
        evaluatorDstSrcBox_->setEnabled(false);
        toggleChainButton(false);
        chainButton_->setEnabled(false);
        chainButton_->blockSignals(false);
        evaluatorDstSrcBox_->blockSignals(false);
        return;
    }

    //set Box
    for (std::vector<std::pair<std::string, std::string> >::iterator i = availableEvaluators.begin();
            i != availableEvaluators.end(); ++i) {
                evaluatorDstSrcBox_->addItem(QString::fromStdString(i->second),QVariant(QString::fromStdString(i->first)));
    }

    //set Evaluator
    LinkEvaluatorBase* sdEval = 0;
    LinkEvaluatorBase* dsEval = 0;
    if(!(sdEval = getEvaluator(src,dst))){
        bool hasOppEval = false;
        if(dsEval = getEvaluator(dst,src)) {
            for (std::vector<std::pair<std::string, std::string> >::iterator i = availableEvaluators.begin(); i != availableEvaluators.end(); ++i) {
                if(i->first == dsEval->getClassName()) {
                    evaluatorDstSrcBox_->setCurrentIndex(evaluatorDstSrcBox_->findText(QString::fromStdString(i->second)));
                    addEvaluatorModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(i->first)));
                    hasOppEval = true;
                    break;
                }
            }
        }
        if(!hasOppEval) {
            evaluatorDstSrcBox_->setCurrentIndex(evaluatorDstSrcBox_->findText(QString::fromStdString(availableEvaluators[0].second)));
            addEvaluatorModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(availableEvaluators[0].first)));
        }
    } else {
        evaluatorDstSrcBox_->setCurrentIndex(evaluatorDstSrcBox_->findText(QString::fromStdString(
            getEvaluator(src,dst)->getGuiName())));
    }

    //are evaluators chained?
    if(evaluatorSrcDstBox_->currentIndex() == -1) {
        toggleChainButton(true);
        chainButton_->setEnabled(false);
    } else
    if(qVariantValue<QString>(evaluatorSrcDstBox_->itemData(evaluatorSrcDstBox_->currentIndex())) == 
       qVariantValue<QString>(evaluatorDstSrcBox_->itemData(evaluatorDstSrcBox_->currentIndex()))) {
        toggleChainButton(true);
        chainButton_->setEnabled(true);
    } else {
        toggleChainButton(false);
        chainButton_->setEnabled(true);
    }
    chainButton_->blockSignals(false);
    evaluatorDstSrcBox_->blockSignals(false);
}

void PropertyLinkDialog::updateArrowButtonGroup(PropertyLinkArrowGraphicsItem* arrow) {
    tgtAssert(arrow,"PropertyLinkDialog::updateArrowButtonGroup: null pointer passed");
    arrowButtonGroup_->blockSignals(true);
    leftArrowButton_->setEnabled(true);
    rightArrowButton_->setEnabled(true);
    bidirectionalArrowButton_->setEnabled(true);

    if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
        leftArrowButton_->setEnabled(true);
        rightArrowButton_->setEnabled(true);
        bidirectionalArrowButton_->setEnabled(true);
        bidirectionalArrowButton_->setChecked(true);
    } else 
        if(sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getSourceItem())) {
            rightArrowButton_->setEnabled(true);
            rightArrowButton_->setChecked(true);
            if(arrow->getDestinationItem()->getProperty()->isLinkableWith(arrow->getSourceItem()->getProperty())) {
                leftArrowButton_->setEnabled(true);
                bidirectionalArrowButton_->setEnabled(true);
            } else {
                leftArrowButton_->setEnabled(false);
                bidirectionalArrowButton_->setEnabled(false);
            }
        }
        else {
            leftArrowButton_->setEnabled(true);
            leftArrowButton_->setChecked(true);
            if(arrow->getDestinationItem()->getProperty()->isLinkableWith(arrow->getSourceItem()->getProperty())) {
                rightArrowButton_->setEnabled(true);
                bidirectionalArrowButton_->setEnabled(true);
            } else {
                rightArrowButton_->setEnabled(false);
                bidirectionalArrowButton_->setEnabled(false);
            }
       
        }
    lastPressedDirectionButton_ = arrowButtonGroup_->checkedButton();
    arrowButtonGroup_->blockSignals(false);
}

void PropertyLinkDialog::srcDstEvaluatorBoxSelectionChanged(int index) {
    if(index == -1) return;
    PropertyLinkArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    if(!arrow) return;

    PropertyGraphicsItem* src = 0;
    PropertyGraphicsItem* dst = 0;

     if(sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getSourceItem())){
        src = arrow->getSourceItem();
        dst = arrow->getDestinationItem();
    } else {
        dst = arrow->getSourceItem();
        src = arrow->getDestinationItem();
    }

    if (arrow) {
        LinkEvaluatorBase* leb = VoreenApplication::app()->createLinkEvaluator(qVariantValue<QString>(evaluatorSrcDstBox_->itemData(index)).toStdString());
        addEvaluatorModification(new Modification(src,dst,leb));
        
        if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone && areEvaluatorsChained_) {
            int dsIndex = evaluatorDstSrcBox_->findData(evaluatorSrcDstBox_->itemData(index));
            if(dsIndex != -1){
                LinkEvaluatorBase* leb2 = VoreenApplication::app()->createLinkEvaluator(qVariantValue<QString>(evaluatorDstSrcBox_->itemData(dsIndex)).toStdString());
                addEvaluatorModification(new Modification(dst,src,leb2));
                evaluatorDstSrcBox_->blockSignals(true);
                evaluatorDstSrcBox_->setCurrentIndex(dsIndex);
                evaluatorDstSrcBox_->blockSignals(false);
            } else {
                toggleChainButton(false);
                chainButton_->setEnabled(false);
            }
        }  else {
            if(getEvaluator(dst,src) && getEvaluator(src,dst)->getClassName() == getEvaluator(dst,src)->getClassName()) {
                toggleChainButton(true);
                chainButton_->setEnabled(true);
            } else {
                toggleChainButton(false);
                chainButton_->setEnabled(false);
            }
        }

        updateArrowButtonGroup(arrow);
    }
}

void PropertyLinkDialog::dstSrcEvaluatorBoxSelectionChanged(int index) {
    if(index == -1) return;
    PropertyLinkArrowGraphicsItem* arrow = getCurrentlySelectedArrow();
    if(!arrow) return;

    PropertyGraphicsItem* src = 0;
    PropertyGraphicsItem* dst = 0;

     if(sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getSourceItem())){
        src = arrow->getSourceItem();
        dst = arrow->getDestinationItem();
    } else {
        dst = arrow->getSourceItem();
        src = arrow->getDestinationItem();
    }

    if (arrow) {
        LinkEvaluatorBase* leb = VoreenApplication::app()->createLinkEvaluator(qVariantValue<QString>(evaluatorDstSrcBox_->itemData(index)).toStdString());
        addEvaluatorModification(new Modification(dst,src,leb));
        
        if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone && areEvaluatorsChained_) {
            int sdIndex = evaluatorSrcDstBox_->findData(evaluatorDstSrcBox_->itemData(index));
            if(sdIndex != -1){
                LinkEvaluatorBase* leb2 = VoreenApplication::app()->createLinkEvaluator(qVariantValue<QString>(evaluatorSrcDstBox_->itemData(sdIndex)).toStdString());
                addEvaluatorModification(new Modification(src,dst,leb2));
                evaluatorSrcDstBox_->blockSignals(true);
                evaluatorSrcDstBox_->setCurrentIndex(sdIndex);
                evaluatorSrcDstBox_->blockSignals(false);
            } else {
                toggleChainButton(false);
                chainButton_->setEnabled(false);
            }
        } else {
            if(getEvaluator(src,dst) && getEvaluator(src,dst)->getClassName() == getEvaluator(dst,src)->getClassName()) {
                toggleChainButton(true);
                chainButton_->setEnabled(true);
            } else {
                toggleChainButton(false);
                chainButton_->setEnabled(false);
            }
        }
        
        updateArrowButtonGroup(arrow);
    }
}

void PropertyLinkDialog::deleteArrowAction() {
    PropertyLinkArrowGraphicsItem* arrow = getCurrentlySelectedArrow();

    if (arrow) {
        if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone){
            arrow->getDestinationItem()->removeGraphicalLink(arrow->getSourceItem());
            addRemoveModification(new Modification(arrow->getDestinationItem(),arrow->getSourceItem(),0));
        }
        addRemoveModification(new Modification(arrow->getSourceItem(),arrow->getDestinationItem(),0));
        arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
        sourceView_->viewport()->update();
        if(destinationView_) destinationView_->viewport()->update();
    }
}

void PropertyLinkDialog::linkProperties(PropertyGraphicsItem* src, PropertyGraphicsItem* dst) {
    tgtAssert(src != dst, "Link between the same Property is not allowed");
    //switch source left
    if(!sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(src)){
        PropertyGraphicsItem* help = src;
        src = dst; dst = help;
    }

    //block render size links
    if(dynamic_cast<RenderSizeOriginProperty*>(const_cast<Property*>(src->getProperty())) ||
       dynamic_cast<RenderSizeOriginProperty*>(const_cast<Property*>(dst->getProperty())) ||
       dynamic_cast<RenderSizeReceiveProperty*>(const_cast<Property*>(src->getProperty())) ||
       dynamic_cast<RenderSizeReceiveProperty*>(const_cast<Property*>(dst->getProperty()))) {
           if (QMessageBox::warning(this, tr("Warning"), QString::fromStdString("<b>You are trying to link RenderSize-Properties. </b><br> <br>"\
               "Please use the RenderSize-Layer in the NetworkEditor to link such properties. Linking RenderSize-Properties "\
               "outside this layer can cause nondeterministic behavior. <br><br> <b>Are you sure you still want to link these properties?</b>"), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
           }
    }

    if(bidirectionalArrowButton_->isChecked()) {
        //check, if one direction exists already
        foreach(PropertyLinkArrowGraphicsItem* arrow, dst->getSourceLinkList()) {
            if(arrow->getDestinationItem() == src)
                return;
        }
        foreach(PropertyLinkArrowGraphicsItem* arrow, src->getSourceLinkList()) {
            if(arrow->getDestinationItem() == dst)
                return;
        }

        //first direction, if possible
        if(src->getProperty()->getCompatibleEvaluators(dst->getProperty()).size()) {
            addAddModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(
                        src->getProperty()->getCompatibleEvaluators(dst->getProperty())[0].first)));
            src->addGraphicalLink(dst);
        }
        //second direction
        if(dst->getProperty()->getCompatibleEvaluators(src->getProperty()).size()) {
            addAddModification(new Modification(dst,src,VoreenApplication::app()->createLinkEvaluator(
                        dst->getProperty()->getCompatibleEvaluators(src->getProperty())[0].first)));
            dst->addGraphicalLink(src);
        }
    } else if (leftArrowButton_->isChecked()) {
        //check if link exists already
        foreach(PropertyLinkArrowGraphicsItem* arrow, dst->getSourceLinkList()) {
            if(arrow->getDestinationItem() == src)
                return;
        }

        if(dst->getProperty()->getCompatibleEvaluators(src->getProperty()).size()) {
            addAddModification(new Modification(dst,src,VoreenApplication::app()->createLinkEvaluator(
                                dst->getProperty()->getCompatibleEvaluators(src->getProperty())[0].first)));
            dst->addGraphicalLink(src);
        }

    } else if (rightArrowButton_->isChecked()) {
        //check, if link exists already
        foreach(PropertyLinkArrowGraphicsItem* arrow, src->getSourceLinkList()) {
            if(arrow->getDestinationItem() == dst)
                return;
        }
        
        if(src->getProperty()->getCompatibleEvaluators(dst->getProperty()).size()) {
            addAddModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator(
                            src->getProperty()->getCompatibleEvaluators(dst->getProperty())[0].first)));
            src->addGraphicalLink(dst);
        }
    }
    src->setVisible(true);
    dst->setVisible(true);
    setCurrentlySelectedArrow(src,dst);
}

void PropertyLinkDialog::directionButtonClicked(QAbstractButton* button) {
    if(lastPressedDirectionButton_ == button) return;
    lastPressedDirectionButton_ = button;
    PropertyLinkArrowGraphicsItem* arrow = getCurrentlySelectedArrow();

    PropertyGraphicsItem* src = 0;
    PropertyGraphicsItem* dst = 0;

    if (arrow) {
        bool isLeftArrow = false;
        arrow->setZValue(ZValuesPropertyLinkArrowGraphicsItem);
        if(sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getSourceItem())) {
            src = arrow->getSourceItem(); //property on sourceView side
            dst = arrow->getDestinationItem();
        } else {
            src = arrow->getDestinationItem(); //property on sourceView side
            dst = arrow->getSourceItem();
            isLeftArrow = true;
        }

        if (button == leftArrowButton_){
            if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
                //from bi to left
                addRemoveModification(new Modification(src,dst,0));
                src->removeGraphicalLink(dst);
            } else {
                //from right to left
                updateDstSrcEvaluatorBox(dst,src);
                addAddModification(new Modification(dst,src, VoreenApplication::app()->createLinkEvaluator
                                  (getEvaluator(dst,src)->getClassName())));
                dst->addGraphicalLink(src);
                addRemoveModification(new Modification(src,dst,0));
                src->removeGraphicalLink(dst);
                src->setVisible(true); //to make arrow visible
            }
        } else if(button == rightArrowButton_) {
            if(arrow->getSourceArrowHeadDirection() != ConnectionBaseGraphicsItem::ArrowHeadDirectionNone) {
                //from bi to right
                addRemoveModification(new Modification(dst,src,0));
                dst->removeGraphicalLink(src);
            } else {
                //from left to right
                updateSrcDstEvaluatorBox(src,dst);
                addAddModification(new Modification(src,dst, VoreenApplication::app()->createLinkEvaluator
                                    (getEvaluator(src,dst)->getClassName())));
                src->addGraphicalLink(dst);
                addRemoveModification(new Modification(dst,src,0));
                dst->removeGraphicalLink(src);
                dst->setVisible(true); //to make arrow visible
            }
        } else if(button == bidirectionalArrowButton_) {
                if(isLeftArrow) { //left to bi
                    updateSrcDstEvaluatorBox(src,dst);
                    addAddModification(new Modification(src,dst,VoreenApplication::app()->createLinkEvaluator
                                        (getEvaluator(src,dst)->getClassName())));
                    src->addGraphicalLink(dst);
                    dst->setVisible(true);
                    src->setVisible(true);
                } else { //right to bi
                    updateDstSrcEvaluatorBox(dst,src);
                    addAddModification(new Modification(dst,src,VoreenApplication::app()->createLinkEvaluator
                                        (getEvaluator(dst,src)->getClassName())));
                    dst->addGraphicalLink(src);
                    dst->setVisible(true);
                    src->setVisible(true);
                }
        } else {
                tgtAssert(false,"unknown direction");
        }
        setCurrentlySelectedArrow(src,dst);
        updateEvaluatorBoxes(getCurrentlySelectedArrow());
    }
    sourceView_->updateViewport();
}

LinkEvaluatorBase* PropertyLinkDialog::getEvaluator(PropertyGraphicsItem* src, PropertyGraphicsItem* dst) {
    foreach(Modification* m, addModificationList_) {
        if(m->src_ == src && m->dst_ == dst)
            return m->eval_;
    }
    foreach(Modification* m, evaluatorModificationList_) {
        if(m->src_ == src && m->dst_ == dst)
            return m->eval_;
    }
    PropertyLink* link = src->getProperty()->getLink(dst->getProperty());
    if(link)
        return link->getLinkEvaluator();

    return 0;
}

void PropertyLinkDialog::toggleChainButton() {
    toggleChainButton(!areEvaluatorsChained_);
}

void PropertyLinkDialog::toggleChainButton(bool b) {
    if(areEvaluatorsChained_ == b) return;
    areEvaluatorsChained_ = b;
    if(areEvaluatorsChained_) {
        chainButton_->setIcon(QIcon(":/voreenve/icons/chain.png"));
        chainButton_->setToolTip(tr("Evaluators are Connected"));
    } else {
        chainButton_->setIcon(QIcon(":/voreenve/icons/unchain.png"));
        chainButton_->setToolTip(tr("Evaluators are not Connected"));
    }
}

void PropertyLinkDialog::deleteAllProcessorLinks() {
    foreach (PropertyGraphicsItem* sProp, sourceItem_->getPropertyList()->getPropertyGraphicsItems()){
        if(sProp->isVisible()){
            foreach (PropertyLinkArrowGraphicsItem* arrow, sProp->getSourceLinkList()){
                if(arrow->getDestinationItem()->isVisible() && arrow->getDestinationItem()->getPropertyOwnerGraphicsItem() != sourceItem_){
                    addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                    arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                }
            }
            foreach (PropertyLinkArrowGraphicsItem* arrow, sProp->getDestinationLinkList()){
                if(arrow->getDestinationItem()->isVisible() && arrow->getDestinationItem()->getPropertyOwnerGraphicsItem() != sourceItem_){
                    addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                    arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                }
            }
        }
    }
    foreach (PropertyGraphicsItem* dProp, destinationItem_->getPropertyList()->getPropertyGraphicsItems()){
        if(dProp->isVisible()){
            foreach (PropertyLinkArrowGraphicsItem* arrow, dProp->getSourceLinkList()){
                if(arrow->getDestinationItem()->isVisible() && arrow->getDestinationItem()->getPropertyOwnerGraphicsItem() != destinationItem_){
                    addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                    arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                }
            }
            foreach (PropertyLinkArrowGraphicsItem* arrow, dProp->getDestinationLinkList()){
                if(arrow->getDestinationItem()->isVisible() && arrow->getDestinationItem()->getPropertyOwnerGraphicsItem() != destinationItem_){
                    addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                    arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                }
            }
        }
    }
    sourceView_->updateViewport();
}

void PropertyLinkDialog::deleteAllPortLinks() {
    foreach(PropertyListGraphicsItem* list, sourceItem_->getPropertyList()->getSubPropertyLists()){
        if(dynamic_cast<PortGraphicsItem*>(list->getParent())) {
            foreach (PropertyGraphicsItem* sProp, list->getPropertyGraphicsItems()){
                if(sProp->isVisible()){
                    foreach (PropertyLinkArrowGraphicsItem* arrow, sProp->getSourceLinkList()){
                        if(arrow->getDestinationItem()->isVisible() 
                            && !sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getDestinationItem())
                            && getEvaluator(arrow->getSourceItem(),arrow->getDestinationItem())->getClassName() != "LinkEvaluatorRenderSize"){
                            addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                            arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                        }
                    }
                    foreach (PropertyLinkArrowGraphicsItem* arrow, sProp->getDestinationLinkList()){
                        if(arrow->getDestinationItem()->isVisible() 
                            && !sourceItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getDestinationItem())
                            && getEvaluator(arrow->getSourceItem(),arrow->getDestinationItem())->getClassName() != "LinkEvaluatorRenderSize"){
                            addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                            arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                        }
                    }
                }
            }
        }
    }
    foreach(PropertyListGraphicsItem* list, destinationItem_->getPropertyList()->getSubPropertyLists()){
        if(dynamic_cast<PortGraphicsItem*>(list->getParent())) {
            foreach (PropertyGraphicsItem* dProp, list->getPropertyGraphicsItems()){
                if(dProp->isVisible()){
                    foreach (PropertyLinkArrowGraphicsItem* arrow, dProp->getSourceLinkList()){
                        if(arrow->getDestinationItem()->isVisible() 
                            && !destinationItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getDestinationItem())
                            && getEvaluator(arrow->getSourceItem(),arrow->getDestinationItem())->getClassName() != "LinkEvaluatorRenderSize"){
                            addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                            arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                        }
                    }
                    foreach (PropertyLinkArrowGraphicsItem* arrow, dProp->getDestinationLinkList()){
                        if(arrow->getDestinationItem()->isVisible()
                            && !destinationItem_->getPropertyList()->getAllPropertyGraphicsItems().contains(arrow->getDestinationItem())
                            && getEvaluator(arrow->getSourceItem(),arrow->getDestinationItem())->getClassName() != "LinkEvaluatorRenderSize"){
                            addRemoveModification(new Modification(arrow->getSourceItem(), arrow->getDestinationItem(), 0));
                            arrow->getSourceItem()->removeGraphicalLink(arrow->getDestinationItem());
                        }
                    }
                }
            }
        }
    }
    sourceView_->updateViewport();
}

void PropertyLinkDialog::showProcessorAutoLinks() {
    autoArrows_.clear();
    foreach (PropertyGraphicsItem* leftProp, sourceItem_->getPropertyList()->getPropertyGraphicsItems()) {
        foreach (PropertyGraphicsItem* rightProp, destinationItem_->getPropertyList()->getPropertyGraphicsItems()) {
            if (leftProp->getProperty()->getGuiName() == rightProp->getProperty()->getGuiName() 
                    && (typeid(*leftProp->getProperty()) == typeid(*rightProp->getProperty())) 
                    && (leftProp != rightProp)
                    && !(leftProp->getProperty()->isLinkedWith(rightProp->getProperty(),true))) {

                if(leftProp->getProperty()->isLinkableWith(rightProp->getProperty()) && 
                   rightProp->getProperty()->isLinkableWith(leftProp->getProperty())) {
                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(leftProp,rightProp);
                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                    arrow->setVisible(true); autoArrows_.append(arrow); 
                    arrow = new PropertyLinkArrowGraphicsItem(rightProp,leftProp);
                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                    arrow->setVisible(true); autoArrows_.append(arrow); 
                }
                else if(leftProp->getProperty()->isLinkableWith(rightProp->getProperty())) {
                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(leftProp,rightProp);
                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                    arrow->setVisible(true); autoArrows_.append(arrow); 
                }
                else if(rightProp->getProperty()->isLinkableWith(leftProp->getProperty())) {
                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(rightProp,leftProp);
                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                    arrow->setVisible(true); autoArrows_.append(arrow); 
                }
            }
        }
    }
    sourceView_->updateViewport();
}

void PropertyLinkDialog::showPortAutoLinks() {
    autoArrows_.clear();
    foreach (PropertyListGraphicsItem* leftList, sourceItem_->getPropertyList()->getSubPropertyLists()) {
        if(dynamic_cast<PortGraphicsItem*>(leftList->getParent())) {
            foreach (PropertyListGraphicsItem* rightList, destinationItem_->getPropertyList()->getSubPropertyLists()) {
                if(dynamic_cast<PortGraphicsItem*>(leftList->getParent())) {
                    foreach (PropertyGraphicsItem* leftProp, leftList->getPropertyGraphicsItems()) {
                        foreach (PropertyGraphicsItem* rightProp, rightList->getPropertyGraphicsItems()) {

                            if (leftProp->getProperty()->getGuiName() == rightProp->getProperty()->getGuiName() 
                                    && (typeid(*leftProp->getProperty()) == typeid(*rightProp->getProperty())) 
                                    && (leftProp != rightProp)
                                    && !(leftProp->getProperty()->isLinkedWith(rightProp->getProperty(),true))
                                    && !(dynamic_cast<RenderSizeOriginProperty*>(const_cast<Property*>(leftProp->getProperty())))
                                    && !(dynamic_cast<RenderSizeReceiveProperty*>(const_cast<Property*>(leftProp->getProperty())))
                                    && !(dynamic_cast<RenderSizeOriginProperty*>(const_cast<Property*>(rightProp->getProperty())))
                                    && !(dynamic_cast<RenderSizeReceiveProperty*>(const_cast<Property*>(rightProp->getProperty())))) {

                                if(leftProp->getProperty()->isLinkableWith(rightProp->getProperty()) && 
                                   rightProp->getProperty()->isLinkableWith(leftProp->getProperty())) {
                                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(leftProp,rightProp);
                                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                                    arrow->setVisible(true); autoArrows_.append(arrow); 
                                    arrow = new PropertyLinkArrowGraphicsItem(rightProp,leftProp);
                                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                                    arrow->setVisible(true); autoArrows_.append(arrow); 
                                }
                                else if(leftProp->getProperty()->isLinkableWith(rightProp->getProperty())) {
                                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(leftProp,rightProp);
                                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                                    arrow->setVisible(true); autoArrows_.append(arrow); 
                                }
                                else if(rightProp->getProperty()->isLinkableWith(leftProp->getProperty())) {
                                    PropertyLinkArrowGraphicsItem* arrow = new PropertyLinkArrowGraphicsItem(rightProp,leftProp);
                                    arrow->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
                                    arrow->setVisible(true); autoArrows_.append(arrow); 
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    sourceView_->updateViewport();
}

void PropertyLinkDialog::hideAutoLinks() {
    foreach (PropertyLinkArrowGraphicsItem* arrow, autoArrows_) {
        delete arrow;
    }
    autoArrows_.clear();
    sourceView_->updateViewport();
}

void PropertyLinkDialog::confirmAutoLinks() {
    foreach (PropertyLinkArrowGraphicsItem* arrow, autoArrows_) {
        if(sourceItem_->getPropertyList()->getPropertyGraphicsItems().contains(arrow->getSourceItem()))
            linkProperties(arrow->getSourceItem(),arrow->getDestinationItem());
        delete arrow;
    }
    autoArrows_.clear();
    sourceView_->updateViewport();
}

//---------------------------------------------------------------------------------------------------------------
//                  modification functions                                                                          
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkDialog::addAddModification(Modification* mod){
    foreach(Modification* m, removeModificationList_) {
        if(*m == *mod) {
            removeModificationList_.removeOne(m);
            if(m->eval_ != mod->eval_)
                evaluatorModificationList_.append(mod);
            delete m;
            return;
        }
    }
    foreach(Modification* m, evaluatorModificationList_) {
        if(*m == *mod) {
            evaluatorModificationList_.removeOne(m);
            delete m;
            break;
        }
    }
    addModificationList_.append(mod);
}

void PropertyLinkDialog::addRemoveModification(Modification* mod) {
    foreach(Modification* m, addModificationList_) {
        if(*m == *mod) {
            addModificationList_.removeOne(m);
            delete m->eval_;
            delete m;
            return;
        }
    }
    removeModificationList_.append(mod);
    foreach(Modification* m, evaluatorModificationList_) {
        if(*m == *mod) {
            evaluatorModificationList_.removeOne(m);
            delete m->eval_;
            delete m;
            return;
        }
    }
}

void PropertyLinkDialog::addEvaluatorModification(Modification* mod) {
    foreach(Modification* m, addModificationList_) {
        if(*m == *mod) {
            delete m->eval_;
            m->eval_ = mod->eval_;
            return;
        }
    }
    foreach(Modification* m, evaluatorModificationList_) {
        if(*m == *mod) {
            delete m->eval_;
            m->eval_ = mod->eval_;
            return;
        }
    }
    evaluatorModificationList_.append(mod);
}

void PropertyLinkDialog::applyAllModifications() {
    foreach(Modification* m, evaluatorModificationList_) {
        PropertyLink* link = m->src_->getProperty()->getLink(m->dst_->getProperty());
        link->setLinkEvaluator(m->eval_);
        evaluatorModificationList_.removeOne(m);
        delete m;
    }

    foreach(Modification* m, addModificationList_) {
        dynamic_cast<NetworkEditor*>(parent())->getProcessorNetwork()->createPropertyLink(
                    const_cast<Property*>(m->src_->getProperty()),const_cast<Property*>(m->dst_->getProperty()),m->eval_);
        addModificationList_.removeOne(m);
        delete m;
    }

    foreach(Modification* m, removeModificationList_) {
        dynamic_cast<NetworkEditor*>(parent())->getProcessorNetwork()->removePropertyLink(
                    const_cast<Property*>(m->src_->getProperty()),const_cast<Property*>(m->dst_->getProperty()));
        removeModificationList_.removeOne(m);
        delete m;
    }
    emit accept();
}

void PropertyLinkDialog::undoAllModifications() {
    foreach(Modification* m, evaluatorModificationList_) {
        evaluatorModificationList_.removeOne(m);
        delete m->eval_;
        delete m;
    }

    foreach(Modification* m, addModificationList_) {
        m->src_->removeGraphicalLink(m->dst_);
        addModificationList_.removeOne(m);
        delete m->eval_;
        delete m;
    }

    foreach(Modification* m, removeModificationList_) {
        m->src_->addGraphicalLink(m->dst_);
        removeModificationList_.removeOne(m);
        delete m;
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  events                                                                                       
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkDialog::keyPressEvent(QKeyEvent* event) {
    QDialog::keyPressEvent(event);

    if (!event->isAccepted() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
        deleteArrowAction();
}




/*




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
/*            info.evaluator = new DependencyLinkEvaluator;
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
                if (info.evaluator->arePropertiesLinkable(info.source->getProperty(), info.destination->getProperty()))
                    emit createLink(info.source->getProperty(), info.destination->getProperty(), info.evaluator);

                if (info.bidirectional && info.evaluator->arePropertiesLinkable(info.destination->getProperty(), info.source->getProperty()))
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


*/
} //voreen namespace
