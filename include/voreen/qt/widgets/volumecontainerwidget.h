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

#ifndef VRN_VOLUMECONTAINERWIDGET_H
#define VRN_VOLUMECONTAINERWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/volume/volume.h"

#ifdef VRN_WITH_DCMTK
#include "voreen/core/io/dicomvolumereader.h"
#include "voreen/qt/dicomdialog.h"
#endif

class QPixmap;

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
    void contextMenuEvent(QContextMenuEvent*);
};

// ---------------------------------------------------------------------------

class VolumeContainerWidget : public QWidget, public VolumeCollectionObserver  {
    Q_OBJECT
public:

    VolumeContainerWidget(VolumeContainer* container, QWidget* parent = 0);
    ~VolumeContainerWidget();

    void setVolumeContainer(VolumeContainer* volumeContainer);

    /// @see VolumeCollectionObserver
    void volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);
    /// @see VolumeCollectionObserver
    void volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/);

public slots:
    void loadVolume();
    void loadRawVolume(std::string);
    void loadDicomFiles();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    void update();
    const std::string getExtensions() const;

    VolumeContainer* volumeContainer_;

    QRCTreeWidget* volumeInfos_;
    bool rawSelected_;
    //QCheckBox* rawCheckBox_;

#ifdef VRN_WITH_DCMTK
    DicomDirDialog* dicomDirDialog_;
#endif

    static const std::string loggerCat_;

private slots:

    std::vector<std::string> openFileDialog();
    void addMultipleVolumes(std::vector<std::string>);

    void loadDicomDir(const std::string& file);
    void loadDicomFiles(const std::string& dir);
    void dicomDirDialogFinished();

    void removeVolume();
    void volumeRefresh(QTreeWidgetItem*);
    void exportDat();

    void resizeOnCollapse(bool);
    void filterChanged(QString);

};

} // namespace

#endif
