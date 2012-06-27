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

#ifndef VRN_CMDLINEPARSER
#define VRN_CMDLINEPARSER

#include <QString>
#include <QStringList>

#include "tgt/logmanager.h"

namespace voreen {

// A simplistic command line parser.
// Note: voreendev and other apps have their own version of cmdlineparser.(cpp|h)
class CmdLineParser {
public:
    CmdLineParser(QStringList args);

    const QStringList& getFileNames() const { return fileNames_; }

    std::string getNetworkName() const {
        return networkName_;
    }

    void setNetworkName(std::string networkName) {
        networkName_ = networkName;
    }

    tgt::LogLevel getDebugLevel() const { return dbgLevel_; }

    QString getCategory() const { return cat_; }

    bool getMaximized() const { return maximized_; }

    static const QString usage_;

private:
    void printUsage();

    std::string networkName_;
    bool              maximized_;
    QStringList       fileNames_;
    QString           cat_;
    tgt::LogLevel     dbgLevel_;
};

}

#endif // VRN_CMDLINEPARSER
