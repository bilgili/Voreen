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

#include "voreen/qt/ioprogressdialog.h"

namespace voreen {

IOProgressDialog::IOProgressDialog(QWidget* parent)
    : progressDialog_(new QProgressDialog(parent))
{
    progressDialog_->setCancelButton(0);
    progressDialog_->setWindowModality(Qt::WindowModal);

    time_ = new QTime();
    time_->start();
}

IOProgressDialog::~IOProgressDialog() {
    delete progressDialog_;
    delete time_;
}

void IOProgressDialog::update() {
    // time to wait between progress bar updates
    const int MINIMAL_UPDATE_WAIT = 50;
    
    if ((time_->elapsed() > MINIMAL_UPDATE_WAIT || progress_ == progressDialog_->maximum())
        && progressDialog_->value() != progress_)
    {
        progressDialog_->setValue(progress_);
        time_->restart();
    }
}

void IOProgressDialog::setNumSteps(int numSteps) {
    progressDialog_->setRange(0, numSteps - 1);
    time_->restart();
}

void IOProgressDialog::show(const std::string& filename) {
    set(0);
    QString title(QObject::tr("Loading file"));
    if (!filename.empty())
        title += '"' +  QString(filename.c_str()) + '"' + "...";
    progressDialog_->setLabelText(title);
    progressDialog_->show();
    progressDialog_->raise();
    progressDialog_->activateWindow();
}

void IOProgressDialog::hide() {
    set(progressDialog_->maximum());
    progressDialog_->hide();
}

} // namespace
