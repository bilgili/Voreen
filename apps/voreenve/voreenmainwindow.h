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

#ifndef VRN_VOREENMAINWINDOW_H
#define VRN_VOREENMAINWINDOW_H

#include "voreenvisualization.h"

#include <QtGui>
#include <QSplashScreen>

namespace tgt {
    class QtCanvas;
}

namespace voreen {

class ConsolePlugin;
class NetworkEvaluator;
class PropertyListWidget;
class NetworkEditor;
class ProcessorNetwork;
class ProcessorListWidget;
class VolumeContainer;
class VolumeContainerWidget;
class VoreenToolWindow;
class VoreenToolDockWindow;
class InputMappingDialog;
class RenderTargetViewer;
class LinkingScriptManager;
class AnimationEditor;


//---------------------------------------------------------------------------

class VoreenSplashScreen : public QSplashScreen {
public:
    VoreenSplashScreen();
    ~VoreenSplashScreen();

    void drawContents(QPainter* painter);
    void showMessage(const QString& message);

protected:
    QPixmap* pixmap_;
    QString message_;
};

//---------------------------------------------------------------------------

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
        MODE_APPLICATION,
        MODE_DEVELOPMENT
    };

    VoreenMainWindow(const std::string& workspace = "", const std::string& dataset = "");
    ~VoreenMainWindow();

    void init(VoreenSplashScreen* splash = 0);

signals:
    void closeMainWindow();

public slots:
    void setWindowTitle(const QString& title);

    // network
    void openNetwork();
    void openNetwork(const QString& filename);
    bool saveNetworkAs();

    // workspace
    void exportWorkspace();
    void extractWorkspaceArchive();
    void extractWorkspaceArchive(QString archivFile);
    void newWorkspace();
    void openWorkspace();
    void openWorkspace(const QString& filename);
    bool saveWorkspace(const QString& filename = "");
    bool saveWorkspaceAs();

    // dataset
    void openDataset();
    void openRawDataset();
    void openRecentFile();

    void buttonAddDICOMClicked();

    // action menu
    void adaptWidgetsToNetwork();
    void rebuildShaders();
    void runScript();
    void runScript(const QString& filename);

    // option menu
    void setLoadLastWorkspace();
    void setReuseTargets();

    // help menu
    void helpFirstSteps();
    void helpAnimation();
    void helpWebsite();
    void helpAbout();

    // further slots
    void guiModeChanged();

protected:
    void changeEvent(QEvent* event);

protected slots:
    void snapshotActionTriggered(bool checked);

    /// Adjust the canvas widgets to the currently active gui mode.
    void adjustCanvasWidgets(GuiMode guiMode);

    /// Adjust snapshot tool menu to network.
    void adjustSnapshotMenu();

    /// Updates the window after network modifications
    void updateWindowTitle();

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
    VoreenToolWindow* addToolDockWindow(QAction* action, QWidget* widget, const QString& name = "",
                                        Qt::DockWidgetArea dockarea = Qt::LeftDockWidgetArea,
                                        Qt::DockWidgetAreas allowedAreas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea ,
                                        bool basic = true);

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
    // startUp management
    //
    void startupComplete(const std::string& phaseName);

    //
    // further methods
    //
    void loadDataset(const std::string& filename);
    bool askSave();
    void showNetworkErrors();
    void showWorkspaceErrors();

    void addToRecentFiles(const QString& filename);
    void updateRecentFiles();

    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    void setGuiMode(GuiMode guiMode);

    GuiMode guiMode_;
    AnimationEditor* animationEditor_;

    tgt::QtCanvas* sharedContext_;
    VolumeContainerWidget* volumeContainerWidget_;
    NetworkEditor* networkEditorWidget_;
    InputMappingDialog* inputMappingDialog_;
    RenderTargetViewer* renderTargetViewer_;
    LinkingScriptManager* linkingScriptManager_;

    VoreenVisualization* vis_;

    QToolBar* fileToolBar_;
    QToolBar* viewToolBar_;
    QToolBar* toolsToolBar_;
    QToolBar* actionToolBar_;
    QList<VoreenToolWindow*> toolWindows_;

    VoreenToolWindow* propertyListTool_;
    PropertyListWidget* propertyListWidget_;
    ProcessorListWidget* processorListWidget_;
    VoreenToolWindow* processorListTool_;
    VoreenToolWindow* volumeContainerTool_;
    VoreenToolWindow* consoleTool_;

    QSettings settings_;
    QByteArray applicationModeState_;
    QByteArray developmentModeState_;
    QByteArray networkEditorWindowState_;

    QMdiArea* mdiArea_;
    VoreenMdiSubWindow* networkEditorWindow_;
    VoreenMdiSubWindow* shortcutPrefWindow_;

    // Main menu
    QMenuBar* menu_;
    QMenu* fileMenu_;
    QMenu* viewMenu_;
    QMenu* toolsMenu_;
    QMenu* actionMenu_;
    QMenu* optionsMenu_;
    QMenu* helpMenu_;

    QAction* workspaceNewAction_;
    QAction* workspaceOpenAction_;
    QAction* workspaceSaveAction_;
    QAction* workspaceSaveAsAction_;

    QAction* aboutAction_;
    QAction* helpFirstStepsAct_;
    QAction* helpAnimationAct_;
    QAction* openDatasetAction_;
    QAction* openRawDatasetAction_;
    QAction* importNetworkAction_;
    QAction* exportNetworkAction_;
    QAction* showShortcutPreferencesAction_;
    QAction* openDicomFilesAct_;
    QAction* quitAction_;

    QAction* workspaceExtractAction_;

    QAction* rebuildShadersAction_;
    QAction* loadLastWorkspaceAct_;
    QAction* scriptAction_;

    QAction* modeApplicationAction_;
    QAction* modeDevelopmentAction_;

    QAction* processorListAction_;
    QAction* snapshotAction_;

    QList<QAction*> recentFileActs_;

    ConsolePlugin* consolePlugin_;

    bool resetSettings_;
    bool loadLastWorkspace_;
    QString lastWorkspace_;

    QString networkPath_;
    QString workspacePath_;

    QString currentNetwork_;
    QString currentWorkspace_;
    QString defaultDataset_;

    QString originalWindowTitle_;
    tgt::ivec2 canvasPos_;
    tgt::ivec2 canvasSize_;

    bool ignoreWindowTitleModified_; ///< will not add * to the window title when this is set

    // startUp
    bool startupWorkspace_;
};

} // namespace

#endif // VRN_VOREENMAINWINDOW_H
