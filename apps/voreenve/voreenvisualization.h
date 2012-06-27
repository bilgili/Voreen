/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_VOREENVISUALIZATION_H
#define VRN_VOREENVISUALIZATION_H

#include "voreen/core/vis/network/processornetwork.h"
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
class VolumeContainer;
class VolumeContainerWidget;
class AnimationPlugin;


class VoreenVisualization : public QObject, public ProcessorNetworkObserver {
    Q_OBJECT
public:
    VoreenVisualization(tgt::GLCanvas* sharedContext);
    ~VoreenVisualization();

    void createConnections();

    void exportWorkspaceAsZip(const std::string& filename, bool overwrite = true)
        throw (SerializationException);
    void importZippedWorkspace(const std::string& archiveName, const std::string& path, 
        bool deflateTemporarily = true) throw (SerializationException);

    void newWorkspace();
    void openWorkspace(const QString& filename)
        throw (SerializationException);
    void saveWorkspace(const std::string& filename, bool reuseTCTargets, bool overwrite = true)
        throw (SerializationException);

    /// @deprecated is to be replacd by importNetwork
    void openNetwork(const std::string& filename) throw (SerializationException);
    /// @deprecated is to be replacd by exportNetwork
    void saveNetwork(const std::string& filename, bool reuseTCTargets) throw (SerializationException);


    NetworkEvaluator* getEvaluator() const;
    Workspace* getWorkspace() const;
    VolumeContainer* getVolumeContainer() const;

    std::vector<std::string> getNetworkErrors();
    std::vector<std::string> getWorkspaceErrors();

    // Implementation of the ProcessorNetworkObserver interface
    void networkChanged();
    void processorAdded(const Processor* processor);
    void processorRemoved(const Processor* processor);

    void setNetworkEditorWidget(NetworkEditor* networkEditorWidget);
    void setPropertyListWidget(PropertyListWidget* propertyListWidget);
    void setProcessorListWidget(ProcessorListWidget* processorListWidget);
    void setVolumeContainerWidget(VolumeContainerWidget* volumeContainerWidget);

    void setAnimationPlugin(AnimationPlugin* animationPlugin);

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
    void propagateNetwork();
    void propagateVolumeContainer();

    void cleanupTempFiles();

    NetworkEvaluator* evaluator_;
    Workspace* workspace_;
    tgt::GLCanvas* sharedContext_;

    NetworkEditor* networkEditorWidget_;
    PropertyListWidget* propertyListWidget_;
    ProcessorListWidget* processorListWidget_;
    VolumeContainerWidget* volumeContainerWidget_;
    AnimationPlugin* animationPlugin_;

    bool readOnlyWorkspace_;

    std::vector<std::string> workspaceErrors_;
    std::vector<std::string> tmpFiles_; /** files which might have been imported temporary only. */
    std::string tmpPath_;               /** path where the temporary files have been imported to. */
    bool modified_;

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_VOREENVISUALIZATION_H
