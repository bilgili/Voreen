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

#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/properties/volumecollectionproperty.h"

#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/widgets/property/volumecollectionpropertywidget.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
#ifdef __APPLE__
    int fontSize = 13;
#else
    int fontSize = 8;
#endif
}

namespace voreen {

const std::string VolumeCollectionPropertyWidget::loggerCat_("voreen.qt.VolumeCollectionPropertyWidget");

VolumeCollectionPropertyWidget::VolumeCollectionPropertyWidget(VolumeCollectionProperty* volumeCollectionProp, QWidget* parent) :
        QPropertyWidget(volumeCollectionProp, parent, false),
        volumeContainer_(0)
{
    tgtAssert(prop_, "No volume collection property");

    setFocusPolicy(Qt::StrongFocus);
    QVBoxLayout* mainLayout = new QVBoxLayout();
    layout_->addLayout(mainLayout);

    volumeInfos_ = new QTreeWidget(this);
    QTreeWidgetItem* header = volumeInfos_->headerItem();
    header->setText(0, tr(""));
    volumeInfos_->setColumnCount(1);
    volumeInfos_->show();
    volumeInfos_->setIconSize(QSize(50,50));
    volumeInfos_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    connect(volumeInfos_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(updateCollection(QTreeWidgetItem*, int)));
    connect(volumeInfos_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SIGNAL(widgetChanged()));
    mainLayout->addWidget(volumeInfos_);

    selectAll_ = new QCheckBox("select All", this);
    selectAll_->move(8, 0);
    connect(selectAll_, SIGNAL(toggled(bool)), this, SLOT(selectAll(bool)));

    updateWidget();
}

void VolumeCollectionPropertyWidget::updateFromProperty() {
    updateWidget();
}

void VolumeCollectionPropertyWidget::updateCollection() {
    if (!prop_ || !volumeContainer_)
        return;

    VolumeCollection* collection = static_cast<VolumeCollectionProperty*>(prop_)->get();
    if (!collection) {
        tgtAssert(false, "No volumecollection");
        return;
    }

    collection->clear();
    QList<QTreeWidgetItem*> items = volumeInfos_->findItems(".", Qt::MatchContains);
    for(size_t i = 0; i < volumeContainer_->size(); i++) {
        if (items.at(i)->checkState(0) == Qt::Checked) {
            collection->add(volumeContainer_->at(i));
        }
    }
    prop_->invalidate();

}

void VolumeCollectionPropertyWidget::updateCollection(QTreeWidgetItem*, int) {
    updateCollection();
}

void VolumeCollectionPropertyWidget::volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    updateWidget();
}

void VolumeCollectionPropertyWidget::volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    updateCollection();
    updateWidget();
}

void VolumeCollectionPropertyWidget::updateWidget() {
    if (!prop_ || !volumeContainer_)
        return;

    VolumeCollection* collection = static_cast<VolumeCollectionProperty*>(prop_)->get();
    if (!collection) {
        tgtAssert(false, "No volumecollection");
        return;
    }

    volumeInfos_->clear();
    for(size_t i = 0 ; i< volumeContainer_->size(); i++) {
        VolumeHandle* handle = volumeContainer_->at(i);
        Volume* volume = volumeContainer_->at(i)->getVolume();
        QTreeWidgetItem* qtwi = new QTreeWidgetItem(volumeInfos_);

        qtwi->setFont(0, QFont(QString("Arial"), fontSize));
        QString info = QString::fromStdString(VolumeViewHelper::getStrippedVolumeName(handle) + "\n"
                                               + VolumeViewHelper::getVolumePath(handle));
        qtwi->setText(0, info);
        qtwi->setIcon(0, QIcon(VolumeViewHelper::generateBorderedPreview(volume, 27, 0)));
        qtwi->setSizeHint(0,QSize(27,27));
        qtwi->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        // set tree widget to checked, if the corresponding volume handle is contained by the property's collection
        // TODO: check for volume handle object instead of origin, when new serialization is in place
        qtwi->setCheckState(0, collection->selectOrigin(handle->getOrigin())->empty() ? Qt::Unchecked : Qt::Checked);

        volumeInfos_->addTopLevelItem(qtwi);
    }

}

void VolumeCollectionPropertyWidget::setVolumeContainer(VolumeContainer* volumeContainer) {

    stopObservation(volumeContainer_);

    volumeContainer_ = volumeContainer;

    if (volumeContainer_)
        volumeContainer_->addObserver(this);

    updateWidget();
}

void VolumeCollectionPropertyWidget::selectAll(bool toggle) {
    QList<QTreeWidgetItem*> items = volumeInfos_->findItems(".", Qt::MatchContains);
    QList<QTreeWidgetItem*>::iterator it = items.begin();
    while(it != items.end()) {
        (*it)->setCheckState(0, toggle ? Qt::Checked : Qt::Unchecked);
        it++;
    }
    updateCollection();
}

CustomLabel* VolumeCollectionPropertyWidget::getNameLabel() const {
    return 0;
}


} //namespace
