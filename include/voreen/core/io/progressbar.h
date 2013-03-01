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

#ifndef VRN_PROGRESSBAR_H
#define VRN_PROGRESSBAR_H

#include "voreen/core/voreencoreapi.h"
#include <string>

namespace voreen {

/**
 * Base class for GUI toolkit specific progress bars.
 */
class VRN_CORE_API ProgressBar {
public:
    ProgressBar();
    virtual ~ProgressBar() {}

    /**
     * Makes the progress dialog visible.
     */
    virtual void show() = 0;

    /**
     * Makes the progress dialog invisible.
     */
    virtual void hide() = 0;

    /**
     * Calling this function is assumed to force a repaint,
     * rather than just schedule an update.
     */
    virtual void forceUpdate() = 0;

    /// Override this method to get notified on IO progress.
    virtual void update() = 0;

    /**
     * Assigns the current progress state.
     *
     * @param progress The current progress. Must
     * lie in [0, 1]
     */
    virtual void setProgress(float progress);

    /**
     * Returns the current amount of progress
     * within the range [0, 1].
     */
    virtual float getProgress() const;

    /**
     * Sets the 'level' of progress. This is used if a processor
     * has multiple expensive tasks to do in a single process call
     * and wants to inform the user about the status of each.
     * Usage: Use \sa setProgress as normal for the first tier
     * after completion call setProgressTier(1) and start using
     * \sa setProgress again, ranging from 0.f to 1.f as normal.
     */
    virtual void setProgressTier(int /*tier*/) {};

    /**
     * Assigns a message that is to displayed by the
     * progress dialog.
     */
    virtual void setMessage(const std::string& message);

    /**
     * Returns the message that is to displayed by the
     * progress dialog.
     */
    virtual std::string getMessage() const;

    /**
     * Assigns a title that is to displayed by the
     * progress dialog.
     */
    virtual void setTitle(const std::string& title);

    /**
     * Returns the title that is to displayed by the
     * progress dialog.
     */
    virtual std::string getTitle() const;

protected:
    float progress_;
    std::string message_;
    std::string title_;

    bool printedErrorMessage_;

private:
    static std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_PROGRESSBAR_H
