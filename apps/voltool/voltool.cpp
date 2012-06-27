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

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#endif

#include "commands_grad.h"
#include "commands_convert.h"
#include "commands_create.h"
#include "commands_modify.h"

#include "voreen/core/utils/cmdparser/commandlineparser.h"

#include "tgt/init.h"
#include "tgt/exception.h"

#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef WIN32
#include <unistd.h>
#endif

using namespace voreen;

int main(int argc, char** argv) {
    std::string loggerCat_ = "voreen.voltool";

#ifdef VRN_NO_OPENGL
    Singleton<LogManager>::init(new LogManager());
//     Singleton<FileSystem>::init(new FileSystem());
#else
    tgt::init();
#endif
    //add a console logger:
    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat("", true, tgt::Debug);
    LogMgr.addLog(clog);

    CommandlineParser cmdparser("Voltool");

    cmdparser.addCommand(new CommandGrad());
    cmdparser.addCommand(new CommandFilterGrad());

#ifdef VRN_WITH_DEVIL
    cmdparser.addCommand(new CommandStackImg());
#endif
    cmdparser.addCommand(new CommandStackRaw());
    cmdparser.addCommand(new CommandConvert());
    cmdparser.addCommand(new CommandConvertFormat());

    cmdparser.addCommand(new CommandCreate());
    cmdparser.addCommand(new CommandGenerateMask());

    cmdparser.addCommand(new CommandCutToPieces());
    cmdparser.addCommand(new CommandScale());
    cmdparser.addCommand(new CommandMirrorZ());
    cmdparser.addCommand(new CommandSubSet());
    cmdparser.addCommand(new CommandBrick());



    //cmdparser.addCommand(new CommandStretchHisto());

#ifdef _OPENMP
    LINFO("OpenMP Supported.");
    LINFO("Number of processors: " << omp_get_num_procs());
    LINFO("Number of threads maximal usable: " << omp_get_max_threads());
#endif

    cmdparser.setCommandLine(argc, argv);
    cmdparser.execute();

    // Display the help if no argument is given
    if (argc == 1)
        cmdparser.displayHelp();

    return EXIT_SUCCESS;
}
