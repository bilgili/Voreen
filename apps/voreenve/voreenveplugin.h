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

#ifndef VRN_VOREENVEPLUGIN_H
#define VRN_VOREENVEPLUGIN_H

#include "tgt/exception.h"
#include "voreen/core/network/workspace.h"

#include <QWidget>
#include <QIcon>
#include <QDockWidget>

namespace voreen {

class VoreenModuleVE;
class VoreenMainWindow;
class VoreenToolWindow;
class NetworkEvaluator;
class Workspace;

/**
 * A tool window that is plugged into the VoreenMainWindow.
 *
 * @note All internal widgets should be created in createWidgets()
 *  instead of in the constructor.
 *
 * @note When the plugin is part of a module, it has to be registered
 *  by the module's VoreenModuleVE class.
 */
class VoreenVEPlugin : public QWidget {

    friend class VoreenModuleVE;
    friend class VoreenMainWindow;

public:
    VoreenVEPlugin(QWidget* parent = 0);
    virtual ~VoreenVEPlugin();

    /// Returns the name of the plugin that is used as window title.
    virtual std::string getName() const = 0;

    /// Returns the icon that is used for the plugin's menu action.
    virtual QIcon getIcon() const = 0;

    /**
     * Returns the dock areas of the mainwindow into which the plugin should be allowed
     * to be dragged by the user. Override this method for specifying the docking behaviour.
     * By default, Qt::NoDockWidgetArea is returned, which makes the plugin non-dockable.
     */
    virtual Qt::DockWidgetAreas getAllowedDockWidgetAreas() const;

    /**
     * Returns the dock area to which the plugin should be initially added.
     * The initial dock area has be one the areas returns by getAllowedDockWidgetAreas().
     * By default, Qt::NoDockWidgetArea is returned, which let's the plugin initially float.
     */
    virtual Qt::DockWidgetArea getInitialDockWidgetArea() const;

    /// Returns whether the plugin has already been initialized by the VoreenMainWindow.
    bool isInitialized() const;

protected:
    /**
     * All internal widgets should be created here instead of in the constructor.
     *
     * The method is called by VoreenMainWindow immediately before the plugin
     * is added to the main window. The main window and the network evaluator
     * are already available, when the method is called.
     */
    virtual void createWidgets() = 0;

    /**
     * OpenGL-dependent or time-consuming initializations should be performed here.
     * However, it is usually not necessary to override this function. It is called
     * by the VoreenMainWindow during application initialization.
     *
     * @throw tgt::Exception to indicate an initialization failure
     */
    virtual void initialize()
        throw (tgt::Exception);

    /**
     * OpenGL-dependent or time-consuming deinitializations should be performed here.
     * However, it is usually not necessary to override this function. It is called
     * by the VoreenMainWindow during application initialization.
     *
     * @throw tgt::Exception to indicate an initialization failure
     */
    virtual void deinitialize()
        throw (tgt::Exception);

    /**
     * Returns the application's main window, or 0 if the
     * main window has not yet been set.
     * @see setMainWindow
     */
    VoreenMainWindow* getMainWindow() const;

    /**
     * Returns the tool window the plugin is embedded into, or 0
     * if the parent window has not yet been assigned by the main window.
     * @see setParentWindow
     */
    VoreenToolWindow* getParentWindow() const;

    /**
     * Returns the application's network evaluator, or 0
     * if the evaluator has not yet been assigned by the main window.
     * @see setNetworkEvaluator
     */
    NetworkEvaluator* getNetworkEvaluator() const;

    /**
     * Returns the currently active workspace as assigned
     * by the main window. May be null.
     * @see setWorkspace
     */
    Workspace* getWorkspace() const;

    /// Called by the VoreenMainWindow to assign itself to the plugin.
    virtual void setMainWindow(VoreenMainWindow* mainWindow);

    /// Called by the VoreenMainWindow to assign the plugin's tool window.
    virtual void setParentWindow(VoreenToolWindow* window);

    /// Called by the VoreenMainWindow to assign the network evalulator.
    virtual void setNetworkEvaluator(NetworkEvaluator* evaluator);

    /// Called by the VoreenMainWindow to assign the current workspace.
    virtual void setWorkspace(Workspace* workspace);

private:
    bool initialized_; //< set by VoreenMainWindow

    // the following members are assigned by the VoreenMainWindow
    VoreenMainWindow* mainWindow_;
    VoreenToolWindow* parentWindow_;
    NetworkEvaluator* evaluator_;
    Workspace* workspace_;
};

} // namespace

#endif // VRN_VOREENVEPLUGIN_H
