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

#ifndef VRN_DOCKBARHANDLER_H
#define VRN_DOCKBARHANDLER_H

#include <QDockWidget>
#include <QScrollArea>
#include <QFrame>
#include <QString>

#include "voreen/qt/widgets/plugindialog.h"

#include <vector>

namespace voreen {

/**
 * This class is a replacement for the Qt window dock.
 * An instance of this class can gather an unlimited count of QDockWidget objects and functions
 * just like the default dock, aside from the following improvements:
 *  - The set of dock widgets is surrounded by a scroll-area. This prevents the main window
 *    from resizing itself or squeezing the dock widgets when the dock widgets' joint height
 *    exceeds the dock's height
 *  - The dock widgets are placed from top to bottom (almost) without gaps between them, even
 *    if only some of them are visible
 *  - The allocated width considers the dock widgets' size hint as well as the vertical scrollbar's width,
 *    so the horizontal scrollbar should never appear
 *
 * After adding the dock widgets and finishing the layout you can add the DockBarHandler to any window dock.
 *
 */
class DockBarHandler : public QDockWidget {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param parent Qt parent widget
     * @param title The string that appears in the dockbarhandler's title line.
     */
    DockBarHandler(QWidget* parent = 0, QString title = "");
    /**
     * Just add your dock widgets here ;)
     */
    void addDialog(PluginDialog* pluginDialog);
    /**
     * This function should be called once after all dock widgets have been added
     * and before the dockbarhandler is added to its window dock.
     */
    void finishLayout();

protected slots:
    void topLevelChanged(bool topLevel);
    void viewActionToggled(bool checked);
    void dialogClose(PluginDialog* dialog);

protected:

    void adjustWidthAndVisibility(PluginDialog* dialogToIgnore = NULL);

    /// The dock widgets hold by the dockbarhandler.
    std::vector<PluginDialog*> dockWidgets_;
    /// The frame surrounding all dock widgets
    QFrame* mainFrame_;
    /// The scroll area surrounding the mainFrame_
    QScrollArea* scrollArea_;
    /// Is false until finishLayout() has been called
    bool finished_;


    QWidget* parent_;
};

} // namespace voreen

#endif // VRN_DOCKBARHANDLER_H
