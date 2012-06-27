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

#include "../../include/voreen/core/io/iosystem.h"

#include <QProgressDialog>

//------------------------------------------------------------------------------
// IOObserver
//------------------------------------------------------------------------------

IOSystem::IOObserver::IOObserver(IOSystem* system)
  : system_(system)
{}

void IOSystem::IOObserver::setNumSteps(int numSteps) {
    system_->progressDialog_->setRange(0, numSteps - 1);
}

void IOSystem::IOObserver::update() {
    system_->progressDialog_->setValue(progress_);
}

//------------------------------------------------------------------------------
// IOSystem
//------------------------------------------------------------------------------

IOSystem::IOSystem(QWidget* parent)
  : observer_( new IOObserver(this) ),
    progressDialog_( new QProgressDialog(parent) )
{
    progressDialog_->setCancelButton(0);
    progressDialog_->setLabelText(QObject::tr("Loading volume..."));
    progressDialog_->setWindowModality(Qt::WindowModal);
}

IOSystem::~IOSystem() {
    delete observer_;
    delete progressDialog_;
}

voreen::IOProgress* IOSystem::getObserver() {
    return observer_;
}

void IOSystem::show() {
    observer_->set(0);
    progressDialog_->show();
}

void IOSystem::hide() {
    observer_->set( progressDialog_->maximum() );
    progressDialog_->hide();
}
