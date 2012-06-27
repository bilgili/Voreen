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

#include "voreen/qt/aboutbox.h"
#include "voreen/core/version.h"

#include "ui_aboutbox.h"

#include <QStringList>

namespace voreen {

AboutBox::AboutBox(const QString& application, const QString& description, const QString& version, QWidget* parent)
    : QDialog(parent)
{
    Ui::VoreenAboutBox ui;
    ui.setupUi(this);

    QString s;
    s = ui.labelLeft->text();
    s.replace("__VERSION__", "1.7");
    s.replace("__APPLICATION__", description + " " + version);
    s.replace("__COPYRIGHT__", QString::fromLatin1("(C) 2004-2009 University of Münster"));
    ui.labelLeft->setText(s);


    s = ui.labelRight->text();
    if (!VoreenVersion::getRevision().empty())
        s.replace("__SVNVERSION__", "svn version " + QString(VoreenVersion::getRevision().c_str()));
    else
        s.replace("__SVNVERSION__", "");
    ui.labelRight->setText(s);

    QStringList developers;
    developers << QString::fromLatin1("Alexander Bock");
    developers << QString::fromLatin1("Christian Döring");
    developers << QString::fromLatin1("Jan Esser");
    developers << QString::fromLatin1("Dirk Feldmann");
    developers << QString::fromLatin1("Jens Kasten");
    developers << QString::fromLatin1("Daniel Kirsch");
    developers << QString::fromLatin1("Roland Leißa");
    developers << QString::fromLatin1("Florian Lindemann");
    developers << QString::fromLatin1("Markus Madeja");
    developers << QString::fromLatin1("Borislav Petkov");
    developers << QString::fromLatin1("Stephan Rademacher");
    developers << QString::fromLatin1("Rainer Reich");
    developers << QString::fromLatin1("Christoph Rosemann");
    developers << QString::fromLatin1("Jan Roters");
    developers << QString::fromLatin1("Michael Specht");
    developers << QString::fromLatin1("Christian Vorholt");
    developers << QString::fromLatin1("Frank Wisniewski");

    developers.replaceInStrings(" ", "&nbsp;");

    QString developersString;
    for (int i=0; i < developers.size(); i++) {
        if (i > 0)
            developersString += ", ";
        developersString += developers[i];
    }

    s = ui.labelDevelopers->text();
    s.replace("__DEVELOPERS__", developersString);
    ui.labelDevelopers->setText(s);

    setWindowTitle(tr("About %1...").arg(application));

    adjustSize();
}

} // namespace
