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

#ifndef VRN_VOREENMAINWINDOW_H
#define VRN_VOREENMAINWINDOW_H

#include "voreenvisualization.h"

#include <QtGui>
#include <QSplashScreen>

#include <vector>

namespace tgt {
    class QtCanvas;
}

namespace voreen {

class VoreenToolWindow;
class VoreenVEPlugin;

class ProcessorListWidget;
class VolumeViewer;
class PropertyListWidget;
class ConsolePlugin;
class InputMappingDialog;
class RenderTargetViewer;
class AnimationEditor;
class VoreenSettingsDialog;

//---------------------------------------------------------------------------

class VoreenSplashScreen : public QSplashScreen {
public:
    VoreenSplashScreen();
    ~VoreenSplashScreen();

    void drawContents(QPainter* painter);
    void showMessage(const QString& message, qreal progress);
    void setProgress(qreal progress);

protected:
    QPixmap* pixmap_;
    QString message_;
    qreal progress_;
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

    VoreenMainWindow(const std::string& workspace = "", bool noInitialWorkspace = false, bool resetSettings = false);
    ~VoreenMainWindow();

    void initialize(VoreenSplashScreen* splash = 0);
    void deinitialize();

signals:
    void closeMainWindow();

public slots:
    void setWindowTitle(const QString& title);

    // network
    void openNetwork();
    void openNetwork(const QString& filename);
    bool saveNetworkAs();

    // workspace
    void newWorkspace();
    void openWorkspace();
    void openWorkspace(const QString& filename);
    bool saveWorkspace(const QString& filename = "");
    bool saveWorkspaceAs();
    bool saveWorkspaceCopyAs();

    // dataset
    void openRecentFile();

    // action menu
    void adaptWidgetsToNetwork();
    void rebuildShaders();

    // option menu
    void setLoadLastWorkspace();
    void setReuseTargets();

    // help menu
    void helpFirstSteps();
    void helpNetworkEditor();
    void helpAnimation();
    void helpTutorialSlides();
    void helpWebsite();
    void helpAbout();

    // further slots
    void guiModeChanged();

protected:
    void changeEvent(QEvent* event);

protected slots:
    void screenshotActionTriggered(bool checked);

    /// Adjust the canvas widgets to the currently active gui mode.
    void adjustCanvasWidgets(GuiMode guiMode);

    /// Adjust screenshot tool menu to network.
    void adjustScreenshotMenu();

    /// Updates the window after network modifications
    void updateWindowTitle();

private:
    //
    // GUI setup
    //
    void createMenus();
    void createToolBars();

    /// Create all tool windows. Has to be called after tgt::initGL() and initCanvas().
    void createToolWindows();

    /// Wraps each VoreenVEPlugin with a tool window and adds it to the main window.
    void addVEPlugins();

    /**
     * Adds an entry to the "Tools" menu and toolbar for a dockable tool window.
     *
     * @param name object name used for serialization of position and size
     * @param dockArea the initial dock area the window should be added to.
     *  Use Qt::NoDockWidgetArea to let the tool window initially float.
     * @param allowedAreas The dock areas the window can be dragged into by the user.
     *  Use Qt::NoDockWidgetArea for a non-dockable tool window.
     * @return the newly created window
     */
    VoreenToolWindow* addToolWindow(QAction* action, QWidget* widget, const QString& name = "",
                                        Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea,
                                        Qt::DockWidgetAreas allowedAreas = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea ,
                                        bool basic = true);

    /**
     * Returns the tool window that encloses the passed widget.
     */
    VoreenToolWindow* getToolWindow(QWidget* childWidget) const;


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
    bool askSave();
    void showNetworkErrors();
    void showWorkspaceErrors();

    void addToRecentFiles(const QString& filename);
    void updateRecentFiles();

    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    void setGuiMode(GuiMode guiMode);

    VoreenVisualization* vis_;
    tgt::QtCanvas* sharedContext_;


    //
    // Menus and tool bars
    //

    // menus
    QMenuBar* menu_;
    QMenu* fileMenu_;
    QMenu* viewMenu_;
    QMenu* toolsMenu_;
    QMenu* optionsMenu_;
    QMenu* helpMenu_;

    // tool bars
    QToolBar* fileToolBar_;
    QToolBar* viewToolBar_;
    QToolBar* toolsToolBar_;

    // actions
    QAction* workspaceNewAction_;
    QAction* workspaceOpenAction_;
    QAction* workspaceSaveAction_;
    QAction* workspaceSaveAsAction_;
    QAction* workspaceSaveCopyAsAction_;

    QAction* aboutAction_;
    QAction* helpFirstStepsAct_;
    QAction* helpNetworkEditorAct_;
    QAction* helpAnimationAct_;
    QAction* helpTutorialSlidesAct_;
    QAction* importNetworkAction_;
    QAction* exportNetworkAction_;
    QAction* showShortcutPreferencesAction_;
    QAction* quitAction_;

    QAction* rebuildShadersAction_;
    //QAction* enterWhatsThisAction_;
    QAction* loadLastWorkspaceAct_;

    QAction* modeApplicationAction_;
    QAction* modeDevelopmentAction_;

    QAction* screenshotAction_;

    QList<QAction*> recentFileActs_;


    //
    // Tools and plugins
    //
    QList<VoreenToolWindow*> toolWindows_; //< each tool is wrapped by a tool window
    std::vector<VoreenVEPlugin*> plugins_;

    // network editor
    QMdiArea* mdiArea_;
    VoreenMdiSubWindow* networkEditorWindow_;
    NetworkEditor* networkEditorWidget_;

    // tools
    ProcessorListWidget* processorListWidget_;
    PropertyListWidget* propertyListWidget_;
    VolumeViewer* volumeViewer_;

    ConsolePlugin* consolePlugin_;
    InputMappingDialog* inputMappingDialog_;
    AnimationEditor* animationEditor_;
    RenderTargetViewer* renderTargetViewer_;

    // additional widgets
    VoreenSettingsDialog* settingsEditor_;


    //
    // Application state
    //
    GuiMode guiMode_;

    QSettings settings_;
    QByteArray applicationModeState_;
    QByteArray developmentModeState_;
    QByteArray networkEditorWindowState_;

    bool noInitialWorkspace_;
    bool loadLastWorkspace_;
    QString lastWorkspace_;

    QString networkPath_;
    QString workspacePath_;

    QString currentWorkspace_;
    QString currentWorkspaceWorkDir_;

    QString originalWindowTitle_;

    bool ignoreWindowTitleModified_; ///< will not add * to the window title when this is set

    bool startupWorkspace_;

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_VOREENMAINWINDOW_H
