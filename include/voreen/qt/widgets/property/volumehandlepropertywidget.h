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

#ifndef VRN_VOLUMEHANDLEPROPERTYWIDGET_H
#define VRN_VOLUMEHANDLEPROPERTYWIDGET_H

#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/properties/volumehandleproperty.h"
#include "voreen/modules/base/processors/datasource/volumesource.h"

#include "voreen/qt/widgets/customlabel.h"
#include "voreen/qt/widgets/volumeloadbutton.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"

#include <QDialog>

class QLabel;
class QPushButton;
class QComboBox;

namespace voreen {

class VolumeHandlePropertyWidget : public QPropertyWidget, public VolumeCollectionObserver {
    Q_OBJECT

public:
    VolumeHandlePropertyWidget(VolumeHandleProperty* volumeHandleProp, QWidget* parent);

    virtual void updateFromProperty();
    void setVolumeContainer(VolumeContainer*);

    /// @see VolumeCollectionObserver
    void volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    /// @see VolumeCollectionObserver
    void volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    /// @see VolumeCollectionObserver
    void volumeChanged(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);

    /// Returns the null pointer, since this widget does not need a separate label.
    virtual const QLabel* getNameLabel() const;

protected:
    void updateFromContainer();
    /// @see QPropertyWidget
    void showNameLabel(bool);

    static const std::string loggerCat_;
    VolumeLoadButton* volumeLoadButton_;

protected slots:
    void changeVolume(int);

private:
    VolumeHandle* getVolume() const;

    QLabel* previewLabel_;
    QLabel* infoLabel_;
    QComboBox* volumeSelectorBox_;

    CustomLabel* volumeNameLabel_;
    CustomLabel* pathLabel_;
    CustomLabel* dimensionLabel_;
    CustomLabel* spacingLabel_;
    CustomLabel* memSizeLabel_;

    VolumeContainer* volumeContainer_;

};

} // namespace voreen

#endif // VRN_VOLUMEHANDLEPROPERTYWIDGET_H
