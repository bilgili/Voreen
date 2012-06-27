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
#endif

#include "voreen/core/voreenapplication.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/modules/moduleregistration.h"

#include "tgt/init.h"
#include "tgt/shadermanager.h"
#include "tgt/logmanager.h"
#include "tgt/glut/glutcanvas.h"

#include <iostream>
#include <fstream>
#include <ctime>

using namespace voreen;

class StringLog : public tgt::Log {
protected:
    std::stringstream str_;
public:
    StringLog(bool dateStamping = true, bool timeStamping= true, bool showCat = true, bool showLevel = true) {
        timeStamping_ = timeStamping;
        dateStamping_ = dateStamping;
        showCat_ = showCat;
        showLevel_ = showLevel;
    }
    virtual ~StringLog() {}
    bool isOpen() { return true; }

    std::string getContent() { return str_.str(); }
    void reset() { str_.clear(); str_.str(""); }
protected:
    virtual void logFiltered(const std::string &cat, tgt::LogLevel level, const std::string &msg, const std::string& /*extendedInfo*/) {
        if (dateStamping_)
            str_ << "[" << getDateString() << "] ";
        if (timeStamping_)
            str_ << "[" << getTimeString() << "] ";
        if (showCat_)
            str_ << cat << " ";
        if (showLevel_)
            str_ << "(" << getLevelString(level) << ") ";
        str_ << msg << '\n';
    }
};

//---------------------------------------------------------------------------

tgt::GLUTCanvas* canvas_ = 0;

//---------------------------------------------------------------------------

int main(int argc, char** argv) {
    VoreenApplication app("simple-memcheckinit", "simple-memcheckinit", argc, argv,
                          (VoreenApplication::ApplicationType) (VoreenApplication::APP_DEFAULT & ~VoreenApplication::APP_PYTHON));
    app.init();
    LogMgr.getConsoleLog()->enableColors(false);

    addAllModules(&app);

    glutInit(&argc, argv);

    // initialize canvas
    canvas_ = new tgt::GLUTCanvas("Voreen - The Volume Rendering Engine", tgt::ivec2(512, 512), tgt::GLCanvas::RGBADD);
    canvas_->init();

    app.initGL();

    // add shader path to the shader manager
    ShdrMgr.addPath(VoreenApplication::app()->getShaderPath());
    ProcessorFactory* pf = ProcessorFactory::getInstance();
    // initialize the network evaluator
    NetworkEvaluator* networkEvaluator = new NetworkEvaluator();
    ProcessorNetwork* network = new ProcessorNetwork();
    networkEvaluator->setProcessorNetwork(network);

    std::stringstream tmpOut;

    clock_t start,finish;
    double time;
    double totalTime = 0.0;

    size_t successful = 0;
    size_t skipped = 0;
    size_t numFailed = 0;
    std::vector<std::string> failed;
    const ProcessorFactory::KnownClassesVector& knownClasses =  pf->getKnownClasses();

    StringLog* log = new StringLog(false, false, true, true);
    log->addCat("", true);
    LogMgr.addLog(log);
    for(size_t i=0; i<knownClasses.size(); ++i) {
        if (pf->getProcessorCodeState(knownClasses[i].second) == Processor::CODE_STATE_BROKEN) {
            tmpOut << "\t<testcase classname=\"Init." << knownClasses[i].first << "\" time=\"" << 0.0f
                   << "\" group=\"" << knownClasses[i].first << "\" name=\"" << knownClasses[i].second << "\">\n";
            tmpOut << "\t\t<system-out>\n";
            tmpOut << "skipped (Codestate: Broken)";
            tmpOut << "\t\t</system-out>\n";
            LWARNINGC("simple-memcheckinit", "Skipping class " << knownClasses[i].second);
            tmpOut << "\t</testcase>\n";
            ++skipped;
            continue;
        }

        if (pf->getProcessorCodeState(knownClasses[i].second) == Processor::CODE_STATE_EXPERIMENTAL) {
            tmpOut << "\t<testcase classname=\"Init." << knownClasses[i].first << "\" time=\"" << 0.0f
                   << "\" group=\"" << knownClasses[i].first << "\" name=\"" << knownClasses[i].second << "\">\n";
            tmpOut << "\t\t<system-out>\n";
            tmpOut << "skipped (Codestate: Experimental)";
            tmpOut << "\t\t</system-out>\n";
            LWARNINGC("simple-memcheckinit", "Skipping class " << knownClasses[i].second);
            tmpOut << "\t</testcase>\n";
            ++skipped;
            continue;
        }

        LINFOC("simple-memcheckinit", "Creating class " << knownClasses[i].second << " [" << (i+1) << "/" << knownClasses.size() << "]");
        Processor* p = pf->create(knownClasses[i].second);
        network->addProcessor(p);
        LINFOC("simple-memcheckinit", "Initializing class " << knownClasses[i].second);

        log->reset();
        start = clock();
        bool result = networkEvaluator->initializeNetwork();
        finish = clock();
        time = (double(finish)-double(start))/CLOCKS_PER_SEC;
        totalTime += time;
        tmpOut << "\t<testcase classname=\"Init." << knownClasses[i].first << "\" time=\"" << time << "\" group=\""
               << knownClasses[i].first << "\" name=\"" << knownClasses[i].second << "\">\n";
        tmpOut << "\t\t<system-out>\n";
        tmpOut << log->getContent();
        tmpOut << "\t\t</system-out>\n";
        if (result) {
            //tmpOut << "/>\n";
            ++successful;
        }
        else {
            ++numFailed;
            //tmpOut << ">\n";
            tmpOut << "\t\t<failure type=\"xyz\" message=\"Failed to initialize!\"/>\n";
            //tmpOut << "\t\t\ttest123\n";
            //tmpOut << "\t\t</failure>\n";
            failed.push_back(knownClasses[i].second);
            LERRORC("simple-memcheckinit", "Failed to initialize class " << knownClasses[i].second);
        }
        tmpOut << "\t</testcase>\n";

        LINFOC("simple-memcheckinit", "Deleting class " << knownClasses[i].second);
        network->removeProcessor(p);
    }
    if (numFailed == 0)
        LINFOC("simple-memcheckinit", "All class initialized successfully. (" << skipped << " skipped)");
    else {
        LERRORC("simple-memcheckinit", successful << " of " << knownClasses.size() << " classes initialized successfully. ("
                << skipped << " skipped)");
        LERRORC("simple-memcheckinit", "Failed init:");
        for(size_t i=0; i<failed.size(); ++i)
            LERRORC("simple-memcheckinit", failed[i]);
    }

    std::ofstream xmlLog;
    xmlLog.open ("memcheck.xml");
    xmlLog << "<testsuite name=\"valgrind-memcheckinit\" failures=\"" << numFailed << "\" time=\"" << totalTime << "\" errors=\""
           << numFailed << "\" tests=\"" << knownClasses.size() << "\" skipped=\"" << skipped << "\">\n";
    xmlLog << tmpOut.str();
    xmlLog << "</testsuite>\n";
    xmlLog.close();
    networkEvaluator->setProcessorNetwork(0, true);
    ProcessorFactory::destroy();
    delete networkEvaluator;
    delete network;
    delete canvas_;

    tgt::deinitGL();
    tgt::deinit();
    if (numFailed == 0)
        return 0;
    else
        return EXIT_FAILURE;
}
