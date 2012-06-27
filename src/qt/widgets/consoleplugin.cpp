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

#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/qdebug.h"

#include "tgt/logmanager.h"


#include <QVBoxLayout>

namespace voreen {

class ConsoleLogQt : public tgt::Log {
public:
    ConsoleLogQt(ConsolePlugin* plugin)
        : tgt::Log(),
          plugin_(plugin)
    {
        timeStamping_ = false;
        dateStamping_ = false;
        showCat_ = true;
        showLevel_ = true;
    }

    bool isOpen() { return true; }

protected:
	void logFiltered(const std::string &cat, tgt::LogLevel level, const std::string &msg, const std::string & /*extendedInfo*/ ="") {
        std::string output;

        if (dateStamping_)
            output += "[" + getDateString() + "] ";
        if (timeStamping_)
            output += "[" + getTimeString() + "] ";
        if (showCat_)
            output += cat + " ";
        if (showLevel_)
            output += "(" + getLevelString(level) + ") ";
        if (output != "")
            output += '\t';

        output += msg;

        plugin_->log(output);
    }

    ConsolePlugin* plugin_;
};

ConsolePlugin::ConsolePlugin(QWidget* parent, MessageReceiver* msgReceiver)
  : WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Console"));

    consoleText_ = new QTextEdit(this);
    consoleText_->setReadOnly(true);
    consoleText_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->addWidget(consoleText_);
    setLayout(vboxLayout);
    
    log_ = new ConsoleLogQt(this);
    log_->addCat("", true, tgt::Info);
    LogMgr.addLog(log_);
}

void ConsolePlugin::log(const std::string& msg) {
    consoleText_->append(msg.c_str());
    consoleText_->ensureCursorVisible();
}

void ConsolePlugin::createWidgets() {
}

void ConsolePlugin::createConnections() {
}

} // namespace voreen
