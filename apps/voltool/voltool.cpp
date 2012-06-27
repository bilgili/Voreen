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

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#endif

#include "command.h"
#include "commands_grad.h"
#include "commands_convert.h"
#include "commands_create.h"
#include "commands_modify.h"
#ifndef VRN_SNAPSHOT
#include "commands_motion.h"
#include "commands_dao.h"
// #include "commands_histo.h"
#endif

#include "tgt/init.h"
#include "tgt/exception.h"

#ifdef VRN_WITH_MATLAB
#include "voreen/core/io/matvolumereader.h"
#endif

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
#else // VRN_NO_OPENGL
    tgt::init();
#endif // VRN_NO_OPENGL
    //add a console logger:
    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat("", true, tgt::Debug);
    LogMgr.addLog(clog);

    CommandMap commandMap_;
    
    //HOWTO: add new command XY:
    //1. add class CommandXY implementing Command::execute and a contructor that sets name_, help_,...
    //2. add CommandXY to map
    
    commandMap_.addCommand(new CommandGrad());
    commandMap_.addCommand(new CommandFilterGrad());
    
#ifdef VRN_WITH_DEVIL
    commandMap_.addCommand(new CommandStackImg());
    commandMap_.addCommand(new CommandStackRaw());
    commandMap_.addCommand(new CommandConvert());
#endif
    
    commandMap_.addCommand(new CommandCreate());
    commandMap_.addCommand(new CommandGenerateMask());
    
    commandMap_.addCommand(new CommandCutToPieces());
    commandMap_.addCommand(new CommandScale());
    commandMap_.addCommand(new CommandMirrorZ());
    commandMap_.addCommand(new CommandSubSet());
    
    
#ifndef VRN_SNAPSHOT
    commandMap_.addCommand(new CommandCreateMotion());

	commandMap_.addCommand(new CommandDao32216());
    commandMap_.addCommand(new CommandDaoSphere());
    commandMap_.addCommand(new CommandRegionDaoSphere());
    commandMap_.addCommand(new CommandVQ());
    commandMap_.addCommand(new CommandVQTrain());
    commandMap_.addCommand(new CommandVQPack());
    commandMap_.addCommand(new CommandVQUnpack());
    commandMap_.addCommand(new CommandVQMeasure());
    commandMap_.addCommand(new CommandStaticGlowValue());
    commandMap_.addCommand(new CommandPlotCB());
    commandMap_.addCommand(new CommandPlotHG());
    commandMap_.addCommand(new CommandHistogram());
#endif

// 	commandMap_.addCommand(new CommandStretchHisto());

#ifdef _OPENMP
    LINFO("OpenMP Supported.");
    LINFO("Number of processors: " << omp_get_num_procs());
    LINFO("Number of threads maximal usable: " << omp_get_max_threads());
#endif
    
    //No parameters given:
    if (argc < 2) {
        LINFO("Usage: voltool OPERATION [...]");
        LINFO("Valid operations:");
        commandMap_.listCommands();
        LINFO("Use >voltool OPERATION< to get further informtion about commands");
        return EXIT_SUCCESS;
    }
    
    std::string operation = argv[1];
    
    //only operation with no parameters is given, display help:
    if (argc < 3) {
        if (commandMap_.available(operation)) {
            LINFO("Help for " << operation << ":");
            LINFO(commandMap_.get(operation)->getInfo());
            LINFO("Syntax: " << commandMap_.get(operation)->getSyntax());
            LINFO(commandMap_.get(operation)->getHelp());
        }
        else {
            LERROR("No command " << operation);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    //execute operation:
    if (commandMap_.available(operation)) {
        std::vector<std::string> parameters;
        for(int i=2; i<argc; i++)
            parameters.push_back(argv[i]);

        try {
            commandMap_.get(operation)->execute(parameters);
        }
        catch (SyntaxException &ex) {
            LERROR(ex.getMessage());
            LERROR("Syntax: " << commandMap_.get(operation)->getSyntax());
            return EXIT_FAILURE;
        }
        catch (tgt::FileException &ex) {
            LERROR(ex.what());
            return EXIT_FAILURE;
        }
        catch (std::bad_alloc) {
            LERROR("Bad alloc!");
            return EXIT_FAILURE;
        }
    }
    else {
        LERROR("No command " << operation);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
