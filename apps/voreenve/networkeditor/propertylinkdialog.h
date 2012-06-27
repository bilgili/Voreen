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

#ifndef VRN_PROPERTYLINKDIALOG_H
#define VRN_PROPERTYLINKDIALOG_H

#include <QDialog>
#include <QMap>
#include <QPushButton>

#include "linkdialogpropertygraphicsitem.h"

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QGraphicsItem;
class QGraphicsView;

namespace voreen {

class LinkDialogArrowGraphicsItem;
class LinkDialogGraphicsView;
class LinkDialogPropertyGraphicsItem;
class LinkEvaluatorBase;
class ProcessorGraphicsItem;
class Property;
class PropertyGraphicsItem;
class PropertyLink;
class RootGraphicsItem;

class EnterExitPushButton : public QPushButton {
    Q_OBJECT
public:
    EnterExitPushButton(QWidget* parent = 0);
    EnterExitPushButton(const QString& text, QWidget* parent = 0);
    EnterExitPushButton(const QIcon& icon, const QString& text, QWidget* parent = 0);

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);

signals:
    void enterEventSignal();
    void leaveEventSignal();
};

class PropertyLinkDialog : public QDialog {
Q_OBJECT
public:
    enum PropertyLinkDirection {
        PropertyLinkDirectionToLeft,
        PropertyLinkDirectionBidirectional,
        PropertyLinkDirectionToRight
    };

    PropertyLinkDialog(QWidget* parent, RootGraphicsItem* sourceGraphicsItem, RootGraphicsItem* destGraphicsItem,
                       const QList<PropertyLink*>& existingLinks = QList<PropertyLink*>());
    PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
                       const PropertyLink* link, PropertyLinkDirection selectedButton = PropertyLinkDirectionBidirectional);

signals:
    void createLink(const Property* sourceProp, const Property* destProp, LinkEvaluatorBase* eval);
    void removeLink(PropertyLink* link);

public slots:
    LinkDialogArrowGraphicsItem* createdArrow(LinkDialogArrowGraphicsItem* arrow, bool bidirectional = true);

private slots:
     void createPropertyLink();
     void sceneSelectionChanged();
     void comboBoxSelectionChanged(const QString& text);
     void deleteArrow(QGraphicsItem* arrow);
     void deleteSelectedArrow();
     void controlButtonClicked(QAbstractButton* button);
     void showAutoLinksByName();
     void hideAutoLinks();
     void confirmAutoLinks();
     void deleteAllLinks();

private:
    struct ConnectionInfo {
        LinkDialogPropertyGraphicsItem* source;
        LinkDialogPropertyGraphicsItem* destination;
        LinkEvaluatorBase* evaluator;
        bool bidirectional;
    };

    void init();
    void initCombobox();
    void initGraphicsItem(RootGraphicsItem* item, ColumnPosition position);
    void initPropertyItems(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem);

    PropertyLinkDirection getDirection(LinkDialogArrowGraphicsItem* arrow) const;
    LinkDialogArrowGraphicsItem* getCurrentlySelectedArrow() const;

    void addProbationalConnection(ConnectionInfo info);

    void createArrowFromPropertyLink(PropertyLink* link);

    RootGraphicsItem* sourceGraphicsItem_;
    RootGraphicsItem* destinationGraphicsItem_;

    bool isEditing_;
    LinkDialogPropertyGraphicsItem* sourcePropertyItem_;
    LinkDialogPropertyGraphicsItem* destinationPropertyItem_;

    LinkDialogGraphicsView* view_;
    QComboBox* functionCB_;
    QButtonGroup* arrowButtonGroup_;
    QPushButton* deleteArrowButton_;
    QPushButton* leftArrowButton_;
    QPushButton* bidirectionalArrowButton_;
    QPushButton* rightArrowButton_;

    QMap<LinkDialogArrowGraphicsItem*, ConnectionInfo> connectionMap_;
    QMap<LinkDialogArrowGraphicsItem*, ConnectionInfo> probationalConnectionMap_;
    QList<PropertyLink*> previouslyExistingLinks_;
    QMultiMap<LinkDialogArrowGraphicsItem*, PropertyLink*> existingLinksMap_;
};

} //namespace voreen

#endif // VRN_PROPERTYLINKDIALOG_H

