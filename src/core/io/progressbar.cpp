/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/io/progressbar.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"

namespace voreen {

std::string ProgressBar::loggerCat_ = "voreen.ProgressBar";

ProgressBar::ProgressBar()
    : progress_(0)
    , printedErrorMessage_(false)
{}

void ProgressBar::setProgress(float progress) {
    if (progress < 0.f) {
        if (!printedErrorMessage_)
            LWARNING("progress value " << progress << " out of valid range [0,1]");
        printedErrorMessage_ = true;
        progress = 0.f;
    } else if (progress > 1.f) {
        if (!printedErrorMessage_)
            LWARNING("progress value " << progress << " out of valid range [0,1]");
        printedErrorMessage_ = true;
        progress = 1.f;
    } else {
        printedErrorMessage_ = false;
    }

    progress_ = progress;
    update();
}

float ProgressBar::getProgress() const {
    return progress_;
}

void ProgressBar::setMessage(const std::string& message) {
    message_ = message;
}

std::string ProgressBar::getMessage() const {
    return message_;
}

void ProgressBar::setTitle(const std::string& title) {
    title_ = title;
}

std::string ProgressBar::getTitle() const {
    return title_;
}

} // namespace voreen
