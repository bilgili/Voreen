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

#ifndef VRN_PROCESSORLISTWIDGET_H
#define VRN_PROCESSORLISTWIDGET_H

#include <QContextMenuEvent>
#include <QTreeWidget>
#include <QWidget>

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/qt/widgets/lineeditresetwidget.h"

class QMouseEvent;
class QTreeWidgetItem;
class QTextBrowser;
class QSplitter;
class QContextMenuEvent;

namespace voreen {

class ProcessorListWidget;

class ProcessorListTreeWidget : public QTreeWidget {
Q_OBJECT
public:
    ProcessorListTreeWidget(ProcessorListWidget* processorListWidget, QWidget* parent = 0);

    enum SortType {
        SORT_BY_CATEGORY,
        SORT_BY_MODULENAME,
        SORT_BY_MODULE_CATEGORY,
        SORT_BY_CATEGORY_MODULE
    };

public slots:
    /**
     * Filters the processor list based in the given text. Every processor will be listed
     * if its name includes the text or, if checkDescrition is true, its descrition contains the text
     */
    void filter(const QString& text);
    void sort(int);
    void hideStatus(bool);
    void searchDescription(bool);

protected:
    void mousePressEvent(QMouseEvent*);

private:
    //void buildItems();

    ProcessorFactory::KnownClassesVector processorVector_;
    QList<QTreeWidgetItem*> items_;

    // pointer to the ProcessorListWidget this widget is part of
    ProcessorListWidget* processorListWidget_;

    void sortByCategory();
    void sortByModuleName();
    void sortByCategoryThenModule();
    void sortByModuleThenCategory();

    SortType sortType_;
    std::string filterText_;
    bool showCodeState_;
    bool searchDescription_;

};

//---------------------------------------------------------------------------

class ProcessorListItem : public QTreeWidgetItem {
public:
    ProcessorListItem(const std::string& id);
    const std::string& getId() { return id_; }
    const std::string& getInfo() { return info_; }
    void setInfo(std::string info) { info_ = info; }

private:
    const std::string id_;
    std::string info_;
};

//---------------------------------------------------------------------------

class ProcessorListWidget : public QWidget {
Q_OBJECT
public:
    ProcessorListWidget(QWidget* parent = 0);
public slots:
    void processorsSelected(const std::vector<Processor*>& processors);
    void setInfo(Processor* processor);

protected:
    ProcessorListTreeWidget* tree_;
    QIcon resetIcon_;
    QTextBrowser* info_;
    QSplitter* splitter_;
    QAction* hideAction_;
    QAction* searchDescription_;

protected slots:
    void setInfo();
    void sortMenu();

signals:
    void sort(int);
    void hideStatus(bool);
    void searchDescription(bool);
};


} //namespace voreen

#endif // VRN_PROCESSORLISTWIDGET_H
