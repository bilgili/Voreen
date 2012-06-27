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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDockWidget>
#include <QScrollArea>
#include "widgetplugin.h"

class QVBoxLayout;

namespace voreen {

///This class shows widgetplugins in a dialogwindow.
class PluginDialog : public QDockWidget
{
    Q_OBJECT
public:
    PluginDialog(WidgetPlugin* plugin, QWidget* parent = 0, bool hideAtStartup = true);
    QAction* createAction();
    WidgetPlugin* getPlugin() { return plugin_; }
    
public slots:
    void floating(bool fl);

signals:
    void dialogClose(PluginDialog* dialog);

protected:
    void closeEvent(QCloseEvent* event); 
    
    WidgetPlugin* plugin_;      /// the plugin thats capsulated in this dialog
    QScrollArea* scrollArea_;   /// the scroll area around the plugin
};

} // namespace voreen

#endif //PLUGINDIALOG_H
