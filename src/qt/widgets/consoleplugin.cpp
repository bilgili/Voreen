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

#include <QApplication>
#include <QVBoxLayout>

namespace voreen {

class ConsoleLogQt : public tgt::Log {
public:
    ConsoleLogQt(ConsolePlugin* plugin, std::string debugStyle = "", std::string infoStyle = "", 
        std::string warnStyle = "", std::string errorStyle = "",
        bool timeStamping = false, bool dateStamping = false, 
        bool showCat = true, bool showLevel = true)
        : tgt::Log(),
          plugin_(plugin),
          debugStyle_(debugStyle),
          infoStyle_(infoStyle),
          warnStyle_(warnStyle),
          errorStyle_(errorStyle)
    {
        timeStamping_ = timeStamping;
        dateStamping_ = dateStamping;
        showCat_ = showCat;
        showLevel_ = showLevel;
    }

    bool isOpen() { return true; }

protected:
    void logFiltered(const std::string &cat, tgt::LogLevel level, const std::string &msg, const std::string & /*extendedInfo*/ ="") {
        std::string output;

        std::string style;
        if (level == tgt::Debug)
            style = debugStyle_;
        else if (level == tgt::Info)
            style = infoStyle_;
        else if (level == tgt::Warning)
            style = warnStyle_;
        else if (level == tgt::Error)
            style = errorStyle_;

        if (dateStamping_)
            output += "[" + getDateString() + "] ";
        if (timeStamping_)
            output += "[" + getTimeString() + "] ";
        if (showCat_)
            output += cat + " ";
        if (showLevel_)
            output += "(" + getLevelString(level) + ")";
        //output += "\t";

        output += "&nbsp;&nbsp;";

        output += "<span style=\"" + style + "\">";
        output += msg;
        output += "</span>";

        plugin_->log(output);
    }

    ConsolePlugin* plugin_;
    std::string debugStyle_;
    std::string infoStyle_;
    std::string warnStyle_;
    std::string errorStyle_;
};

ConsolePlugin::ConsolePlugin(QWidget* parent)
  : WidgetPlugin(parent)
{
    setObjectName(tr("Console"));

    consoleText_ = new QTextEdit(this);
    consoleText_->setReadOnly(true);
    consoleText_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->addWidget(consoleText_);
    setLayout(vboxLayout);

    log_ = new ConsoleLogQt(this, "", "", "color: brown; font-weight: bold", "color: red; font-weight: bold");
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
