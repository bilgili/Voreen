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

#ifndef VRN_VOREENVEAPPLICATION_H
#define VRN_VOREENVEAPPLICATION_H

#include "voreen/qt/voreenapplicationqt.h"
#include <QApplication>

namespace voreen {

class VoreenModuleVE;

/**
 * Common subclass of QApplication and VoreenApplicationQt.
 * Extends the CommandLineParser by VoreenVE-specific commands
 * and handles the VoreenVEModules. It also acts as exception catcher
 * "of last resort".
 */
class VoreenVEApplication : public QApplication, public VoreenApplicationQt {
    Q_OBJECT
public:
    VoreenVEApplication(int& argc, char** argv);
    ~VoreenVEApplication();

    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    virtual void initializeGL() throw (VoreenException);
    virtual void deinitializeGL() throw (VoreenException);

    static VoreenVEApplication* veApp();

    /// Overridden for catching all uncaught exceptions reaching the application level.
    virtual bool notify(QObject* receiver, QEvent* event);

    /// Registers a VoreenVE module.
    void registerVEModule(VoreenModuleVE* module);

    /// Registers a VoreenVE module.
    const std::vector<VoreenModuleVE*>& getVEModules() const;

protected:
    /// Adds VE modules.
    virtual void loadModules() throw (VoreenException);

private:
    static VoreenVEApplication* veApp_;
    std::vector<VoreenModuleVE*> veModules_;

    static const std::string loggerCat_;
};

} // namespace

#endif
