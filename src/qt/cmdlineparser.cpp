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

#include "voreen/qt/cmdlineparser.h"

#include <QObject>

namespace voreen {

const QString CmdLineParser::usage_(QObject::tr(
        "Usage: voreenapp [--fancy|--simple|--slice] [-x|--maximized] [--dbg level=(d(ebug)?|"
        "w(arning)?|e(rror)?|f(atal)?] [--dbg-cat category] [filename]"
    ));

void RendererTypeFound(bool& rendererTypeFound,
                       std::string& rendererTypeDst,
                       std::string rendererTypeSrc)
{
    if (rendererTypeFound)
        std::cerr << "only one renderer type can be given" << std::endl;
    else {
        rendererTypeFound = true;
        rendererTypeDst = rendererTypeSrc;
    }
}

CmdLineParser::CmdLineParser(QStringList args)
  : networkName_("standard.vnw"),
    maximized_(false),
    cat_(""), // default: show messages from all catagories
    dbgLevel_(tgt::Info)
{
    bool rendererTypeFound = false;

    for (int i = 1; i < args.count(); ++i) {
        if (args[i] == "-x" || args[i] == "--maximized")
            maximized_ = true;

        else if (args[i] == "--dbg") {
            if (i+1 >= args.count()) {
                std::cerr << "debug level missing, setting to level Info" << dbgLevel_ << std::endl;
                return;
            }

            QString dbgl = args[++i];

            if (dbgl.contains(QRegExp("d(ebug)?", Qt::CaseInsensitive)))
                dbgLevel_ = tgt::Debug;
            else if (dbgl.contains(QRegExp("w(arning)?", Qt::CaseInsensitive)))
                dbgLevel_ = tgt::Warning;
            else if (dbgl.contains(QRegExp("i(nfo)?", Qt::CaseInsensitive)))
                dbgLevel_ = tgt::Info;
            else if (dbgl.contains(QRegExp("e(rror)?", Qt::CaseInsensitive)))
                dbgLevel_ = tgt::Error;
            else if (dbgl.contains(QRegExp("f(atal)?", Qt::CaseInsensitive)))
                dbgLevel_ = tgt::Fatal;
            else
                std::cerr << "invalid debug level: " << args[i].toStdString() << std::endl;
        }

        else if (args[i] == "--dbg_cat") {
            // the order in the next compound if-statement is important
            if ( (i+1 == args.count()) || (args[i+1].startsWith("--")) )  {
                // catch corner case where last argument has got to be a filename
                // or dbg category is missing and thus leave default dbg category
            } else {
                cat_ = args[++i];
            }
        }
        else if (args[i] == "--fancy")
            RendererTypeFound(rendererTypeFound, networkName_, "standard.vnw");
        else if (args[i] == "--simple")
            RendererTypeFound(rendererTypeFound, networkName_, "simple.vnw");
        else if (args[i] == "--slice")
            RendererTypeFound(rendererTypeFound, networkName_, "slice.vnw");
        else if (args[i] == "--help") {
            std::cout << usage_.toStdString() << std::endl;
            exit(0);
        }
        else {
            if (args[i][0] == '-') {
                std::cerr << "unknown option: " << args[i].toStdString() << std::endl;
                exit(1);
            }

            fileNames_.append(args[i]);
        }
    }
}

} // namespace
