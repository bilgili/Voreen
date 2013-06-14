/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_VOREENVISUALIZATION_H
#define VRN_VOREENVISUALIZATION_H

#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkevaluator.h"
#include <QtGui>

namespace tgt {
    class GLCanvas;
}

namespace voreen {

class Workspace;
class NetworkEvaluator;
class PropertyListWidget;
class NetworkEditor;
class ProcessorNetwork;
class ProcessorListWidget;
class RenderTargetViewer;
class InputMappingDialog;
class VolumeViewer;

class VoreenVisualization : public QObject, public ProcessorNetworkObserver, public NetworkEvaluatorObserver {
    Q_OBJECT
public:
    VoreenVisualization(tgt::GLCanvas* sharedContext);
    ~VoreenVisualization();

    void createConnections();

    void newWorkspace();
    void openWorkspace(const QString& filename, const QString& workDir = "")
        throw (SerializationException);
    void saveWorkspace(const QString& filename, bool overwrite = true, const QString& workDir = "")
        throw (SerializationException);

    void importNetwork(const QString& filename) throw (SerializationException);
    void exportNetwork(const QString& filename) throw (SerializationException);


    NetworkEvaluator* getEvaluator() const;
    Workspace* getWorkspace() const;

    std::vector<std::string> getNetworkErrors();
    std::vector<std::string> getWorkspaceErrors();

    // Implementation of the ProcessorNetworkObserver interface
    void networkChanged();
    void processorAdded(const Processor* processor);
    void processorRemoved(const Processor* processor);

    void setNetworkEditorWidget(NetworkEditor* networkEditorWidget);
    void setPropertyListWidget(PropertyListWidget* propertyListWidget);
    void setProcessorListWidget(ProcessorListWidget* processorListWidget);
    void setRenderTargetViewer(RenderTargetViewer* renderTargetViewer);
    void setVolumeViewer(VolumeViewer* volumeViewer);
    void setInputMappingDialog(InputMappingDialog* inputMappingDialog);

    bool readOnlyWorkspace() const { return readOnlyWorkspace_; }

    bool isModified() const;

public slots:
    void setModified(bool isModified = true);
    bool rebuildShaders();

signals:
    void newNetwork(ProcessorNetwork* network);
    void networkModified(ProcessorNetwork* network);
    void modified();

private:
    /**
     * Called by the NetworkEvaluator (via observation).
     * Propagates the new network to the GUI components.
     */
    virtual void networkAssigned(ProcessorNetwork* newNetwork, ProcessorNetwork* previousNetwork);

    void propagateWorkspaceDescription(const std::string& description);

    /**
     * Deletes all tmpFiles located in tmpPath.
     */
    void cleanupTempFiles(std::vector<std::string> tmpFiles, std::string tmpPath);

    NetworkEvaluator* evaluator_;
    Workspace* workspace_;
    tgt::GLCanvas* sharedContext_;

    NetworkEditor* networkEditorWidget_;
    PropertyListWidget* propertyListWidget_;
    ProcessorListWidget* processorListWidget_;
    RenderTargetViewer* renderTargetViewer_;
    VolumeViewer* volumeViewer_;
    InputMappingDialog* inputMappingDialog_;

    bool readOnlyWorkspace_;
    bool modified_;
    std::vector<std::string> workspaceErrors_;

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_VOREENVISUALIZATION_H
