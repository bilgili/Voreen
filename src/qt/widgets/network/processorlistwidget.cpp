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

#include "voreen/qt/widgets/network/processorlistwidget.h"
#include "voreen/qt/widgets/lineeditresetwidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace voreen {

ProcessorListWidget::ProcessorListWidget(QWidget* parent)
    : QWidget(parent),
    tree_(new ProcessorListTreeWidget(this)),
    resetIcon_(),
    info_(new QTextBrowser()),
    splitter_(new QSplitter(Qt::Vertical))
{
    QPushButton* sortButton = new QPushButton(this);
    sortButton->setIcon(QIcon(":/voreenve/icons/configure.png"));
    sortButton->setGeometry(0,0,32,32);
    sortButton->setFlat(true);
    sortButton->setToolTip(tr("Sort processor list"));

    LineEditResetWidget* edit = new LineEditResetWidget(this);
    edit->setToolTip(tr("Filter processor list"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* filterSortLayout = new QHBoxLayout();
    info_->resize(120, 100);
    info_->setReadOnly(true);

    connect(edit, SIGNAL(textChanged(const QString&)), tree_, SLOT(filter(const QString&)));
    connect(tree_, SIGNAL(itemSelectionChanged()), this, SLOT(setInfo()));
    connect(sortButton, SIGNAL(clicked()), this, SLOT(sortMenu()));
    connect(this, SIGNAL(sort(int)), tree_, SLOT(sort(int)));
    connect(this, SIGNAL(hideStatus(bool)), tree_, SLOT(hideStatus(bool)));
    connect(this, SIGNAL(searchDescription(bool)), tree_, SLOT(searchDescription(bool)));

    filterSortLayout->addWidget(edit);
    filterSortLayout->addWidget(sortButton);
    splitter_->addWidget(tree_);
    splitter_->addWidget(info_);
    QList<int> widgetSpacing;
    widgetSpacing << 500 << 50;
    splitter_->setSizes(widgetSpacing);
    mainLayout->addLayout(filterSortLayout);
    mainLayout->addWidget(splitter_);

    hideAction_ = new QAction(QString::fromStdString("Show Code State"), this);
    hideAction_->setCheckable(true);
    hideAction_->setChecked(true);
    searchDescription_ = new QAction(QString::fromStdString("Search Processor Descriptions"), this);
    searchDescription_->setCheckable(true);
}

void ProcessorListWidget::setInfo(){
    QTreeWidgetItem * tempItem = tree_->currentItem();
    ProcessorListItem * currentItem = dynamic_cast<ProcessorListItem *>(tempItem);
    if(currentItem != 0)
        info_->setHtml(QString::fromStdString(currentItem->getInfo()));
    else
        info_->setHtml("");
}

void ProcessorListWidget::setInfo(Processor * processor) {
    if(processor != 0){
        info_->setHtml(QString::fromStdString(processor->getProcessorInfo()));
    }
    else{
        info_->setHtml("");
    }
}

void ProcessorListWidget::sortMenu() {
    QMenu* menu = new QMenu();
    menu->addAction("Sort by Category");
    menu->addAction("Sort by Module");
    menu->addAction("Sort by Category, then Module");
    menu->addAction("Sort by Module, then Category");
    menu->addSeparator();
    menu->addAction(hideAction_);
    menu->addAction(searchDescription_);
    QAction* action = menu->exec(QCursor::pos());
    if (action) {
        if(action->text() == "Sort by Category")
            emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY);
        else if (action->text() == "Sort by Module")
            emit sort(ProcessorListTreeWidget::SORT_BY_MODULENAME);
        else if (action->text() == "Sort by Category, then Module")
            emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE);
        else if (action->text() == "Sort by Module, then Category")
            emit sort(ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY);
        else if (action->text() == "Show Code State") {
            emit hideStatus(hideAction_->isChecked());
        }
        else if (action->text() == "Search Processor Descriptions") {
            emit searchDescription(searchDescription_->isChecked());
        }
    }
}

void ProcessorListWidget::processorsSelected(const std::vector<Processor*>& processors) {
    if (processors.size() == 1)
        setInfo(processors.front());
    else
        setInfo(0);
}

// ----------------------------------------------------------------------------

ProcessorListTreeWidget::ProcessorListTreeWidget(ProcessorListWidget* processorListWidget, QWidget* parent)
    : QTreeWidget(parent),
      processorListWidget_(processorListWidget),
      sortType_(ProcessorListTreeWidget::SORT_BY_CATEGORY),
      showCodeState_(true),
      searchDescription_(false)
{
    QStringList headeritems = QStringList();
    headeritems << "Processor" << "State";
    setHeaderLabels(headeritems);

    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setStretchLastSection(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setColumnCount(2);
    processorVector_ = ProcessorFactory::getInstance()->getKnownClasses();

    sortByCategory();
}

void ProcessorListTreeWidget::filter(const QString& text) {
    // First clear the former searches
    processorVector_.clear();
    items_.clear();

    filterText_ = text.toStdString();

    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);
        if (proc.indexOf(text, 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(processors[i]);
        if(searchDescription_) {
            QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
            if (des.indexOf(text, 0, Qt::CaseInsensitive) != -1)
                processorVector_.push_back(processors[i]);
        }
    }

    clear();
    if(sortType_ == ProcessorListTreeWidget::SORT_BY_CATEGORY)
        sortByCategory();
    else if(sortType_ == ProcessorListTreeWidget::SORT_BY_MODULENAME)
        sortByModuleName();
    else if(sortType_ == ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE)
        sortByCategoryThenModule();
    else if(sortType_ == ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY)
        sortByModuleThenCategory();
}

void ProcessorListTreeWidget::mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);

    // if not left button - return
    if (!(event->buttons() & Qt::LeftButton))
        return;

    // if no item selected, return
    if (currentItem() == NULL)
        return;

    ProcessorListItem* item = dynamic_cast<ProcessorListItem*>(itemAt(event->pos()));

    // if no ProcessorListItem selected, return
    if (!item)
        return;

    QString idStr(item->getId().c_str());

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(idStr);

    QDrag *drag = new QDrag(processorListWidget_);
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    //drag->setHotSpot(event->pos()-item->pos());

    drag->start(Qt::CopyAction);

}

void ProcessorListTreeWidget::sortByCategory() {
    sortType_ = ProcessorListTreeWidget::SORT_BY_CATEGORY;
        // First clear the former searches
    processorVector_.clear();
    items_.clear();

    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);
        if (proc.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(processors[i]);
        else if(searchDescription_) {
            QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
            if (des.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                processorVector_.push_back(processors[i]);
        }
    }
    clear();

    int position;
    QStringList categories;
    QString categoryIdentifier("");

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString categoryIdentifier = processorVector_[i].first.c_str();
        if (! categories.contains(categoryIdentifier)){
            items_.append(new QTreeWidgetItem(QStringList(categoryIdentifier)));
            categories << categoryIdentifier;
        }
    }

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString categoryIdentifier = processorVector_[i].first.c_str();
        ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);

        position = categories.indexOf(categoryIdentifier);
        items_.at(position)->addChild(tempListItem);
        tempListItem->setInfo(ProcessorFactory::getInstance()->getProcessorInfo(processorVector_[i].second));
        if(showCodeState_) {
            if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_EXPERIMENTAL ||
                ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_OBSOLETE ||
                ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_BROKEN) {

                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-broken.png"));
                tempListItem->setToolTip(1, "Code state: experimental, obsolete or broken");
            }
            else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_TESTING) {
                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-testing.png"));
                tempListItem->setToolTip(1, "Code state: testing");
            }
            else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_STABLE) {
                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-stable.png"));
                tempListItem->setToolTip(1, "Code state: stable");
            }
        }
    }

    insertTopLevelItems(0, items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
}
void ProcessorListTreeWidget::sortByModuleName() {
    sortType_ = ProcessorListTreeWidget::SORT_BY_MODULENAME;
    // First clear the former searches
    processorVector_.clear();
    items_.clear();

    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);
        if (proc.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(processors[i]);
        else if(searchDescription_) {
            QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
            if (des.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                processorVector_.push_back(processors[i]);
        }
    }
    clear();

    int position;
    QStringList moduleNames;
    QString moduleName("");

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
        if (! moduleNames.contains(moduleIdentifier)){
            items_.append(new QTreeWidgetItem(QStringList(moduleIdentifier)));
            moduleNames << moduleIdentifier;
        }
    }

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
        ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);

        position = moduleNames.indexOf(moduleIdentifier);
        items_.at(position)->addChild(tempListItem);
        ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second);
        tempListItem->setInfo(ProcessorFactory::getInstance()->getProcessorInfo(processorVector_[i].second));

        if(showCodeState_) {
            if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_EXPERIMENTAL ||
                ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_OBSOLETE ||
                ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_BROKEN) {

                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-broken.png"));
                tempListItem->setToolTip(1, "Code state: experimental, obsolete or broken");
            }
            else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_TESTING) {
                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-testing.png"));
                tempListItem->setToolTip(1, "Code state: testing");
            }
            else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_STABLE) {
                tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-stable.png"));
                tempListItem->setToolTip(1, "Code state: stable");
            }
        }
    }

    insertTopLevelItems(0, items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
}

void ProcessorListTreeWidget::sortByModuleThenCategory() {

    sortType_ = ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY;
    // First clear the former searches
    processorVector_.clear();
    items_.clear();

    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);
        if (proc.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(processors[i]);
        else if(searchDescription_) {
            QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
            if (des.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                processorVector_.push_back(processors[i]);
        }
    }
    clear();

    int position;
    QStringList moduleNames;
    QString moduleName("");

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
        if (! moduleNames.contains(moduleIdentifier)){
            moduleNames << moduleIdentifier;
        }
    }

    // do this evaluation for every category
    for (int ii = 0; ii < moduleNames.size(); ++ii) {

        //QString categoryIdentifier=categories.at(i);
        QTreeWidgetItem* moduleItem = new QTreeWidgetItem(QStringList(moduleNames.at(ii)));
        items_.append(moduleItem);

        QStringList categories;
        QString categoryIdentifier("");

        for (unsigned int i = 0; i < processorVector_.size(); ++i) {
            QString categoryIdentifier = processorVector_[i].first.c_str();
            if (! categories.contains(categoryIdentifier)){
                categories << categoryIdentifier;
            }
        }

        //for every module in the current category check processors
        for (int j = 0; j < categories.size(); ++j) {
            QTreeWidgetItem* categoryItem = new QTreeWidgetItem(QStringList(categories.at(j)));
            moduleItem->addChild(categoryItem);

            for (unsigned int i = 0; i < processorVector_.size(); ++i) {

                if(categories.at(j) == processorVector_[i].first.c_str()
                    && moduleNames.at(ii) == QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second))) {

                    QString categoryIdentifier = processorVector_[i].first.c_str();
                    ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);

                    position = categories.indexOf(categoryIdentifier);
                    categoryItem->addChild(tempListItem);
                    tempListItem->setInfo(ProcessorFactory::getInstance()->getProcessorInfo(processorVector_[i].second));
                    if(showCodeState_) {
                        if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_EXPERIMENTAL ||
                            ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_OBSOLETE ||
                            ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_BROKEN) {

                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-broken.png"));
                            tempListItem->setToolTip(1, "Code state: experimental, obsolete or broken");
                        }
                        else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_TESTING) {
                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-testing.png"));
                            tempListItem->setToolTip(1, "Code state: testing");
                        }
                        else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_STABLE) {
                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-stable.png"));
                            tempListItem->setToolTip(1, "Code state: stable");
                        }
                    }
                }
            }
            if (categoryItem->childCount() == 0) {
                    moduleItem->removeChild(categoryItem);
            }
    }

    insertTopLevelItems(0, items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
    }
}

void ProcessorListTreeWidget::sortByCategoryThenModule() {

    sortType_ = ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE;

    // First clear the former searches
    processorVector_.clear();
    items_.clear();

    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);
        if (proc.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(processors[i]);
        else if(searchDescription_) {
            QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
            if (des.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                processorVector_.push_back(processors[i]);
        }
    }

    clear();

    int position;
    QStringList categories;
    QString categoryIdentifier("");

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString categoryIdentifier = processorVector_[i].first.c_str();
        if (! categories.contains(categoryIdentifier)){
            categories << categoryIdentifier;
        }
    }

    // do this evaluation for every category
    for (int i = 0; i < categories.size(); ++i) {

        QString categoryIdentifier=categories.at(i);
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(QStringList(categoryIdentifier));
        items_.append(categoryItem);

        QStringList moduleNames;
        QString moduleName("");

        for (unsigned int i = 0; i < processorVector_.size(); ++i) {
            QString moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
            if (! moduleNames.contains(moduleIdentifier)){
                moduleNames << moduleIdentifier;
            }
        }

        //for every module in the current category check processors
        for (int j = 0; j < moduleNames.size(); ++j) {
            QTreeWidgetItem* moduleItem = new QTreeWidgetItem(QStringList(moduleNames.at(j)));
            categoryItem->addChild(moduleItem);

            for (unsigned int i = 0; i < processorVector_.size(); ++i) {

                if(categoryIdentifier == processorVector_[i].first.c_str()
                    && moduleNames.at(j) == QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second))) {

                    QString categoryIdentifier = processorVector_[i].first.c_str();
                    ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);

                    position = categories.indexOf(categoryIdentifier);
                    moduleItem->addChild(tempListItem);
                    tempListItem->setInfo(ProcessorFactory::getInstance()->getProcessorInfo(processorVector_[i].second));
                    if(showCodeState_) {
                        if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_EXPERIMENTAL ||
                            ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_OBSOLETE ||
                            ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_BROKEN) {

                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-broken.png"));
                            tempListItem->setToolTip(1, "Code state: experimental, obsolete or broken");
                        }
                        else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_TESTING) {
                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-testing.png"));
                            tempListItem->setToolTip(1, "Code state: testing");
                        }
                        else if(ProcessorFactory::getInstance()->getProcessorCodeState(processorVector_[i].second) == Processor::CODE_STATE_STABLE) {
                            tempListItem->setIcon(1, QIcon(":/voreenve/icons/processor-stable.png"));
                            tempListItem->setToolTip(1, "Code state: stable");
                        }
                    }


                }
            }
            if (moduleItem->childCount() == 0) {
                    categoryItem->removeChild(moduleItem);
            }
    }

    insertTopLevelItems(0, items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
    }
}

void ProcessorListTreeWidget::hideStatus(bool hidden) {
    showCodeState_ = hidden;
    filter(QString::fromStdString(filterText_));
}

void ProcessorListTreeWidget::searchDescription(bool search) {
    searchDescription_ = search;
    filter(QString::fromStdString(filterText_));
}

void ProcessorListTreeWidget::sort(int type) {
    if(type == ProcessorListTreeWidget::SORT_BY_CATEGORY)
        sortByCategory();
    if(type == ProcessorListTreeWidget::SORT_BY_MODULENAME)
        sortByModuleName();
    if(type == ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE)
        sortByCategoryThenModule();
    if(type == ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY)
        sortByModuleThenCategory();
}

// ----------------------------------------------------------------------------

ProcessorListItem::ProcessorListItem (const std::string& id)
    :  QTreeWidgetItem(QStringList(QString(id.c_str()))),
    id_(id)
{}

} //namespace voreen
