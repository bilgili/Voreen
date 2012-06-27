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

class CmdLineParser {
public:
    CmdLineParser(QStringList* args);

    QStringList getArgs() const { return args_; }

    QString getFileName() const { return fileName_; }
    QString getSegmentationFileName() const { return segmentationFileName_; }
    QString getTransFuncFileName() const { return tfFileName_; }

    std::string getNetworkName() {
        return networkName_;
    }

    void setNetworkName(std::string networkName) {
        networkName_ = networkName;
    }

    bool getNoCalcGradients() const { return noCalcGradients_; }

    tgt::LogLevel getDebugLevel() const { return dbgLevel_; }

    QString getCategory() const { return cat_; }

    bool getMaximized() const { return maximized_; }

    static const QString usage_;

private:
    void printUsage();

    QStringList args_;
    std::string networkName_;
    bool        noCalcGradients_;
    bool        maximized_;
    QString     fileName_;
    QString     segmentationFileName_;
    QString     tfFileName_;
    QString     cat_;
    tgt::LogLevel dbgLevel_;
};

#endif // VRN_CMDLINEPARSER
