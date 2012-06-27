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

#ifndef VRN_VOREENMAINWINDOW_H
#define VRN_VOREENMAINWINDOW_H

#include <QtGui>

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

class ConsolePlugin;
class NetworkEvaluator;
class RptPropertyListWidget;
class RptGraphWidget;
class RptNetwork;
class RptPainterWidget;
class RptProcessorListWidget;
class VolumeSetContainer;
class VolumeSetWidget;
class VoreenToolWindow;
class VoreenToolDockWindow;
class WidgetPlugin;

//---------------------------------------------------------------------------

class VoreenMainWindow;

class VoreenVisualization : public QObject {
    Q_OBJECT
public:
    VoreenVisualization();
    ~VoreenVisualization();

    void init();
    
    NetworkEvaluator* getEvaluator() const { return evaluator_; }
    VolumeSetContainer* getVolumeSetContainer() const { return volsetContainer_; }
    tgt::Camera* getCamera() const { return camera_; }

    void openNetwork(const std::string& filename, VoreenMainWindow* mainwindow) throw (SerializerException);
    void saveNetwork(const std::string& filename, bool reuseTCTargets) throw (SerializerException);

    void newWorkspace();
    void openWorkspace(const std::string& filename, VoreenMainWindow* mainwindow)
        throw (SerializerException);
    void saveWorkspace(const std::string& filename, bool reuseTCTargets, VoreenMainWindow* mainwindow)
        throw (SerializerException);

    void clearScene();
    void setNetwork(RptNetwork* network);
    std::vector<std::string> getNetworkErrors();
    bool evaluateNetwork();
    bool rebuildShaders();

    void setRenderWidget(RptPainterWidget* renderWidget);
    void setNetworkEditorWidget(RptGraphWidget* networkEditorWidget);
    void setVolumeSetWidget(VolumeSetWidget* volumeSetWidget);

    bool readOnlyWorkspace() const { return readOnlyWorkspace_; }

signals:
    void networkLoaded(RptNetwork* network);
    void processorDeleted(Processor* processor);

public slots:    
    void deleteFromNetwork();
    
private:
    NetworkEvaluator* evaluator_;
    RptNetwork* rptnet_;
    VolumeSetContainer* volsetContainer_;
    GeometryContainer* geoContainer_;
    tgt::Camera* camera_;

    RptPainterWidget* renderWidget_;
    RptGraphWidget* networkEditorWidget_;
    VolumeSetWidget* volumeSetWidget_;
    bool readOnlyWorkspace_;
};

class VoreenMdiSubWindow : public QMdiSubWindow {
public:
    VoreenMdiSubWindow(QWidget* widget, QWidget* parent, Qt::WindowFlags flags = 0);

    QByteArray saveGeometry() const;
    bool restoreGeometry(const QByteArray& geometry);
protected:
    void closeEvent(QCloseEvent* event);   
};

/**
 * The main window that contains all the other widgets, tool bar, menu bar, etc.
 */
class VoreenMainWindow : public QMainWindow {
    Q_OBJECT
public:

    enum GuiMode {
        MODE_NONE,
        MODE_VISUALIZATION,
        MODE_NETWORK
    };
    
    VoreenMainWindow(const std::string& network = "", const std::string& dataset = "");
    ~VoreenMainWindow();

    void init();

               
public slots:
    void setWindowTitle(const QString& title);

    // network
    void openNetwork();
    void openNetwork(const QString& filename);
    bool saveNetworkAs();
    void setNetwork(RptNetwork* network);

    // workspace
    void newWorkspace();
    void openWorkspace();
    void openWorkspace(const QString& filename);
    void saveWorkspace(const QString& filename = "");
    void saveWorkspaceAs();

    // dataset
    void openDataset();

    void openRecentFile();

    // action menu
    void evaluateNetwork();
    void rebuildShaders();
    void runScript();

    // option menu
    void navigationChanged();
    void setLoadLastWorkspace();
    void setReuseTargets();
    
    // help menu
    void helpAbout();
    void helpFirstSteps();

    // further slots
    void processorSelected(Processor* processor);
    void showProperties();
    void modified();

    void guiModeChanged();

protected:
    void changeEvent(QEvent* event);
    
private:
    //
    // GUI setup
    //
    
    void createMenus();
    void createToolBars();

    /**
     * Adds an entry to the "Tools" menu and toolbar for a non-dockable tool window.
     *
     * @param name object name used for serialization of position and size
     * @return the newly created window
     */
    VoreenToolWindow* addToolWindow(QAction* action, QWidget* widget, const QString& name = "", bool basic = true);

    /**
     * Adds an entry to the "Tools" menu and toolbar for a dockable tool window.
     *
     * @param name object name used for serialization of position and size
     * @return the newly created window
     */
    VoreenToolDockWindow* addToolDockWindow(QAction* action, QWidget* widget, const QString& name = "",
                                            Qt::DockWidgetArea dockarea = Qt::LeftDockWidgetArea, bool basic = true);
    
    /**
     * Create all tool windows. Has to be called after tgt::initGL() and initCanvas().
     */
    void createToolWindows();

    //
    // settings
    //
    void loadSettings();
    void loadWindowSettings();
    void saveSettings();

    //
    // further methods
    //
    void loadDataset(const std::string& filename, bool showProgress = true);
    void clearScene();
    bool askSave();
    void showNetworkErrors();

    void addToRecentFiles(const QString& filename);
    void updateRecentFiles();

    void updateWindowTitle();
    
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    void setGuiMode(GuiMode guiMode);
    
    GuiMode guiMode_;

    RptPainterWidget* renderWidget_;
    VolumeSetWidget* volumeSetWidget_;
    RptGraphWidget* networkEditorWidget_;

    VoreenVisualization* vis_;
    
    QToolBar* fileToolBar_;
    QToolBar* toolsToolBar_;
    QToolBar* guiModeToolBar_;
    QToolBar* processorToolsToolBar_;
    QList<VoreenToolWindow*> toolWindows_;
    QList<std::pair<WidgetPlugin*, QAction*> > tools_;
    
    VoreenToolDockWindow* propertyListTool_;
    RptPropertyListWidget* propertyListWidget_;
    RptProcessorListWidget* processorListWidget_;
    VoreenToolDockWindow* processorListTool_;
    
    QSettings settings_;
    QByteArray visualizationModeState_;
    QByteArray networkModeState_;
    QByteArray renderWindowStateNetworkMode_;
    QByteArray renderWindowStateVisualizationMode_;
    QByteArray networkEditorWindowState_;

    QMdiArea* mdiArea_;
    VoreenMdiSubWindow* networkEditorWindow_;
    VoreenMdiSubWindow* renderWindow_;
    
    // Main menu
    QMenuBar* menu_;
    QMenu* fileMenu_;
    QMenu* dicomMenu_;
    QMenu* editMenu_;
    QMenu* actionMenu_;
    QMenu* toolsMenu_;
    QMenu* optionsMenu_;
    QMenu* helpMenu_;
    QMenu* navigationMenu_;

    QAction* workspaceNewAction_;
    QAction* workspaceOpenAction_;
    QAction* workspaceSaveAction_;
    QAction* workspaceSaveAsAction_;

    QAction* aboutAction_;
    QAction* evaluatorAction_;
    QAction* helpFirstStepsAct_;
    QAction* openDatasetAction_;
    QAction* openNetworkFileAction_;
    QAction* saveNetworkAsAction_;
    QAction* openDicomDirAct_;
    QAction* openDicomFilesAct_;
    QAction* quitAction_;
    QAction* trackballNaviAction_;
    QAction* flythroughNaviAction_;
    QActionGroup* navigationGroup_;

    QAction* setReuseTargetsAction_;
    QAction* rebuildShadersAction_;
    QAction* loadLastWorkspaceAct_;
    QAction* scriptAction_;

    QAction* modeVisualizationAction_;
    QAction* modeNetworkAction_;
    
    QAction* processorListAction_;

    QList<QAction*> recentFileActs_;
        
    ConsolePlugin* consolePlugin_;
    
    bool resetSettings_;
    bool loadLastWorkspace_;
    QString lastWorkspace_;

    QString datasetPath_;
    QString networkPath_;
    QString workspacePath_;

    QString currentNetwork_; 
    QString currentWorkspace_;   
    QString defaultDataset_;

    QString originalWindowTitle_;
};

} // namespace

#endif // VRN_VOREENMAINWINDOW_H
