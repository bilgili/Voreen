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

#ifndef VRN_PROGRESSREPORTER_H
#define VRN_PROGRESSREPORTER_H

#include "voreen/core/voreencoreapi.h"
#include "tgt/vector.h"
#include <string>

namespace voreen {

/**
 * Interface for classes that report a task progress to the user.
 */
class VRN_CORE_API ProgressReporter {

public:
    virtual ~ProgressReporter() {}

    /**
     * Assigns the current progress state.
     *
     * @param progress The current progress. Must
     * lie in [0, 1]
     */
    virtual void setProgress(float progress) = 0;

    /// Returns the current progress value.
    virtual float getProgress() const = 0;

    /**
     * Assigns the range into which the progress value will be transformed,
     * i.e., actualProgress = progressRange.x + progress*(progressRange.y-progressRange.x)
     * The progress range must be a subrange of [0.f;1.f].
     * The default range is [0.f;1.f].
     */
    virtual void setProgressRange(const tgt::vec2& progressRange) = 0;

    /// Returns the current progress range.
    virtual tgt::vec2 getProgressRange() const = 0;

    /**
     * Assigns a message that is to be displayed, e.g. in a progress dialog.
     */
    virtual void setProgressMessage(const std::string& message) = 0;

    /// Returns the current progress message;
    virtual std::string getProgressMessage() const = 0;

};

} // namespace voreen

#endif // VRN_PROGRESSREPORTER_H
