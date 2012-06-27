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

#ifndef VRN_VOLUMECONTAINERWIDGET_H
#define VRN_VOLUMECONTAINERWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/datastructures/volume/volume.h"

#ifdef VRN_MODULE_DICOM
#include "voreen/modules/dicom/dicomvolumereader.h"
#include "voreen/modules/dicom/qt/dicomdialog.h"
#endif

class QPixmap;
class VolumeLoadButton;

namespace voreen {

class QRCTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    QRCTreeWidget(QWidget* parent = 0);

signals:
    void refresh(QTreeWidgetItem*);
    void remove();
    void add();
    void addRaw();
    void exportDat();

protected:
    void keyPressEvent(QKeyEvent* event);
    void contextMenuEvent(QContextMenuEvent*);
};

// ---------------------------------------------------------------------------

class VolumeContainerWidget : public QWidget, public VolumeCollectionObserver  {
    Q_OBJECT
public:

    VolumeContainerWidget(VolumeContainer* container, QWidget* parent = 0);
    ~VolumeContainerWidget();

    void setVolumeContainer(VolumeContainer* volumeContainer);

    virtual QSize sizeHint() const;

    /// @see VolumeCollectionObserver
    void volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    /// @see VolumeCollectionObserver
    void volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    /// @see VolumeCollectionObserver
    void volumeChanged(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);

    void loadDicomFiles();
    void loadVolume();
    void loadVolumeRawFilter();

protected:
    void keyPressEvent(QKeyEvent*);
    std::string calculateSize();

    VolumeLoadButton* volumeLoadButton_;
    QLabel* containerInfo_;
    std::map<Volume*, QTreeWidgetItem*> volumeItems_;

private:
    void update();

    VolumeContainer* volumeContainer_;

    /**
    * QTreeWidget with custom context Menu is only applied
    * by the volumecontainer for now
    */
    QRCTreeWidget* volumeInfos_;

    static const std::string loggerCat_;

private slots:

    /// removes a volume
    void removeVolume();
    /// reloads a volume into memory
    void volumeRefresh(QTreeWidgetItem*);
    /// exports currently selected volumes
    void exportDat();

    void resizeOnCollapse(bool);

};

} // namespace

#endif
