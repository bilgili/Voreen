/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_PROGRESSDIALOG_H
#define VRN_PROGRESSDIALOG_H

#include <QProgressDialog>
#include <QTime>

#include "voreen/core/io/progressbar.h"

namespace voreen {

/**
 * A class for a small dialog indicating the progress of loading a file by
 * using a QProgressDialog.
 */
class ProgressDialog : public ProgressBar {
public:
    ProgressDialog(QWidget* parent, const std::string& message = "");
    ~ProgressDialog();

    void update();
    virtual void setProgressMessage(const std::string& message);
    void setTitle(const std::string& title);
    void show();
    void hide();
    void forceUpdate();

protected:
    QProgressDialog* progressDialog_;
    QTime* time_;
};

} // namespace

#endif // VRN_PROGRESSDIALOG_H
