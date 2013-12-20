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

#include "voreen/core/io/progressbar.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/vector.h"

namespace voreen {

std::string ProgressBar::loggerCat_ = "voreen.ProgressBar";

ProgressBar::ProgressBar()
    : progress_(0)
    , progressRange_(0.f, 1.f)
    , printedErrorMessage_(false)
{}

void ProgressBar::setProgress(float progress) {
    tgtAssert(progressRange_.x <= progressRange_.y && progressRange_.x >= 0.f && progressRange_.y <= 1.f, "invalid progress range");

    if (progress < 0.f) {
        if (!printedErrorMessage_)
            LWARNING("progress value " << progress << " out of valid range [0,1]");
        printedErrorMessage_ = true;
        progress = 0.f;
    }
    else if (progress > 1.f) {
        if (!printedErrorMessage_)
            LWARNING("progress value " << progress << " out of valid range [0,1]");
        printedErrorMessage_ = true;
        progress = 1.f;
    }
    else {
        printedErrorMessage_ = false;
    }

    progress_ = progressRange_.x + progress*(progressRange_.y-progressRange_.x);

    update();
}

float ProgressBar::getProgress() const {
    return progress_;
}

void ProgressBar::setProgressMessage(const std::string& message) {
    message_ = message;
    update();
}

std::string ProgressBar::getProgressMessage() const {
    return message_;
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

void ProgressBar::setProgressRange(const tgt::vec2& progressRange) {
    tgtAssert(progressRange.x <= progressRange.y && progressRange.x >= 0.f && progressRange.y <= 1.f, "invalid progress range");
    progressRange_ = progressRange;
}

tgt::vec2 ProgressBar::getProgressRange() const {
    return progressRange_;
}

} // namespace voreen
