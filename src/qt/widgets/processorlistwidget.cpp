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

#include "voreen/qt/widgets/processorlistwidget.h"
#include "voreen/qt/widgets/lineeditresetwidget.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QAction>
#include <QApplication>
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
    : QWidget(parent)
    , tree_(new ProcessorListTreeWidget(this))
    , resetIcon_()
    , info_(new QTextBrowser())
    , splitter_(new QSplitter(Qt::Vertical))
{
    QPushButton* sortButton = new QPushButton(this);
    sortButton->setIcon(QIcon(":/voreenve/icons/configure.png"));
    sortButton->setGeometry(0,0,32,32);
    sortButton->setFlat(true);
    sortButton->setToolTip(tr("Sort processor list"));

    edit_ = new LineEditResetWidget(this);
    edit_->setToolTip(tr("Filter processor list"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* filterSortLayout = new QHBoxLayout();
    info_->resize(120, 100);
    info_->setReadOnly(true);

    connect(edit_, SIGNAL(textChanged(const QString&)), tree_, SLOT(filter(const QString&)));
    connect(tree_, SIGNAL(itemSelectionChanged()), this, SLOT(setInfo()));
    connect(sortButton, SIGNAL(clicked()), this, SLOT(sortMenu()));
    connect(this, SIGNAL(sort(int)), tree_, SLOT(sort(int)));
    connect(this, SIGNAL(hideStatus(bool)), tree_, SLOT(hideStatus(bool)));
    connect(this, SIGNAL(searchDescription(bool)), tree_, SLOT(searchDescription(bool)));

    connect(this, SIGNAL(showModule(QString, bool)), tree_, SLOT(setModuleNameVisibility(QString, bool)));

    filterSortLayout->addWidget(edit_);
    filterSortLayout->addWidget(sortButton);
    splitter_->addWidget(tree_);
    splitter_->addWidget(info_);
    QList<int> widgetSpacing;
    widgetSpacing << 500 << 100;
    splitter_->setSizes(widgetSpacing);
    mainLayout->addLayout(filterSortLayout);
    mainLayout->addWidget(splitter_);

    hideAction_ = new QAction(QString::fromStdString("Show Code State"), this);
    hideAction_->setCheckable(true);
    hideAction_->setChecked(true);
    searchDescription_ = new QAction(QString::fromStdString("Search Processor Descriptions"), this);
    searchDescription_->setCheckable(true);

    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();
    //get all moduleNames and set bool for visibility filtration
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    std::string moduleName;
    //pf->initializeClassList();
    for (size_t i = 0; i < processors.size(); ++i) {
        moduleName = pf->getProcessorModuleName(processors.at(i).second);
        moduleVisibility_[moduleName] = true;
        //emit showModule(QString::fromStdString(moduleName), false);     // this must be done another way
        tree_->moduleVisibility_[moduleName] = true;
    }
    sbc_ = new QAction("Sort by Category", this);
    sbm_ = new QAction("Sort by Module", this);
    sbmtc_ = new QAction("Sort by Module, then Category", this);
    sbctm_ = new QAction("Sort by Category, then Module", this);
    sbc_->setCheckable(true);
    sbm_->setCheckable(true);
    sbctm_->setCheckable(true);
    sbmtc_->setCheckable(true);

    css_ = new QAction("Show Code State 'stable'", this);
    cst_ = new QAction("Show Code State 'testing'", this);
    csb_ = new QAction("Show Code State 'experimental'", this);
    css_->setCheckable(true);
    cst_->setCheckable(true);
    csb_->setCheckable(true);
    moduleVisibilityMenu_ = new QMenu("Show Modules", this);
    std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
    moduleVisibilityMenu_->addAction("Check All");
    moduleVisibilityMenu_->addAction("Uncheck All");
    moduleVisibilityMenu_->addSeparator();
    loadSettings();
    while(it != moduleVisibility_.end()) {
        QAction* tempAction = new QAction(QString::fromStdString((*it).first), this);
        moduleVisibilityActions_[(*it).first] = tempAction;
        moduleVisibilityMenu_->addAction(tempAction);
        tempAction->setCheckable(true);
        tempAction->setChecked((*it).second);
        ++it;
    }
}

ProcessorListWidget::~ProcessorListWidget() {
    saveSettings();
}

void ProcessorListWidget::loadSettings()
{
    QSettings settings;
    settings.beginGroup("ProcessorListWidget");
    if(!settings.contains("core")) {    // first time load without any data written in the settings
        sbc_->setChecked(true);         // first time and no data is stored in qsettings
        std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
        while(it != moduleVisibility_.end()) {
            moduleVisibility_[(*it).first] = true;
            tree_->setModuleNameVisibility(QString::fromStdString((*it).first), true);
            it++;
        }
        hideAction_->setChecked(true);
        searchDescription_->setChecked(false);
        css_->setChecked(true);
        cst_->setChecked(true);
        csb_->setChecked(true);
        tree_->codeState_.insert(Processor::CODE_STATE_STABLE);
        tree_->codeState_.insert(Processor::CODE_STATE_TESTING);
        tree_->codeState_.insert(Processor::CODE_STATE_BROKEN);
        tree_->codeState_.insert(Processor::CODE_STATE_OBSOLETE);
        tree_->codeState_.insert(Processor::CODE_STATE_EXPERIMENTAL);
        settings.endGroup();
        sort(ProcessorListTreeWidget::SORT_BY_CATEGORY);
        return;
    }
    std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
    while(it != moduleVisibility_.end()) {
        if(settings.value(QString::fromStdString((*it).first)).toString() == "") {
            moduleVisibility_[(*it).first] = true;
            tree_->setModuleNameVisibility(QString::fromStdString((*it).first), true);
        }
        else {
            moduleVisibility_[(*it).first] = settings.value(QString::fromStdString((*it).first)).toBool();
            tree_->setModuleNameVisibility(QString::fromStdString((*it).first), settings.value(QString::fromStdString((*it).first)).toBool());
        }
        it++;
    }
    tree_->filterText_ = settings.value("filtertext").toString().toStdString() ;
    edit_->setText(QString::fromStdString(tree_->filterText_));

    hideAction_->setChecked(settings.value("hideCodeStatus").toBool());
    searchDescription_->setChecked(settings.value("searchDescription").toBool());
    css_->setChecked(settings.value("csw").toBool());
    cst_->setChecked(settings.value("cst").toBool());
    csb_->setChecked(settings.value("csb").toBool());
    if(css_->isChecked()) {
        tree_->codeState_.insert(Processor::CODE_STATE_STABLE);
        css_->setChecked(true);
    }
    else {
        tree_->codeState_.erase(Processor::CODE_STATE_STABLE);
        css_->setChecked(false);
    }
    if(cst_->isChecked()) {
        tree_->codeState_.insert(Processor::CODE_STATE_TESTING);
        cst_->setChecked(true);
    }
    else {
        tree_->codeState_.erase(Processor::CODE_STATE_TESTING);
        cst_->setChecked(false);
    }

    if(csb_->isChecked()) {
        tree_->codeState_.insert(Processor::CODE_STATE_BROKEN);
        tree_->codeState_.insert(Processor::CODE_STATE_OBSOLETE);
        tree_->codeState_.insert(Processor::CODE_STATE_EXPERIMENTAL);
        csb_->setChecked(true);
    }
    else {
        tree_->codeState_.erase(Processor::CODE_STATE_BROKEN);
        tree_->codeState_.erase(Processor::CODE_STATE_OBSOLETE);
        tree_->codeState_.erase(Processor::CODE_STATE_EXPERIMENTAL);
        csb_->setChecked(false);
    }
    if(settings.value("sbc").toBool()) {
        sbc_->setChecked(true);
        sort(ProcessorListTreeWidget::SORT_BY_CATEGORY);
    }
    else if(settings.value("sbm").toBool()) {
        sbm_->setChecked(true);
        sort(ProcessorListTreeWidget::SORT_BY_MODULENAME);
    }
    else if(settings.value("sbctm").toBool()) {
        sbctm_->setChecked(true);
        sort(ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE);
    }
    else if(settings.value("sbmtc").toBool()) {
        sbmtc_->setChecked(true);
        sort(ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY);
    }

    settings.endGroup();
}

void ProcessorListWidget::saveSettings() {
    QSettings settings;
    settings.beginGroup("ProcessorListWidget");
    std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
    while(it != moduleVisibility_.end()) {
        QVariant val = QVariant(moduleVisibility_[(*it).first]);
        settings.setValue(QString::fromStdString((*it).first), val);
        it++;
    }
    settings.setValue("filtertext", QString::fromStdString(tree_->filterText_));

    settings.setValue("sbc", sbc_->isChecked());
    settings.setValue("sbm", sbm_->isChecked());
    settings.setValue("sbmtc", sbmtc_->isChecked());
    settings.setValue("sbctm", sbctm_->isChecked());

    settings.setValue("csw", css_->isChecked());
    settings.setValue("cst", cst_->isChecked());
    settings.setValue("csb", csb_->isChecked());

    settings.setValue("hideCodeStatus", hideAction_->isChecked());
    settings.setValue("searchDescription", searchDescription_->isChecked());
    settings.endGroup();
}

void ProcessorListWidget::setInfo() {
    ProcessorListItem* currentItem = dynamic_cast<ProcessorListItem *>(tree_->currentItem());
    if (currentItem)
        info_->setHtml(QString::fromStdString(currentItem->getInfo()));
    else
        info_->setHtml("");
}

void ProcessorListWidget::setInfo(Processor* processor) {
    if (processor && !processor->getProcessorInfo().empty())
        info_->setHtml(QString::fromStdString("<b>" + processor->getClassName() + "</b><br>" + processor->getProcessorInfo()));
    else
        info_->setHtml("");
}

void ProcessorListWidget::sortMenu() {
    QMenu* menu = new QMenu();
    QActionGroup* sortActions = new QActionGroup(this);
    sortActions->addAction(sbc_);
    sortActions->addAction(sbm_);
    sortActions->addAction(sbctm_);
    sortActions->addAction(sbmtc_);
    // sort by category
    menu->addAction(sbc_);
    // sort by module
    menu->addAction(sbm_);
    // sort by category then module
    menu->addAction(sbctm_);
    //sort by module then category
    menu->addAction(sbmtc_);
    menu->addSeparator();

    menu->addAction(hideAction_);
    menu->addAction(searchDescription_);
    menu->addSeparator();

    menu->addAction(css_);
    menu->addAction(cst_);
    menu->addAction(csb_);
    menu->addSeparator();

    menu->addMenu(moduleVisibilityMenu_);
    QAction* action = menu->exec(QCursor::pos());
    if (action) {
        if(action == sbc_) {
            emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY);

        }
        else if (action == sbm_) {
            emit sort(ProcessorListTreeWidget::SORT_BY_MODULENAME);

        }
        else if (action == sbctm_) {
            emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE);

        }
        else if (action == sbmtc_) {
            emit sort(ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY);

        }
        if(action == css_ || action == cst_ || action == csb_) {
            if (action == css_) {
                if(css_->isChecked())
                    tree_->codeState_.insert(Processor::CODE_STATE_STABLE);
                else
                    tree_->codeState_.erase(Processor::CODE_STATE_STABLE);
            }
            else if (action == cst_) {
                if(cst_->isChecked())
                    tree_->codeState_.insert(Processor::CODE_STATE_TESTING);
                else
                    tree_->codeState_.erase(Processor::CODE_STATE_TESTING);
            }
            else if (action == csb_) {
                    if(csb_->isChecked()) {
                        tree_->codeState_.insert(Processor::CODE_STATE_BROKEN);
                        tree_->codeState_.insert(Processor::CODE_STATE_OBSOLETE);
                        tree_->codeState_.insert(Processor::CODE_STATE_EXPERIMENTAL);
                    }
                    else {
                        tree_->codeState_.erase(Processor::CODE_STATE_BROKEN);
                        tree_->codeState_.erase(Processor::CODE_STATE_OBSOLETE);
                        tree_->codeState_.erase(Processor::CODE_STATE_EXPERIMENTAL);
                    }
            }
            if(sbc_->isChecked())
                emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY);
            if(sbm_->isChecked())
                emit sort(ProcessorListTreeWidget::SORT_BY_MODULENAME);
            if(sbctm_->isChecked())
                emit sort(ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE);
            if(sbmtc_->isChecked())
                emit sort(ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY);
        }
        else if (action == hideAction_) {
            emit hideStatus(hideAction_->isChecked());

        }
        else if (action == searchDescription_) {
            emit searchDescription(searchDescription_->isChecked());
        }
        else {
            emit showModule(action->text(), action->isChecked());
            setModuleNameVisibility(action->text().toStdString(), action->isChecked());
        }
        //saveSettings();
    }
}

void ProcessorListWidget::setModuleNameVisibility(std::string name, bool visibility) {
    bool checked;
    if(name == "Uncheck All" || name == "Check All"){
        if(name == "Uncheck All") {
            checked = false;
        }
        else
            checked = true;

        std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
        while(it != moduleVisibility_.end()) {
            (*it).second = checked;
            it++;
        }
        std::map<std::string, QAction*>::iterator it2 = moduleVisibilityActions_.begin();
        while(it2 != moduleVisibilityActions_.end()) {
            (*it2).second->setChecked(checked);
            it2++;
        }

    }
    else
        moduleVisibility_[name] = visibility;
        //moduleVisibilityActions_[name]->setChecked(visibility);
    //saveSettings();

}

void ProcessorListWidget::processorsSelected(const QList<Processor*>& processors) {
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

    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    //get all moduleNames and set bool for visibility filtration
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    std::string moduleName;
    for (size_t i = 0; i < processors.size(); ++i) {
        moduleName = pf->getProcessorModuleName(processors.at(i).second);
        moduleVisibility_[moduleName] = true;
    }
    sortByCategory();
}

void ProcessorListTreeWidget::filter(const QString& text) {
    filterText_ = text.toStdString();
    ProcessorFactory::KnownClassesVector processors = getVisibleProcessors();

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

void ProcessorListTreeWidget::setModuleNameVisibility(QString module, bool visible) {
    bool checked;
    if(module == "Uncheck All" || module == "Check All"){
        if(module == "Uncheck All") {
            checked = false;
        }
        else
            checked = true;
            std::map<std::string, bool>::iterator it = moduleVisibility_.begin();
        while(it != moduleVisibility_.end()) {
            (*it).second = checked;
            it++;
        }
    }
    else
        moduleVisibility_[module.toStdString()] = visible;
    //filter(QString::fromStdString(filterText_));
    sort(sortType_);

}

ProcessorFactory::KnownClassesVector ProcessorListTreeWidget::getVisibleProcessors() {
    processorVector_.clear();
    items_.clear();

    ProcessorFactory* pf = ProcessorFactory::getInstance();
    // Get all available processor ids
    ProcessorFactory::KnownClassesVector processors = ProcessorFactory::getInstance()->getKnownClasses();

    for (size_t i = 0; i < processors.size(); ++i) {
        QString proc = QString::fromStdString(processors[i].second);

        if(codeState_.find(pf->getProcessorCodeState(processors[i].second)) != codeState_.end()/*pf->getProcessorCodeState(processors[i].second) == Processor::CODE_STATE_EXPERIMENTAL ||*/) {

            if(moduleVisibility_[pf->getProcessorModuleName(proc.toStdString())]) {
                if (proc.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                    processorVector_.push_back(processors[i]);
                else if(searchDescription_) {
                    QString des = QString::fromStdString(pf->getProcessorInfo(processors[i].second));
                    if (des.indexOf(QString::fromStdString(filterText_), 0, Qt::CaseInsensitive) != -1)
                        processorVector_.push_back(processors[i]);
                }
            }
        }

    }
    return processors;
}

void ProcessorListTreeWidget::sortByCategory() {
    sortType_ = ProcessorListTreeWidget::SORT_BY_CATEGORY;

    ProcessorFactory::KnownClassesVector processors = getVisibleProcessors();

    clear();
    QString categoryIdentifier("");
    std::map<QString, QTreeWidgetItem*> categories;

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        categoryIdentifier = processorVector_[i].first.c_str();
        if(categories.find(categoryIdentifier) == categories.end()) {
            categories[categoryIdentifier] = new QTreeWidgetItem(QStringList(categoryIdentifier));
        }
    }

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString categoryIdentifier = processorVector_[i].first.c_str();
        ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);
        categories[categoryIdentifier]->addChild(tempListItem);
        tempListItem->setProcessorInfo(processorVector_[i].second);
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

    std::map<QString, QTreeWidgetItem*>::iterator it = categories.begin();
    while(it != categories.end()) {
        addTopLevelItem((*it).second);
        ++it;
    }
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
}
void ProcessorListTreeWidget::sortByModuleName() {
    sortType_ = ProcessorListTreeWidget::SORT_BY_MODULENAME;
    // First clear the former searches
    ProcessorFactory::KnownClassesVector processors = getVisibleProcessors();
    clear();

    QString moduleIdentifier("");
    std::map<QString, QTreeWidgetItem*> modules;

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
        if(modules.find(moduleIdentifier) == modules.end()) {
            modules[moduleIdentifier] = new QTreeWidgetItem(QStringList(moduleIdentifier));
        }
    }

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString moduleIdentifier = QString::fromStdString(ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second));
        ProcessorListItem* tempListItem = new ProcessorListItem(processorVector_[i].second);

        modules[moduleIdentifier]->addChild(tempListItem);
        ProcessorFactory::getInstance()->getProcessorModuleName(processorVector_[i].second);
        tempListItem->setProcessorInfo(processorVector_[i].second);

        if(showCodeState_) {
            setCodeStateIcon(processorVector_[i].second, tempListItem);
        }
    }

    std::map<QString, QTreeWidgetItem*>::iterator it = modules.begin();
    while(it != modules.end()) {
        addTopLevelItem((*it).second);
        ++it;
    }

    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
}

void ProcessorListTreeWidget::sortByModuleThenCategory() {

    sortType_ = ProcessorListTreeWidget::SORT_BY_MODULE_CATEGORY;
    ProcessorFactory::KnownClassesVector processors = getVisibleProcessors();
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
                    tempListItem->setProcessorInfo(processorVector_[i].second);
                    if(showCodeState_) {
                        setCodeStateIcon(processorVector_[i].second, tempListItem);
                    }
                }
            }
            if (categoryItem->childCount() == 0) {
                    moduleItem->removeChild(categoryItem);
            }
    }

    addTopLevelItems(items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
    }
    QTreeWidget::sortItems(0, Qt::AscendingOrder);
}

void ProcessorListTreeWidget::sortByCategoryThenModule() {

    sortType_ = ProcessorListTreeWidget::SORT_BY_CATEGORY_MODULE;
    ProcessorFactory::KnownClassesVector processors = getVisibleProcessors();
    clear();

    int position;
    QStringList categories;
    QString categoryIdentifier("");

    for (unsigned int i = 0; i < processorVector_.size(); ++i) {
        QString categoryIdentifier = processorVector_[i].first.c_str();
        if (!categories.contains(categoryIdentifier)){
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
            if (!moduleNames.contains(moduleIdentifier)){
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
                    tempListItem->setProcessorInfo(processorVector_[i].second);
                    if(showCodeState_) {
                        setCodeStateIcon(processorVector_[i].second, tempListItem);
                    }


                }
            }
            if (moduleItem->childCount() == 0) {
                    categoryItem->removeChild(moduleItem);
            }
    }

    addTopLevelItems(items_);
    expandAll();
    if(!showCodeState_)
        header()->hide();
    else
        header()->show();
    }
}

void ProcessorListTreeWidget::setCodeStateIcon(std::string codeState, QTreeWidgetItem* item) {
    if(ProcessorFactory::getInstance()->getProcessorCodeState(codeState) == Processor::CODE_STATE_EXPERIMENTAL ||
        ProcessorFactory::getInstance()->getProcessorCodeState(codeState) == Processor::CODE_STATE_OBSOLETE ||
        ProcessorFactory::getInstance()->getProcessorCodeState(codeState) == Processor::CODE_STATE_BROKEN) {

        item->setIcon(1, QIcon(":/voreenve/icons/processor-broken.png"));
        item->setToolTip(1, "Code state: experimental, obsolete or broken");
    }
    else if(ProcessorFactory::getInstance()->getProcessorCodeState(codeState) == Processor::CODE_STATE_TESTING) {
        item->setIcon(1, QIcon(":/voreenve/icons/processor-testing.png"));
        item->setToolTip(1, "Code state: testing");
    }
    else if(ProcessorFactory::getInstance()->getProcessorCodeState(codeState) == Processor::CODE_STATE_STABLE) {
        item->setIcon(1, QIcon(":/voreenve/icons/processor-stable.png"));
        item->setToolTip(1, "Code state: stable");
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

void ProcessorListItem::setProcessorInfo(const std:: string& name) {
    if (!name.empty() && !ProcessorFactory::getInstance()->getProcessorInfo(name).empty())
        info_ = "<b>" + name + "</b><br>" + ProcessorFactory::getInstance()->getProcessorInfo(name);
    else
        info_ = "";
}


} //namespace voreen
