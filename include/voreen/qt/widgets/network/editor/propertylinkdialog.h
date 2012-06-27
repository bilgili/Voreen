/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

class QPushButton;
class QComboBox;

namespace voreen {

class PropertyGraphicsItem;
class LinkEvaluatorBase;

class PropertyLinkDialog : public QDialog {
    Q_OBJECT
public:
    enum PropertyLinkSelectedButton {
        BUTTON_TO_LEFT,
        BUTTON_BIDIRECTIONAL,
        BUTTON_TO_RIGHT
    };

    PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
                       PropertyLinkSelectedButton selectedButton = BUTTON_BIDIRECTIONAL);

signals:
     void closeDialog();
     void create(PropertyGraphicsItem* sourceGraphicsItem, PropertyGraphicsItem* destGraphicsItem,
                 LinkEvaluatorBase* eval);

private slots:
     void createPropertyLink();

     void functionComboBoxChange();
#ifdef VRN_WITH_PYTHON
     void addFunction(const QString& functionName, const QString& functionBody);
#endif
     void setLeftArrowButtonChecked();
     void setRightArrowButtonChecked();
     void setBidirectionalButtonChecked();

private:
    void createWidgets();
    void setupCombobox();

    PropertyGraphicsItem* sourceGraphicsItem_;
    PropertyGraphicsItem* destinationGraphicsItem_;

    QPushButton* bidirectionalButton_;
    QPushButton* leftArrowButton_;
    QPushButton* rightArrowButton_;
    QComboBox* functionCB_;
 };

} //namespace voreen

#endif // VRN_GUIITEM_H

