/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include "voreen/core/volume/volumecontainer.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QSpinBox>
#include <QDialog>

namespace voreen {

class AddDialog;

/**
* This struct contains informations about a volume. It stores the combobox where the modality is
* choosen and the spinbox where the timestep can be set.
*/
//FIXME: rename to VolumeFrame, make non public unless neded
class volumeFrame : public QFrame {
    Q_OBJECT
    
    voreen::Volume* volume_; //< volume where the combobox and spinbox belongs to
    QComboBox* modalityCombo_; //< combobox where the modality can be set
    QSpinBox* timeSpinBox_; //< spinbox where the timestep can be set
    QPushButton* calcGradients_;
    AddDialog* addDialog_;

    public:
    volumeFrame(voreen::Volume* vol, voreen::Modality mod, float time, AddDialog* addDialog); //< constructor
    voreen::Volume* getVolume() { return volume_; }
    QString getModalityString() { return modalityCombo_->currentText(); }
    int getTime() { return timeSpinBox_->value(); }
    
    public slots:
    void calcGradients();
};


//FIXME: rename to AddVolumeDialog
class AddDialog : public QDialog {
    Q_OBJECT
public:
    /**
     * Constructor - creates a dialog from the volumes in first and second volumeContainer
     */
    AddDialog(QWidget* parent, voreen::VolumeContainer* firstVolcont, voreen::VolumeContainer* secondVolcont);
    
    /**
     * Destructor - does nothing
     */
    ~AddDialog();

    /**
     * This method handles the event when the window is closed
     */
    virtual void closeEvent(QCloseEvent *e);

    /**
     * This method handles the following keys:
     *   - escape
     *   - return
     */
    virtual void keyPressEvent(QKeyEvent *e);

    /**
     * creates the widgets for the dialog
     */
    void createWidgets();

    void createConnections();

    /**
     * Returns the new volumeContainer with the mapping choosen in the dialog
     */
    voreen::VolumeContainer* getVolumeContainer();
    
    ///Add dataset to volume container and gui
    void addDataset(voreen::Volume* vol, voreen::Modality mod, float time);

public slots:
    void addDialogCancel();
    void addDialogFinish();

private:
    
    /**
     * Checks whether choosen mapping is correct or not.
     *
     * @return true if the selected mapping is correct false otherwise
     */
    bool checkMapping();

    /**
     * This method creates a frame with combobox for modality and a spinbox for
     * timestep. Furthermore the fiename is displayed as label.
     *
     * @return frame for given volume, modality and timestep
     */
    QFrame* createVolumeFrame(voreen::Volume* vol, voreen::Modality mod, float time);

    /**
     * Creates a whole layout for all volumes in container
     */
    QVBoxLayout* createVolumeWidget(voreen::VolumeContainer* container);

    voreen::VolumeContainer* newVolumeContainer_; //< volumeContainer that is returned by the dialog
    voreen::VolumeContainer* firstVolumeContainer_; //< volumeContainer containig the already loaded volumes
    voreen::VolumeContainer* secondVolumeContainer_; //< volumeContainer containing the added volumes


    QVBoxLayout* back_;
    QPushButton* doneButton_; //< button for applying current settings
    QPushButton* cancelButton_; //< button for canceling mapping and discard added volumes

    std::vector<volumeFrame*> volumeFrames_; //< vector with all volumeFrames

    float maxTime_; //< maximum timestep foundin already loaded volumes
    bool secondContainer_; //< variable indicating wether the added volumes 
};

} //namespace voreen

#endif
