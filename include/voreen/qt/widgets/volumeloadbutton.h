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

#ifndef VRN_VOLUMELOADBUTTON_H
#define VRN_VOLUMELOADBUTTON_H

#include <QToolButton>

#ifdef VRN_MODULE_DICOM
#include "voreen/modules/dicom/dicomvolumereader.h"
#include "voreen/modules/dicom/qt/dicomdialog.h"
#endif

namespace voreen {

class VolumeContainer;

class VolumeLoadButton : public QToolButton {
    Q_OBJECT
public:
    VolumeLoadButton(VolumeContainer* vc, QWidget* parent = 0);
    void setVolumeContainer(VolumeContainer* vc);

public slots:
    void loadVolume();
    void loadActionTriggered(QAction*);

    /// opens the file dialog  with a preset raw filter. Restores old filter afterwards.
    void loadVolumeRawFilter();

    void loadRawVolume(const std::string& filename);
    void loadRawSlices(std::vector<std::string> sliceFiles);
    void loadDicomFiles();

protected:
    QAction* loadDatAction_;
    QAction* loadRawAction_;

#ifdef VRN_MODULE_DICOM
    QAction* loadDicomAction_;
#endif
    VolumeContainer* volumeContainer_;

    /// Filter string used in the dialog
    QString volumeFilesFilter_;
    QString rawVolumeFilesFilter_;
    QString philipsUSFilesFilter_;
    QString rawSliceFilesFilter_;

    // currently selected filter
    QString selectedFilter_;

#ifdef VRN_MODULE_DICOM
    DicomDirDialog* dicomDirDialog_;
#endif

    static const std::string loggerCat_;

    std::string getExtensions() const;

private slots:
    /// returns a vector of strings with selected filenames
    std::vector<std::string> openFileDialog();
    /// allows adding multiple volumes to the attached volumecontainer
    void addMultipleVolumes(std::vector<std::string>);

    // opens a dicomdir
    void loadDicomDir(const std::string& file);
    /// opens dicom files
    void loadDicomFiles(const std::string& dir);
    void dicomDirDialogFinished();

    /// store the changed file filter
    void filterChanged(QString filter);

signals:
    void VolumeAdded(int);
};

} // namespace

#endif // VRN_VOLUMELOADBUTTON_H
