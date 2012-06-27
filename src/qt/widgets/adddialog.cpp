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

#include "voreen/qt/widgets/adddialog.h"

#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumemetadata.h"
#include "voreen/core/volume/modality.h"
#include "voreen/core/volume/gradient.h"

#include <QtGui>
#include <QFont>
#include <QPoint>
#include <QLabel>
#include <QFrame>

#include <algorithm>

namespace voreen {

AddDialog::AddDialog(QWidget *parent, VolumeContainer* firstVolcont, VolumeContainer* secondVolcont)
    : QDialog(parent)
    , newVolumeContainer_(0)
    , firstVolumeContainer_(firstVolcont)
    , secondVolumeContainer_(secondVolcont)
    , maxTime_(0.0)
    , secondContainer_(false)
{
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Choose Mapping of Volumes"));
    
    createWidgets();
    createConnections();
    adjustSize();

    QPoint parent_center = parent->geometry().center();
    QPoint dialog_center = geometry().center();
    move(parent_center.x()-dialog_center.x(), parent_center.y()-dialog_center.y());
}

AddDialog::~AddDialog() {
}

void AddDialog::addDialogFinish() {
    if (checkMapping()) {
        newVolumeContainer_ = new VolumeContainer();
        for (size_t i = 0; i < volumeFrames_.size(); ++i) {
            voreen::Volume* volume = volumeFrames_.at(i)->getVolume();
            QString entry = volumeFrames_.at(i)->getModalityString();
            voreen::Modality mod = voreen::Modality(entry.toStdString());
            int time = volumeFrames_.at(i)->getTime();
            newVolumeContainer_->add(volume, volume->meta().getFileName(), mod, float(time));
        }
        done(0);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("The choosen mapping is incorrect.\n"
            "One or more volumes have same timestep and modality\n"
            "or the timesteps start not with zero."));
    }
}

void AddDialog::addDialogCancel() {
    newVolumeContainer_ = firstVolumeContainer_;
    delete secondVolumeContainer_;
    done(1);
}

void AddDialog::createWidgets() {
    setMinimumSize(440, 300);
    setMaximumSize(700, 350);

    QFont boldFont = QFont();
    boldFont.setBold(true);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    // Buttons on the lower right
    QHBoxLayout* hbox_bottom = new QHBoxLayout();
    hbox_bottom->addStretch();
    hbox_bottom->setSpacing(2);
    doneButton_ = new QPushButton(tr("Done"));
    hbox_bottom->addWidget(doneButton_);
    cancelButton_ = new QPushButton(tr("Cancel"));
    hbox_bottom->addWidget(cancelButton_);

    if (secondVolumeContainer_) {

        QGridLayout* grid_upper = new QGridLayout();
        // frames and label on right side (volumes already loaded)
        QLabel* label = new QLabel();
        label->setText(tr("Already loaded Volumes"));
        label->setFont(boldFont);
        grid_upper->addWidget(label, 0, 1, Qt::AlignHCenter);
        QVBoxLayout* vbox_upperright = createVolumeWidget(firstVolumeContainer_);
        vbox_upperright->addStretch();
        QFrame* scrollFrame_right = new QFrame(this);
        scrollFrame_right->setLayout(vbox_upperright);
        QScrollArea* scrollArea_right = new QScrollArea(this);
        scrollArea_right->setWidget(scrollFrame_right);
        scrollArea_right->setFrameStyle(QFrame::NoFrame);
        scrollArea_right->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea_right->setWidgetResizable(true);
        // assign scroll area to right side
        grid_upper->addWidget(scrollArea_right, 1, 1);
    
        // frames and label on left side (added volumes)
        secondContainer_ = true;
        label = new QLabel();
        label->setText(tr("<font color=\"red\">Added Volumes</font>"));
        label->setFont(boldFont);
        grid_upper->addWidget(label, 0, 0, Qt::AlignHCenter);
        //all volumes are put in a single layout
        QVBoxLayout* vbox_upperleft = createVolumeWidget(secondVolumeContainer_);
        vbox_upperleft->addStretch();
        QFrame* scrollFrame_left = new QFrame(this);
        scrollFrame_left->setLayout(vbox_upperleft);
        QScrollArea* scrollArea_left = new QScrollArea(this);
        scrollArea_left->setWidget(scrollFrame_left);
        scrollArea_left->setFrameStyle(QFrame::NoFrame);
        scrollArea_left->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea_left->setWidgetResizable(true);
        // assign scroll area to left side
        grid_upper->addWidget(scrollArea_left, 1, 0);

        mainLayout->addItem(grid_upper);
    }
    else {
        QVBoxLayout* vbox_upper = new QVBoxLayout();
        // frames and label on right side (volumes already loaded)
        QLabel* label = new QLabel();
        label->setText(tr("<font color=\"red\">Loaded Volumes</font>"));
        label->setFont(boldFont);
        vbox_upper->addWidget(label, 0, Qt::AlignHCenter);

        //iterate through all loaded volumes and arrange them rowwise
        VolumeContainer::Entries entries = firstVolumeContainer_->getEntries();
        std::multimap<float, VolumeContainer::Entry>::iterator it = entries.begin();
        QVBoxLayout* vbox_frames = new QVBoxLayout();
        while (it != entries.end()) {
            QHBoxLayout* hbox = new QHBoxLayout();
            hbox->addWidget(createVolumeFrame(it->second.volume_, it->second.modality_, it->first), 0, Qt::AlignLeft);
            hbox->addStretch();
            ++it;
            if (it != entries.end()) {
                hbox->addWidget(createVolumeFrame(it->second.volume_, it->second.modality_, it->first), 0, Qt::AlignRight);
                ++it;
            }
            vbox_frames->addItem(hbox);
            vbox_frames->addStretch();
        }
        //put scrollArea aroud all widgets
        QFrame* scrollFrame = new QFrame(this);
        scrollFrame->setLayout(vbox_frames);
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidget(scrollFrame);
        scrollArea->setFrameStyle(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setWidgetResizable(true);
        vbox_upper->addWidget(scrollArea);
        mainLayout->addItem(vbox_upper);
    }
    //add bottom and top layout to mainlayout

    mainLayout->addItem(hbox_bottom);

    setLayout(mainLayout);
}

QVBoxLayout* AddDialog::createVolumeWidget(VolumeContainer* container) {
    back_ = new QVBoxLayout();
    back_->setSpacing(3);
    VolumeContainer::Entries entries = container->getEntries();
    std::multimap<float, VolumeContainer::Entry>::iterator it;
    //create a frame for every volume in volumeContainer and arrange them in a VBoxLayout
    for (it = entries.begin(); it != entries.end(); ++it) {
        QFrame* volumeFrame = createVolumeFrame((*it).second.volume_, (*it).second.modality_, (*it).first);
        back_->addWidget(volumeFrame);
    }
    back_->addStretch();
    return back_;
}

QFrame* AddDialog::createVolumeFrame(voreen::Volume* vol, voreen::Modality mod, float time) {
    //increment time if volumes of second volumecontainer are processed
    if (secondContainer_) {
        time = maxTime_+1.0;
        maxTime_ += 1.0;
    }
    
    volumeFrame* frame = new volumeFrame(vol, mod, time, this);
    //store all informations in a struct - needed for construction of new volumecontainer
    volumeFrames_.push_back(frame);

    if (!secondContainer_ && time > maxTime_)
        maxTime_ = time;
    
    return frame;
}

void AddDialog::createConnections() {
    connect(doneButton_, SIGNAL(released()), this, SLOT(addDialogFinish()));
    connect(cancelButton_, SIGNAL(released()), this, SLOT(addDialogCancel()));
}

void AddDialog::closeEvent(QCloseEvent *e) {
    //take close of window as cancel
    e->accept();
    addDialogCancel();
}

void AddDialog::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) { // cancel if escape was pressed
        e->accept();
        addDialogCancel();
    }
    else if (e->key() == Qt::Key_Return) { //finish if return was pressed
        e->accept();
        addDialogFinish();
    }
    else
        e->ignore();
}

bool AddDialog::checkMapping() {
    //only timesteps must be checked
    std::vector<int> times_found;
    bool zero_found = false;
    for (size_t i = 0; i < volumeFrames_.size(); ++i) {
        int value = volumeFrames_.at(i)->getTime();
        if (value == 0)
            zero_found = true;
        //check whether an element with same time and same modality was already inserted
        std::string mod_first = volumeFrames_.at(i)->getModalityString().toStdString();
        std::vector<int>::iterator it = std::find(times_found.begin(), times_found.end(), value);
        if (it != times_found.end()) {
            std::string mod_second = volumeFrames_.at(*it)->getModalityString().toStdString();
            if (mod_first == mod_second)
                return false;
            ++it;
        }
        times_found.push_back(value);
    }
    return zero_found;
}


VolumeContainer* AddDialog::getVolumeContainer() {
    return newVolumeContainer_;
}

void AddDialog::addDataset(voreen::Volume* vol, voreen::Modality mod, float time) {
    firstVolumeContainer_->add(vol, "", mod, time);
    
    volumeFrame* frame = new volumeFrame(vol, mod, time, this);
    //store all informations in a struct - needed for construction of new volumecontainer
    volumeFrames_.push_back(frame);
//     back_->addWidget(frame);
}


//-----------------------------------------------------------------------------

volumeFrame::volumeFrame(voreen::Volume* vol, voreen::Modality mod, float time, AddDialog* addDialog) : volume_(vol) {
    addDialog_ = addDialog;

    setFrameStyle(QFrame::Sunken | QFrame::Panel);
    QGridLayout* grid = new QGridLayout();
    
    //label with filename to identify the volume
    std::string volumeName = voreen::VolumeMetaData::getFileNameWithoutPath(vol->meta().getFileName());
    
    QLabel* label = new QLabel(tr(volumeName.c_str()));
    grid->addWidget(label, 0, 0, 1, 3);
    
    QFrame* separator = new QFrame();
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addWidget(separator, 1, 0, 1, 3);
    
    //combobox for selecting modality
    modalityCombo_ = new QComboBox();
    std::vector<std::string> modalities = voreen::Modality::getModalities();
    int indexOfActualMod = 0;
    //insert all modality-strings and store the index of modality of actual volume
    for (size_t i = 0; i < modalities.size(); ++i) {
        std::string modString = modalities.at(i);
        if (modString == mod.getName())
            indexOfActualMod = i;
        modalityCombo_->addItem(tr(modString.c_str()));
    }
    modalityCombo_->setCurrentIndex(indexOfActualMod);
    
    grid->addWidget(new QLabel(tr("Modality")), 2, 0, 1, 1);
    grid->addWidget(modalityCombo_, 2, 1, 1, 2);
    
    //spinbox for setting timestep
    grid->addWidget(new QLabel(tr("TimeStep")), 3, 0, 1, 2);
    timeSpinBox_ = new QSpinBox();
    timeSpinBox_->setAlignment(Qt::AlignRight);
    timeSpinBox_->setRange(0, 100);

    timeSpinBox_->setValue((int)time);
    grid->addWidget(timeSpinBox_, 3, 2, 1, 1);
    
    calcGradients_ = new QPushButton(tr("Calculate Gradients"));
    connect(calcGradients_, SIGNAL(released()), this, SLOT(calcGradients()));
    grid->addWidget(calcGradients_, 4, 0, 1, 3);
    if(volume_->getBitsStored() > 16)
        calcGradients_->setEnabled(false);
    
    grid->setColumnStretch(1, 1);
    
    setLayout(grid);
}

void volumeFrame::calcGradients() {
//     Volume* volgrad = voreen::calcGradients<tgt::col3>(volume_);
 Volume* volgrad = calcGradientsSobel<tgt::col3>(volume_, false);
    addDialog_->addDataset(volgrad, Modality::MODALITY_NORMALS, getTime());
}

} //namespace voreen
