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

#ifndef VRN_APPLICATIONQT_H
#define VRN_APPLICATIONQT_H

#include "voreen/core/voreenapplication.h"

#include "voreen/qt/ioprogressdialog.h"

#include <QString>

class QMainWindow;

namespace voreen {

class VoreenApplicationQt : public VoreenApplication {
public:
    VoreenApplicationQt(const std::string& name, const std::string& displayName,
                        int argc, char** argv, ApplicationType appType = APP_DEFAULT);
    virtual void init();

    virtual void initGL();

    /**
     * Allows access to the global instance of this class.
     */
    static VoreenApplicationQt* qtApp();

    void setMainWindow(QMainWindow* mainWindow);

    QMainWindow* getMainWindow() const;

    /**
     * Creates a tgt::QtTimer.
     *
     * @param handler The event handler that will be used
     *  for broadcasting the timer events. Must not be null.
     */
    virtual tgt::Timer* createTimer(tgt::EventHandler* handler) const;

    /**
     * Creates a IOProgressDialog.
     */
    virtual IOProgressDialog* createProgressDialog() const;

    /**
     * Constructs an absolute path consisting of the VoreenQt shader source directory
     * and the given filename.
     */
    virtual std::string getShaderPathQt(const std::string& filename = "") const;

private:
    static VoreenApplicationQt* qtApp_;
    QMainWindow* mainWindow_;
    std::string shaderPathQt_;
};

} // namespace

#endif //VRN_APPLICATIONQT_H
