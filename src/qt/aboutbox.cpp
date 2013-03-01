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

#include "voreen/qt/aboutbox.h"
#include "voreen/core/version.h"

#include "ui_aboutbox.h"

#include <QStringList>

namespace voreen {

AboutBox::AboutBox(const QString& application, const QString& description, QWidget* parent)
    : QDialog(parent)
{
    Ui::VoreenAboutBox ui;
    ui.setupUi(this);

    QString s;
    s = ui.labelLeft->text();
    s.replace("__VERSION__", QString(VoreenVersion::getVersion().c_str()));
    s.replace("__APPLICATION__", description);
    s.replace("__COPYRIGHT__", QString(VoreenVersion::getCopyright().c_str()));
    ui.labelLeft->setText(s);


    s = ui.labelRight->text();
    s.replace("__REVISION__", "Revision: " + QString(VoreenVersion::getRevision().c_str()).left(8));
    ui.labelRight->setText(s);

    QStringList developers;
    developers << QString::fromLatin1("Alexander Bock");
    developers << QString::fromLatin1("Benjamin Bolte");
    developers << QString::fromLatin1("Helge Böschen");
    developers << QString::fromLatin1("Raphael Bruns");
    developers << QString::fromLatin1("Mathias Dehne");
    developers << QString::fromLatin1("Christian Döring");
    developers << QString::fromLatin1("Maike Dudek");
    developers << QString::fromLatin1("Jan Esser");
    developers << QString::fromLatin1("André Exeler");
    developers << QString::fromLatin1("Dirk Feldmann");
    developers << QString::fromLatin1("Alejandro Figueroa Meana");
    developers << QString::fromLatin1("Timo Griese");
    developers << QString::fromLatin1("Philipp Hanraths");
    developers << QString::fromLatin1("Dieter Janzen");
    developers << QString::fromLatin1("Jens Kasten");
    developers << QString::fromLatin1("Daniel Kirsch");
    developers << QString::fromLatin1("Rico Lehmann");
    developers << QString::fromLatin1("Roland Leißa");
    developers << QString::fromLatin1("Markus Madeja");
    developers << QString::fromLatin1("Zoha Moztarzadeh");
    developers << QString::fromLatin1("Reza Nawrozi");
    developers << QString::fromLatin1("Borislav Petkov");
    developers << QString::fromLatin1("Stephan Rademacher");
    developers << QString::fromLatin1("Rainer Reich");
    developers << QString::fromLatin1("Mona Riemenschneider");
    developers << QString::fromLatin1("Christoph Rosemann");
    developers << QString::fromLatin1("Jan Roters");
    developers << QString::fromLatin1("Sönke Schmid");
    developers << QString::fromLatin1("Christian Schulte zu Berge");
    developers << QString::fromLatin1("Michael Specht");
    developers << QString::fromLatin1("Fabian Spiegel");
    developers << QString::fromLatin1("David Terbeek");
    developers << QString::fromLatin1("Christian Vorholt");
    developers << QString::fromLatin1("Carolin Walter");
    developers << QString::fromLatin1("Michael Weinkath");
    developers << QString::fromLatin1("Frank Wisniewski");
    developers << QString::fromLatin1("Marco Ziolkowski");
//    developers.replaceInStrings(" ", "&nbsp;");


    QString developersString;
    for (int i=0; i < developers.size(); i++) {
        if (i > 0)
            developersString += ", ";
        developersString += developers[i];
    }
    s = ui.labelDevelopers->text();
    s.replace("__DEVELOPERS__", developersString);
    ui.labelDevelopers->setText(s);

    QStringList mainDevelopers;
    mainDevelopers << QString::fromLatin1("Tobias Brix");
    mainDevelopers << QString::fromLatin1("Stefan Diepenbrock");
    mainDevelopers << QString::fromLatin1("Florian Lindemann");
    mainDevelopers << QString::fromLatin1("Jörg Mensmann");
    mainDevelopers << QString::fromLatin1("Jennis Meyer-Spradow");
    mainDevelopers << QString::fromLatin1("Jörg-Stefan Praßni");
    mainDevelopers << QString::fromLatin1("Timo Ropinski");
//    mainDevelopers.replaceInStrings(" ", "&nbsp;");

    QString mainDevelopersString;
    for (int i=0; i < mainDevelopers.size(); i++) {
        if (i > 0)
            mainDevelopersString += ", ";
        mainDevelopersString += mainDevelopers[i];
    }
    s = ui.labelMainDevelopers->text();
    s.replace("__MAINDEVELOPERS__", mainDevelopersString);
    ui.labelMainDevelopers->setText(s);

    setWindowTitle(tr("About %1...").arg(application));

#if (QT_VERSION >= 0x040400) && !defined(__APPLE__) && !defined(VRN_NO_STYLESHEET)
    setStyleSheet("QDialog { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #444444, stop:1 #aaaaaa) }\n"
                  "QFrame#frame { background-color: #8E8E8E }");
#endif


    adjustSize();
}

} // namespace
