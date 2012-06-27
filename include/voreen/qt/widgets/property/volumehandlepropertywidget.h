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

#ifndef VRN_VOLUMEHANDLEPROPERTYWIDGET_H
#define VRN_VOLUMEHANDLEPROPERTYWIDGET_H

#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/vis/properties/volumehandleproperty.h"
#include "voreen/core/vis/processors/volume/volumesourceprocessor.h"

#include "voreen/qt/widgets/property/qpropertywidget.h"

#include <QDialog>
#include <QComboBox>

class QLabel;
class QPushButton;

namespace voreen {

class VolumeHandlePropertyWidget : public QPropertyWidget, public VolumeCollectionObserver {
    Q_OBJECT

public:
    VolumeHandlePropertyWidget(VolumeHandleProperty* volumeHandleProp, QWidget* parent);

    virtual void updateFromProperty();
    void setVolumeContainer(VolumeContainer*);

    /**
    * These are virtual methods of the VolumeCollectionObserver Interface
    */
    void volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    void volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);


protected:
    void updateFromContainer();

    static const std::string loggerCat_;

protected slots:
    void newFileDialog();
    void changeVolume(int);
    void filterChanged(QString);

private:
    VolumeHandle* getVolume() const;

    QLabel* previewLabel_;
    QPushButton* loadVolumeButton_;
    QLabel* infoLabel_;
    QComboBox* volumeSelectorBox_;

    QLabel* nameLabel_;
    QLabel* pathLabel_;
    QLabel* dimensionLabel_;
    QLabel* spacingLabel_;
    QLabel* memSizeLabel_;

    VolumeContainer* volumeContainer_;

    //Indicates the FilenameFilter of the volume opening dialog
    bool rawSelected_;

};

} // namespace voreen

#endif // VRN_VOLUMEHANDLEPROPERTYWIDGET_H
