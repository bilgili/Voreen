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

#ifndef LABELINGWIDGETQT_H
#define LABELINGWIDGETQT_H

#include "widgetplugin.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include <QTabWidget>
#include <QListWidget>


namespace voreen {

class LabelingWidgetQt : public WidgetPlugin, LabelingWidget
{
	Q_OBJECT

public:
    LabelingWidgetQt(QWidget* parent=0, MessageReceiver* msgReceiver=0);
    std::string showInputDialog(const std::string &title, 
        const std::string &label,
        const std::string &text);

public slots:
    void selectionChanged(QListWidgetItem* item=0);

protected:
    virtual void createWidgets();
    virtual void createConnections();

    QTabWidget* tabWidget_;
    std::vector<QListWidget*> listWidgets_;
};

}
#endif
