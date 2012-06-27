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

#include "voreen/qt/widgets/consoleplugin.h"

#include "tgt/logmanager.h"

#include <QApplication>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>

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

        output += "&nbsp;&nbsp;";

        output += "<span style=\"" + style + "\">";

        size_t lookHere = 0;
        size_t foundHere;
        std::string s = msg;
        const std::string br = "<br/>";
        while ((foundHere = s.find("\n", lookHere)) != std::string::npos) {
            s.replace(foundHere, 1, br);
            lookHere = foundHere + br.size();
        }

        output += s;
        output += "</span>";

        plugin_->log(output);
    }

    ConsolePlugin* plugin_;
    std::string debugStyle_;
    std::string infoStyle_;
    std::string warnStyle_;
    std::string errorStyle_;
};

ConsolePlugin::ConsolePlugin(QWidget* parent, bool autoScroll)
    : QWidget(parent)
    , autoScroll_(autoScroll)
{
    setObjectName(tr("Console"));

    consoleText_ = new QTextEdit(this);
    consoleText_->setReadOnly(true);
    consoleText_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->addWidget(consoleText_);
    setLayout(vboxLayout);

    if (tgt::Singleton<tgt::LogManager>::isInited()) {
        log_ = new ConsoleLogQt(this, "", "", "color: brown; font-weight: bold", "color: red; font-weight: bold");
        log_->addCat("", true, tgt::Info);
        LogMgr.addLog(log_);
    }
}

ConsolePlugin::~ConsolePlugin() {
    if (tgt::Singleton<tgt::LogManager>::isInited()) {
        LogMgr.removeLog(log_);
        delete log_;
    }
}

void ConsolePlugin::log(const std::string& msg) {

    // write log message to text box
    consoleText_->append(msg.c_str());

    // scroll to bottom
    if (autoScroll_ && isVisible()) {
        QScrollBar* scrollBar = consoleText_->verticalScrollBar();
        if (scrollBar)
            scrollBar->setValue(scrollBar->maximum());
    }
}

} // namespace voreen
