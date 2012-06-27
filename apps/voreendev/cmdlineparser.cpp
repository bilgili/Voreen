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

#include "cmdlineparser.h"

#include <QObject> // for tr

const QString CmdLineParser::usage_(QObject::tr(
        "Usage: voreendev "
        "["
#ifdef VRN_MODULE_DEFORMATION
        "--deform|"
#endif
        "--dynamic|--fancy|--glyph|--label|--lens|"
        "--simple|--slice|--vessel|--dao|--virtclip|--mirror|--petct|--raytracing|--fast|--global] "
        "[-x|--maximized] [--ncg] "
        "[--dbg level=(d(ebug)?|w(arning)?|e(rror)?|f(atal)?] [--dbg-cat category] "
        "[<-n|--network> <filename>] [-w <width>] [-h <height>] [--benchmark]"
        "[<-s|--segmentation> <filename>] [<-tf> <filename>] [filename]"
    ));

namespace {

void RendererTypeFound(bool& rendererTypeFound,
                       std::string& rendererTypeDst,
                       std::string rendererTypeSrc = "")
{
    if (rendererTypeFound) {
        std::cerr << "only one renderer type can be given" << std::endl;
        exit(1);        
    }
    else if (rendererTypeSrc.empty()) {
        std::cerr << "no network file known for this renderer (add in CmdLineParser ctor)" << std::endl;
        exit(1);
    }
    else {
        rendererTypeFound = true;
        rendererTypeDst = rendererTypeSrc;
    }
}

} // namespace

CmdLineParser::CmdLineParser(QStringList* args)
  : args_(*args),
#ifndef VRN_SNAPSHOT
    networkName_("../../data/networks/standard.vnw"),
#else
    networkName_("../../data/networks/simple.vnw"),
#endif
    maximized_(false),
    cat_(""), // default: show messages from all catagories
    dbgLevel_(tgt::Info),
    canvasWidth_(-1),
    canvasHeight_(-1),
    benchmark_(false)
{
    bool fileNameFound = false;
    bool rendererTypeFound = false;

    for (int i = 1; i < args_.count(); ++i) {
        if (args_[i] == "-x" || args_[i] == "--maximized")
            maximized_ = true;

        else if (args_[i] == "--dbg") {
            if (i+1 >= args_.count()) {
                std::cerr << "debug level missing, setting to level Info" << dbgLevel_ << std::endl;
                return;
            }

            QString dbgl = args_[++i];

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
                std::cerr << "invalid debug level: " << args_[i].toStdString() << std::endl;
        }

        else if (args_[i] == "--dbg_cat") {
            // the order in the next compound if-statement is important
            if ( (i+1 == args_.count()) || (args_[i+1].startsWith("--")) )  {
                // catch corner case where last argument has got to be a filename
                // or dbg category is missing and thus leave default dbg category
            } else {
                cat_ = args_[++i];
            }
        }
#ifdef VRN_MODULE_DEFORMATION
        else if (args_[i] == "--deform")
            RendererTypeFound(rendererTypeFound, networkName_, "../../src/modules/deformation/deform.vnw");
#endif
        else if (args_[i] == "--dynamic")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--fancy")
            RendererTypeFound(rendererTypeFound, networkName_, "../../data/networks/standard.vnw");
        else if (args_[i] == "--closeups")
            RendererTypeFound(rendererTypeFound, networkName_, "../../data/networks/overviewcloseup.vnw");
        else if (args_[i] == "--raytracing")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--global")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--dao")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--glyph")
            RendererTypeFound(rendererTypeFound, networkName_, "../../data/networks/glyph.vnw");
        else if (args_[i] == "--label")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--simple")
            RendererTypeFound(rendererTypeFound, networkName_, "../../data/networks/simple.vnw");
        else if (args_[i] == "--fast")
            RendererTypeFound(rendererTypeFound, networkName_, "../../data/networks/slice.vnw");
        else if (args_[i] == "--slice")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--virtclip")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--speedlines")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--vectorfieldvisualization0")
            RendererTypeFound(rendererTypeFound, networkName_);
		else if (args_[i] == "--vectorfieldvisualization1")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--mirror")
			RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--petct")
            RendererTypeFound(rendererTypeFound, networkName_);
         else if (args_[i] == "--petctclip")
             RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--picking")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--overview")
            RendererTypeFound(rendererTypeFound, networkName_);
        else if (args_[i] == "--help") {
            std::cout << usage_.toStdString() << std::endl;
            exit(0);
        }
        else if (args_[i] == "-w") {
            i++;
            if (i < args_.count()) {
                bool ok;
                int number = args_[i].toInt(&ok);
                if (ok) {
                    canvasWidth_ = number;
                } else {
                    std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an integer argument." << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an argument." << std::endl;
                exit(1);
            }           
        }
        else if (args_[i] == "-h") {
            i++;
            if (i < args_.count()) {
                bool ok;
                int number = args_[i].toInt(&ok);
                if (ok) {
                    canvasHeight_ = number;
                } else {
                    std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an integer argument." << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an argument." << std::endl;
                exit(1);
            }           
        }
        else if (args_[i] == "--benchmark") {
            benchmark_ = true;
        }
        else if (args_[i] == "--segmentation" || args_[i] == "-s") {
            i++;
            if (i < args_.count()) {
                segmentationFileName_ = args_[i];
            } else {
                std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an argument." << std::endl;
                exit(1);
            }
        }
        else if (args_[i] == "--network" || args_[i] == "-n") {
            i++;
            if (i < args_.count()) {
                networkName_ = args_[i].toStdString();
            } else {
                std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an argument." << std::endl;
                exit(1);
            }
        }
        else if (args_[i] == "-tf") {
            i++;
            if (i < args_.count()) {
                tfFileName_ = args_[i];
            } else {
                std::cerr << "parameter "  << args_[i-1].toStdString() << " expects an argument." << std::endl;
                exit(1);
            }
        }
        else {
            if (args_[i][0] == '-') {
                std::cerr << "unkown option: " << args_[i].toStdString() << std::endl;
                exit(1);
            }

            if (fileNameFound) {
                std::cerr << "only one file name can be given" << std::endl;
                continue;
            }

            fileName_ = args_[i];
            fileNameFound = true;
        }
    }
}
