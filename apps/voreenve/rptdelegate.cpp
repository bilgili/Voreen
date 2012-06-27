/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

 #include <QtGui>

 #include "voreen/rptgui/rptdelegate.h"

 SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
     : QItemDelegate(parent)
 {
 }

 QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
 {
     QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(0);
     editor->setMaximum(100);

     /* QComboBox *editor = new QComboBox(parent);
      editor->insertItem(0,"zeile 0");
      editor->insertItem(1,"zeile 1");*/
     // QCheckBox* editor = new QCheckBox("holla",parent);  

     return editor;
 }

 void SpinBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
 {

     int value = index.model()->data(index, Qt::DisplayRole).toInt();
     //QComboBox *comboBox = static_cast<QComboBox*>(editor);
     //comboBox->setCurrentIndex(value); //   -setValue(value);
    

     // int value = index.model()->data(index, Qt::DisplayRole).toInt();

     QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->setValue(value);
    
    // bool value = index.model()->data(index, Qt::DisplayRole).toBool();
    // cBox->setChecked(value);     
    // QCheckBox *cBox = static_cast<QCheckBox*>(editor); 
    

 }

 void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
     QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->interpretText();
     int value = spinBox->value();
     
     model->setData(index, 0, Qt::EditRole);
     
   /*  QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
     bool value = checkBox->isChecked(); 
     
     model->setData(index, value, Qt::EditRole);*/

   /*  QComboBox *comboBox = static_cast<QComboBox*>(editor);
     int value = comboBox->currentIndex(); 
     
     model->setData(index, comboBox->itemText(value), Qt::EditRole);*/


 }

 void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }


 ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
     : QItemDelegate(parent)
 {
 }

 QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
 {
    /* QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(0);
     editor->setMaximum(100);*/
      QComboBox *editor = new QComboBox(parent);
      editor->insertItem(0,"low");
      editor->insertItem(1,"medium");
      editor->insertItem(2,"high");

     // QCheckBox* editor = new QCheckBox("holla",parent);  

     return editor;
 }

 void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
 {

     int value = index.model()->data(index, Qt::DisplayRole).toInt();
     QComboBox *comboBox = static_cast<QComboBox*>(editor);
     comboBox->setCurrentIndex(value); //   -setValue(value);
    

     // int value = index.model()->data(index, Qt::DisplayRole).toInt();

     //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     /*spinBox->setValue(value);*/
    
    // bool value = index.model()->data(index, Qt::DisplayRole).toBool();
    // cBox->setChecked(value);     
    // QCheckBox *cBox = static_cast<QCheckBox*>(editor); 
    

 }

 void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
     /*QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->interpretText();
     int value = spinBox->value();
     
     model->setData(index, 0, Qt::EditRole);*/
     
   /*  QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
     bool value = checkBox->isChecked(); 
     
     model->setData(index, value, Qt::EditRole);*/

     QComboBox *comboBox = static_cast<QComboBox*>(editor);
     int value = comboBox->currentIndex(); 
     
     model->setData(index, comboBox->itemText(value), Qt::EditRole);


 }

 void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }

//----------------

 CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
     : QItemDelegate(parent)
 {
 }

 QWidget *CheckBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
 {
    /* QSpinBox *editor = new QSpinBox(parent);
     editor->setMinimum(0);
     editor->setMaximum(100);*/
      /*QComboBox *editor = new QComboBox(parent);
      editor->insertItem(0,"zeile 0");
      editor->insertItem(1,"zeile 1");*/
     QCheckBox* editor = new QCheckBox("change",parent);  

     return editor;
 }

 void CheckBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
 {

     //int value = index.model()->data(index, Qt::DisplayRole).toInt();
     //QComboBox *comboBox = static_cast<QComboBox*>(editor);
     //comboBox->setCurrentIndex(value); //   -setValue(value);
    

     // int value = index.model()->data(index, Qt::DisplayRole).toInt();

     //QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     /*spinBox->setValue(value);*/
     
     QCheckBox* cBox = static_cast<QCheckBox*>(editor);
     bool value = index.model()->data(index, Qt::DisplayRole).toBool();
     cBox->setChecked(value);     
    

 }

 void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
 {
     /*QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
     spinBox->interpretText();
     int value = spinBox->value();
     
     model->setData(index, 0, Qt::EditRole);*/
     
     QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
     bool value = checkBox->isChecked(); 
     
     model->setData(index, value, Qt::EditRole);

    /* QComboBox *comboBox = static_cast<QComboBox*>(editor);
     int value = comboBox->currentIndex(); 
     
     model->setData(index, comboBox->itemText(value), Qt::EditRole);*/


 }

 void CheckBoxDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }
