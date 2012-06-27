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

#ifndef VRN_VOLUMECOLLECTIONPROPERTYWIDGET_H
#define VRN_VOLUMECOLLECTIONPROPERTYWIDGET_H


#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/properties/volumecollectionproperty.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/properties/volumehandleproperty.h"
#include "voreen/modules/base/processors/datasource/volumesource.h"

#include "voreen/qt/widgets/property/qpropertywidget.h"

#include <QDialog>
#include <QCheckBox>
#include <QTreeWidget>

namespace voreen {


class VolumeCollectionPropertyWidget : public QPropertyWidget, public VolumeCollectionObserver {
    Q_OBJECT

public:
    VolumeCollectionPropertyWidget(VolumeCollectionProperty* volumeCollectionProp, QWidget* parent);

    virtual void updateFromProperty();

    /**
    * These are virtual methods of the VolumeCollectionObserver Interface
    */
    void volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    void volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);

    void setVolumeContainer(VolumeContainer*);

    /// Returns the null pointer, since this widget does not need a separate label.
    virtual const QLabel* getNameLabel() const;

protected:
    static const std::string loggerCat_;

private:
    QTreeWidget* volumeInfos_;
    VolumeContainer* volumeContainer_;
    void updateWidget();
    QCheckBox* selectAll_;

private slots:
    void updateCollection();
    void updateCollection(QTreeWidgetItem*, int);
    void selectAll(bool);

};

} //namespace

#endif // VRN_VOLUMECOLLECTIONPROPERTYWIDGET_H
