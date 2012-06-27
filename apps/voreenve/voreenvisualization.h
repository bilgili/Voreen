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

#ifndef VRN_VOREENVISUALIZATION_H
#define VRN_VOREENVISUALIZATION_H

#include "voreen/core/network/processornetwork.h"
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
class InputMappingDialog;
class VolumeContainer;
class VolumeContainerWidget;

class VoreenVisualization : public QObject, public ProcessorNetworkObserver {
    Q_OBJECT
public:
    VoreenVisualization(tgt::GLCanvas* sharedContext);
    ~VoreenVisualization();

    void createConnections();

    void exportWorkspaceToZipArchive(const QString& filename, bool overwrite = true)
        throw (SerializationException);

    /**
     * Extracts the specified workspace archive to the passed destination path.
     *
     * @throws SerializationException if the archive could not be extracted,
     *      or if it does not contain a vws-file.
     *
     * @return The full path to the contained workspace file (*.vws).
     */
    QString extractWorkspaceArchive(const QString& archiveName, const QString& destPath,
        bool overwrite = true) throw (SerializationException);

    void newWorkspace();
    void openWorkspace(const QString& filename)
        throw (SerializationException);
    void saveWorkspace(const QString& filename, bool overwrite = true)
        throw (SerializationException);

    void importNetwork(const QString& filename) throw (SerializationException);
    void exportNetwork(const QString& filename) throw (SerializationException);


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
    void propagateNetwork();
    void propagateVolumeContainer();

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
    VolumeContainerWidget* volumeContainerWidget_;
    InputMappingDialog* inputMappingDialog_;

    bool readOnlyWorkspace_;
    bool modified_;
    std::vector<std::string> workspaceErrors_;

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_VOREENVISUALIZATION_H
