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

#include "voreen/qt/createtrfnwidget.h"
#include "createtrfnwidgetplugin.h"

#include <QtPlugin>

CreateTrFnWidgetPlugin::CreateTrFnWidgetPlugin(QObject *parent) : QObject(parent) {
    initialized = false;
}

bool CreateTrFnWidgetPlugin::isContainer() const {
    return false;
}

bool CreateTrFnWidgetPlugin::isInitialized() const {
    return initialized;
}

QIcon CreateTrFnWidgetPlugin::icon() const {
    return QIcon();
}

QString CreateTrFnWidgetPlugin::codeTemplate() const {
    return "";
}

QString CreateTrFnWidgetPlugin::domXml() const {
    return "<widget class=\"CreateTrFnWidget\" name=\"CreateTrFnWidget\">\n"
               " <property name=\"geometry\">\n"
               "  <rect>\n"
               "   <x>0</x>\n"
               "   <y>0</y>\n"
               "   <width>152</width>\n"
               "   <height>165</height>\n"
               "  </rect>\n"
               " </property>\n"
               " <property name=\"toolTip\" >\n"
               "  <string></string>\n"
               " </property>\n"
               " <property name=\"whatsThis\" >\n"
               "  <string>"
               "</string>\n"
               " </property>\n"
               "</widget>\n";

}

QString CreateTrFnWidgetPlugin::group() const {
    return "Voreen Widgets";
}

QString CreateTrFnWidgetPlugin::includeFile() const {
    return "voreen/qt/createtrfnwidget.h";
}

QString CreateTrFnWidgetPlugin::name() const {
    return "CreateTrFnWidget";
}

QString CreateTrFnWidgetPlugin::toolTip() const {
    return "";
}

QString CreateTrFnWidgetPlugin::whatsThis() const {
    return "";
}

QWidget *CreateTrFnWidgetPlugin::createWidget(QWidget *parent) {
    return new CreateTrFnWidget(parent);
}

void CreateTrFnWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
    if(initialized) {
        return;
    }
    initialized = true;
}

Q_EXPORT_PLUGIN2(CreateTrFnWidgetPlugin, CreateTrFnWidgetPlugin)
} // namespace voreen

