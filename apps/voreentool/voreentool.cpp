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

#ifdef VRN_MODULE_PYTHON
// Must come first!
#include "modules/python/pythonmodule.h"
#endif

#include "networkconfigurator.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/utils/commandlineparser.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/workspace.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/utils/stringutils.h"

// core module is always available
#include "modules/core/processors/output/canvasrenderer.h"
#include "modules/core/processors/output/imagesequencesave.h"
#include "modules/core/processors/output/geometrysave.h"
#include "modules/core/processors/output/textsave.h"
#include "modules/core/processors/output/volumesave.h"
#include "modules/core/processors/output/volumecollectionsave.h"

#include "tgt/init.h"
#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/qt/qtcanvas.h"

#include <QApplication>

using namespace voreen;

const std::string APP_BINARY  = "voreentool";
const std::string APP_NAME    = "VoreenTool";
const std::string APP_DESC    = "Command-line interface for the manipulation and execution of Voreen networks.";

const std::string loggerCat_  = "voreentool.main";

void exitFailure(const std::string& errorMsg);

#ifdef VRN_MODULE_PYTHON
void runScript(const std::string& filename) throw (VoreenException);
#endif

NetworkEvaluator* networkEvaluator_ = 0;
Workspace* workspace_ = 0;

// Qt application and created canvases (only in OpenGL mode)
QApplication* qtApp_ = 0;
std::vector<tgt::QtCanvas*> canvases_;

int main(int argc, char* argv[]) {

    // create Voreen application
    VoreenApplication vrnApp(APP_BINARY, APP_NAME, APP_DESC, argc, argv,
        VoreenApplication::ApplicationFeatures(VoreenApplication::APP_ALL &~ VoreenApplication::APP_WIDGETS));

    // prepare command line parser
    CommandLineParser* cmdParser = vrnApp.getCommandLineParser();
    tgtAssert(cmdParser, "no CommandLineParser");
    int cmdStyle = po::command_line_style::default_style ^ po::command_line_style::allow_guessing;
    cmdParser->setCommandLineStyle(static_cast<po::command_line_style::style_t>(cmdStyle));

    std::string workspacePath;
    cmdParser->addOption<std::string>("workspace,w", workspacePath, CommandLineParser::RequiredOption,
        "The workspace to evaluate");

    std::vector<std::string> configurationOptions;
    cmdParser->addMultiOption<std::string>("configuration,c", configurationOptions, CommandLineParser::MainOption,
        "Network configuration as a list of property value assignments in the format: \n"
        "ProcessorName.PropertyName=value");

    std::vector<std::string> actionOptions;
    cmdParser->addMultiOption<std::string>("actions,a", actionOptions, CommandLineParser::MainOption,
        "List of ButtonProperties to be triggered after the network has been evaluated, in the format: \n"
        "ProcessorName.ButtonPropertyName");

    bool glMode = false;
    cmdParser->addFlagOption("opengl", glMode, CommandLineParser::MainOption,
        "Run the workspace in OpenGL mode: initialize OpenGL context and create canvases.");

    bool triggerAllActions = false;
    cmdParser->addFlagOption("trigger-all-actions", triggerAllActions, CommandLineParser::MainOption,
        "Trigger all ButtonProperties after the network has been evaluated.");

    bool triggerImageSaves = false;
    cmdParser->addFlagOption("trigger-imagesaves", triggerImageSaves, CommandLineParser::MainOption,
        "Trigger a \"save file\" event on all Canvases and ImageSequenceSave processors "
        "after the network has been evaluated (requires OpenGL mode).");

    bool triggerVolumeSaves = false;
    cmdParser->addFlagOption("trigger-volumesaves", triggerVolumeSaves, CommandLineParser::MainOption,
        "Trigger a \"save file\" event on all VolumeSave and VolumeCollectionSave processors after the network has been evaluated.");

    bool triggerGeometrySaves = false;
    cmdParser->addFlagOption("trigger-geometrysaves", triggerGeometrySaves, CommandLineParser::MainOption,
        "Trigger a \"save file\" event on all GeometrySave and TextSave processors after the network has been evaluated.");

#ifdef VRN_MODULE_PYTHON
    std::string scriptFilename;
    vrnApp.getCommandLineParser()->addOption("script", scriptFilename, CommandLineParser::MainOption,
        "Run a Python script after the network configuration has been applied.");
#endif

    std::string workingDirectory;
    vrnApp.getCommandLineParser()->addOption("workdir",  workingDirectory, CommandLineParser::MainOption,
        "Absolute work directory of the workspace. If unspecified, the workspace file location is used.");

    // init application
    try {
        vrnApp.initialize();
    }
    catch (VoreenException& e) {
        exitFailure("Failed to initialize application: " + std::string(e.what()));
    }

    // initialize OpenGL context and initializeGL VoreenApplication
    if (glMode) {
        qtApp_ = new QApplication(argc, argv);
        tgt::QtCanvas* initContext = new tgt::QtCanvas("Init Canvas", tgt::ivec2(32, 32), tgt::GLCanvas::RGBADD, 0, true);
        canvases_.push_back(initContext);
        initContext->show();
        try {
            vrnApp.initializeGL();
        }
        catch (VoreenException& e) {
            exitFailure("OpenGL initialization failed: " + std::string(e.what()));
        }
        initContext->hide();
    }

    // load workspace
    workspace_ = new Workspace();
    workspacePath = tgt::FileSystem::absolutePath(workspacePath);
    try {
        LINFO("Loading workspace " << workspacePath);
        if (!workingDirectory.empty()) {
            workingDirectory = tgt::FileSystem::absolutePath(workingDirectory);
            LINFO("Workspace working path: " << workingDirectory);
        }
        workspace_->load(workspacePath, workingDirectory);
    }
    catch (SerializationException& e) {
        exitFailure(e.what());
    }
    // log errors occurred during workspace deserialization
    for (size_t i=0; i<workspace_->getErrors().size(); i++)
        LERROR(workspace_->getErrors().at(i));

    ProcessorNetwork* network = workspace_->getProcessorNetwork();
    tgtAssert(network, "no network");

    // create network evaluator
    networkEvaluator_ = new NetworkEvaluator(glMode, (!canvases_.empty() ? canvases_.front() : 0));
    VoreenApplication::app()->setNetworkEvaluator(networkEvaluator_);

    // create separate canvas for each CanvasRenderer in the network (only in OpenGL mode)
    if (glMode) {
        std::vector<CanvasRenderer*> canvasRenderers = network->getProcessorsByType<CanvasRenderer>();
        if (!canvasRenderers.empty())
            LINFO("Creating canvases ...");
        for (size_t i=0; i<canvasRenderers.size(); i++) {
            CanvasRenderer* canvasRenderer = canvasRenderers.at(i);
            tgt::QtCanvas* canvas;
            if (i == 0) {
                // recycle init canvas for first CanvasRenderer
                canvas = canvases_.front();
                canvas->setWindowTitle(QString::fromStdString(canvasRenderer->getName()));
                canvas->resize(canvasRenderer->getCanvasSize().x, canvasRenderer->getCanvasSize().y);
            }
            else {
                // create new canvases for following CanvasRenderers
                canvas = new tgt::QtCanvas(canvasRenderer->getName(), canvasRenderer->getCanvasSize(), tgt::GLCanvas::RGBADD, 0, true);
                canvases_.push_back(canvas);
            }
            VoreenPainter* painter = new VoreenPainter(canvas, networkEvaluator_, canvasRenderer);
            canvas->setPainter(painter);
            canvas->show();
            canvasRenderer->setCanvas(canvas);
        }
    }

    // initialize network evaluator and assign network to it, which also initializes the processors
    LINFO("Initializing network ...");
    networkEvaluator_->setProcessorNetwork(network);

    // apply network configuration
    NetworkConfigurator configurator(network);
    LINFO("Applying network configuration ...");
    for (size_t i=0; i<configurationOptions.size(); i++) {
        std::string optionStr = configurationOptions.at(i);
        LDEBUG("applying configuraton option: " << optionStr);
        try {
            configurator.setPropertyValue(optionStr);
        }
        catch (VoreenException& e) {
            exitFailure("in config option '" + optionStr + "': " + e.what());
        }
    }

#ifdef VRN_MODULE_PYTHON
    if (!scriptFilename.empty()) {
        try {
            runScript(scriptFilename);
        }
        catch (VoreenException& e) {
            exitFailure(e.what());
        }
    }

#endif

    // evaluate network
    LINFO("Evaluating network ...");
    try {
        networkEvaluator_->process();
    }
    catch (std::exception& e) {
        exitFailure("exception during network evaluation: " + std::string(e.what()));
    }

    // trigger actions
    LINFO("Triggering post-evaluation actions ...");
    if (triggerAllActions) {
        std::vector<ButtonProperty*> buttonProps = network->getPropertiesByType<ButtonProperty>();
        for (size_t i=0; i<buttonProps.size(); i++) {
            LDEBUG("triggering ButtonProperty: " << buttonProps.at(i)->getFullyQualifiedID());
            buttonProps.at(i)->clicked();
        }
    }
    else {
        for (size_t i=0; i<actionOptions.size(); i++) {
            std::string optionStr = actionOptions.at(i);
            LDEBUG("triggering action: " << optionStr);
            try {
                configurator.triggerButtonProperty(optionStr);
            }
            catch (VoreenException& e) {
                exitFailure("in action '" + optionStr + "': " + e.what());
            }
        }
    }

    // take Canvas screenshots and write out imagesequences (only in OpenGL mode)
    if (glMode && triggerImageSaves) {
        std::vector<CanvasRenderer*> canvasRenderers = network->getProcessorsByType<CanvasRenderer>();
        for (size_t i=0; i<canvasRenderers.size(); i++) {
            ButtonProperty* screenshotProp = dynamic_cast<ButtonProperty*>(canvasRenderers.at(i)->getProperty("saveScreenshot"));
            if (!screenshotProp) {
                LERROR("'saveScreenshot' property of CanvasRenderer '" << canvasRenderers.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering CanvasRenderer screenshot: " << canvasRenderers.at(i)->getName());
            screenshotProp->clicked();
        }

        std::vector<ImageSequenceSave*> imageSequenceSaves = network->getProcessorsByType<ImageSequenceSave>();
        for (size_t i=0; i<imageSequenceSaves.size(); i++) {
            ButtonProperty* screenshotProp = dynamic_cast<ButtonProperty*>(imageSequenceSaves.at(i)->getProperty("save"));
            if (!screenshotProp) {
                LERROR("'save' property of ImageSequenceSave '" << imageSequenceSaves.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering 'save' property of ImageSequenceSave: " << imageSequenceSaves.at(i)->getName());
            screenshotProp->clicked();
        }
    }

    // trigger VolumeSave and VolumeCollectionSave processors
    if (triggerVolumeSaves) {
        std::vector<VolumeSave*> volumeSaves = network->getProcessorsByType<VolumeSave>();
        for (size_t i=0; i<volumeSaves.size(); i++) {
            ButtonProperty* saveProp = dynamic_cast<ButtonProperty*>(volumeSaves.at(i)->getProperty("save"));
            if (!saveProp) {
                LERROR("'save' property of VolumeSave '" << volumeSaves.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering 'save' property of VolumeSave: " << volumeSaves.at(i)->getName());
            saveProp->clicked();
        }

        std::vector<VolumeCollectionSave*> volumeCollectionSaves = network->getProcessorsByType<VolumeCollectionSave>();
        for (size_t i=0; i<volumeCollectionSaves.size(); i++) {
            ButtonProperty* saveProp = dynamic_cast<ButtonProperty*>(volumeCollectionSaves.at(i)->getProperty("save"));
            if (!saveProp) {
                LERROR("'save' property of VolumeCollectionSave '" << volumeCollectionSaves.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering 'save' property of VolumeCollectionSave: " << volumeCollectionSaves.at(i)->getName());
            saveProp->clicked();
        }
    }

    // trigger GeometrySave and TextSave processors
    if (triggerGeometrySaves) {
        std::vector<GeometrySave*> geometrySaves = network->getProcessorsByType<GeometrySave>();
        for (size_t i=0; i<geometrySaves.size(); i++) {
            ButtonProperty* saveProp = dynamic_cast<ButtonProperty*>(geometrySaves.at(i)->getProperty("save"));
            if (!saveProp) {
                LERROR("'save' property of GeometrySave '" << geometrySaves.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering 'save' property of GeometrySave: " << geometrySaves.at(i)->getName());
            saveProp->clicked();
        }

        std::vector<TextSave*> textSaves = network->getProcessorsByType<TextSave>();
        for (size_t i=0; i<textSaves.size(); i++) {
            ButtonProperty* saveProp = dynamic_cast<ButtonProperty*>(textSaves.at(i)->getProperty("save"));
            if (!saveProp) {
                LERROR("'save' property of TextSave '" << textSaves.at(i)->getName() << "' not found");
                continue;
            }
            LDEBUG("triggering 'save' property of TextSave: " << textSaves.at(i)->getName());
            saveProp->clicked();
        }
    }

    // clean up
    LINFO("Deinitializing network ...");
    networkEvaluator_->deinitializeNetwork();
    delete networkEvaluator_;
    delete workspace_;

    if (glMode) {
        LDEBUG("Deinitializing OpenGL ...");
        vrnApp.deinitializeGL();
        for (size_t i=0; i<canvases_.size(); i++)
            delete canvases_.at(i);
        canvases_.clear();
        delete qtApp_;
        qtApp_ = 0;
    }

    vrnApp.deinitialize();
    return 0;
}

void exitFailure(const std::string& errorMsg) {
    LFATAL(errorMsg);

    if (networkEvaluator_)
        networkEvaluator_->deinitializeNetwork();
    delete networkEvaluator_;
    delete workspace_;

    if (VoreenApplication::app()->isInitializedGL()) {
        LDEBUG("Deinitializing OpenGL ...");
        VoreenApplication::app()->deinitializeGL();
        for (size_t i=0; i<canvases_.size(); i++)
            delete canvases_.at(i);
        canvases_.clear();
        delete qtApp_;
        qtApp_ = 0;
    }

    VoreenApplication::app()->deinitialize();

    std::cerr << std::endl << "FAILURE: " << errorMsg << std::endl;
    exit(EXIT_FAILURE);
}

#ifdef VRN_MODULE_PYTHON
void runScript(const std::string& filename) throw (VoreenException) {
    if (!PythonModule::getInstance())
        throw VoreenException("Failed to run Python script: PythonModule not instantiated");

    LINFO("Running Python script '" << filename << "' ...");
    PythonModule::getInstance()->runScript(filename, false);  //< throws VoreenException on failure
    LINFO("Python script finished.");
}
#endif

