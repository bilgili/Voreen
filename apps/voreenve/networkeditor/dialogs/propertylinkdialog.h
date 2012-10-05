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

#ifndef VRN_PROPERTYLINKDIALOG_H
#define VRN_PROPERTYLINKDIALOG_H

#include <QDialog>

//class QGraphicsView;
class QPushButton;
class QComboBox;
class QButtonGroup;
class QAbstractButton;

namespace voreen {

    class SplitGraphicsView;
    class PortOwnerGraphicsItem;
    class PropertyGraphicsItem;
    class PropertyOwnerGraphicsItem;
    class PropertyLinkArrowGraphicsItem;
    class PropertyListGraphicsItem;
    class LinkEvaluatorBase;

class PropertyLinkDialog : public QDialog {
Q_OBJECT
public:

    struct Modification {
        PropertyGraphicsItem* src_;
        PropertyGraphicsItem* dst_;
        LinkEvaluatorBase* eval_;

        Modification(PropertyGraphicsItem* src, PropertyGraphicsItem* dst, LinkEvaluatorBase* eval) 
            : src_(src), dst_(dst), eval_(eval) {};

        bool operator== (Modification& mod) {
            return (src_ == mod.src_ && dst_ == mod.dst_);
        }
    };

    //constructor and destructor
    PropertyLinkDialog(QWidget* parent, PortOwnerGraphicsItem* source, PortOwnerGraphicsItem* destination);
    ~PropertyLinkDialog();

    //---------------------------------------------------------------------------------
    //      view functions                                                             
    //---------------------------------------------------------------------------------
private:
    PropertyLinkArrowGraphicsItem* getCurrentlySelectedArrow() const;
    void setCurrentlySelectedArrow(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);

public slots:
    void sceneSelectionChanged();
    //---------------------------------------------------------------------------------
    //      layout functions                                                           
    //---------------------------------------------------------------------------------
protected:
    void init(bool oneView);
    void initPropertyList(SplitGraphicsView* view, PropertyListGraphicsItem* list);

private:
    //member
    SplitGraphicsView* sourceView_;
    SplitGraphicsView* destinationView_;
    QComboBox* evaluatorSrcDstBox_;
    QComboBox* evaluatorDstSrcBox_;
    QButtonGroup* arrowButtonGroup_;
        QAbstractButton* lastPressedDirectionButton_;
    QPushButton* leftArrowButton_;
    QPushButton* bidirectionalArrowButton_;
    QPushButton* rightArrowButton_;
    QPushButton* chainButton_;
        bool areEvaluatorsChained_;

    //member
    PortOwnerGraphicsItem* sourceItem_;
    PortOwnerGraphicsItem* destinationItem_;
        //autoLinking
    QList<PropertyLinkArrowGraphicsItem*> autoArrows_;

    //---------------------------------------------------------------------------------
    //      evaluator functions and slots                                              
    //---------------------------------------------------------------------------------
    LinkEvaluatorBase* getEvaluator(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);
    void updateEvaluatorBoxes(PropertyLinkArrowGraphicsItem* arrow);
    void updateArrowButtonGroup(PropertyLinkArrowGraphicsItem* arrow);
protected:
    void updateSrcDstEvaluatorBox(PropertyGraphicsItem* src = 0,PropertyGraphicsItem* dst = 0);
    void updateDstSrcEvaluatorBox(PropertyGraphicsItem* src = 0,PropertyGraphicsItem* dst = 0);
protected slots:
    void toggleChainButton();
    void toggleChainButton(bool b);
    void srcDstEvaluatorBoxSelectionChanged(int index);
    void dstSrcEvaluatorBoxSelectionChanged(int index);
    void linkProperties(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);
    void deleteArrowAction();
    void deleteAllProcessorLinks();
    void deleteAllPortLinks();
    void showProcessorAutoLinks();
    void showPortAutoLinks();
    void hideAutoLinks();
    void confirmAutoLinks();
    void directionButtonClicked(QAbstractButton* button);
    //---------------------------------------------------------------------------------
    //      modification functions                                                     
    //---------------------------------------------------------------------------------
protected:
    void addAddModification(Modification* mod);
    void addRemoveModification(Modification* mod);
    void addEvaluatorModification(Modification* mod);
protected slots:
    void applyAllModifications();
    void undoAllModifications();

private:
    //member
    QList<Modification*> addModificationList_;
    QList<Modification*> removeModificationList_;
    QList<Modification*> evaluatorModificationList_;

    //---------------------------------------------------------------------------------
    //      events                                                                     
    //---------------------------------------------------------------------------------
protected:
    void keyPressEvent(QKeyEvent* event);

};

} //namespace voreen

#endif // VRN_PROPERTYLINKDIALOG_H










    //PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
    //                   const PropertyLink* link, PropertyLinkDirection selectedButton = PropertyLinkDirectionBidirectional);

//    bool getNewArrowIsBirectional() const;
//    bool allowConnectionBetweenProperties(const Property* p1, const Property* p2) const;

//signals:
//    void createLink(const Property* sourceProp, const Property* destProp, LinkEvaluatorBase* eval);
//    void removeLink(PropertyLink* link);

//public slots:
//    LinkDialogArrowGraphicsItem* createdArrow(LinkDialogArrowGraphicsItem* arrow, bool bidirectional = true);

//private slots:
/*     void createPropertyLink();
     void sceneSelectionChanged();
     void comboBoxSelectionChanged(const QString& text);
     void deleteArrow(QGraphicsItem* arrow);
     void deleteSelectedArrow();
     void controlButtonClicked(QAbstractButton* button);
     void modeButtonClicked(QAbstractButton* button);

     
     void setDependencyHistoryLengthLabel(int newValue);
     void scrollUpSlot();
     void scrollDownSlot();

     void movedArrow(LinkDialogArrowGraphicsItem* arrow);
     void endedArrow();

private:
    struct ConnectionInfo {
        LinkDialogPropertyGraphicsItem* source;
        LinkDialogPropertyGraphicsItem* destination;
        LinkEvaluatorBase* evaluator;
        bool bidirectional;
    };

    
    
    void updateCombobox(LinkDialogPropertyGraphicsItem* source, LinkDialogPropertyGraphicsItem* destination);
    
    void initPropertyItems(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem);

    PropertyLinkDirection getDirection(LinkDialogArrowGraphicsItem* arrow) const;
    LinkDialogArrowGraphicsItem* getCurrentlySelectedArrow() const;

    void addProbationalConnection(ConnectionInfo info);

    void createArrowFromPropertyLink(PropertyLink* link);
    */
