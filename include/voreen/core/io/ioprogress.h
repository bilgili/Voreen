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

#ifndef VRN_IO_PROGRESS_H
#define VRN_IO_PROGRESS_H

#include <string>

namespace voreen {

/**
 * Base class for GUI toolkit specific progress bars.
 */
class IOProgress {
public:
    IOProgress();
    virtual ~IOProgress() {}

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
     * @param progress Current amount of progress.
     *  Must lie within the range [0, totalSteps]
     *
     * @see setTotalSteps
     */
    virtual void setProgress(int progress);

    /**
     * Returns the current amount of progress
     * within the range [0, totalSteps].
     */
    virtual int getProgress() const;

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

    /**
     * Assigns the total number of progress steps.
     *
     * @param totalSteps number of steps >= 0
     */
    virtual void setTotalSteps(int totalSteps) = 0;

protected:
    int progress_;
    std::string message_;
    std::string title_;
};

} // namespace voreen

#endif // VRN_IO_PROGRESS_H
