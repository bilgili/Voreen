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

#include "voreen/qt/thresholdwidget.h"
#include "voreen/qt/widgets/thresholdwidgetplugin.h"

#include <QtPlugin>

ThresholdWidgetPlugin::ThresholdWidgetPlugin(QObject *parent) : QObject(parent) {
    initialized = false;
}

bool ThresholdWidgetPlugin::isContainer() const {
    return false;
}

bool ThresholdWidgetPlugin::isInitialized() const {
    return initialized;
}

QIcon ThresholdWidgetPlugin::icon() const {
    return QIcon();
}

QString ThresholdWidgetPlugin::codeTemplate() const {
    return "";
}

QString ThresholdWidgetPlugin::domXml() const {
    return "<widget class=\"ThresholdWidget\" name=\"TresholdWidget\">\n"
               " <property name=\"geometry\">\n"
               "  <rect>\n"
               "   <x>0</x>\n"
               "   <y>0</y>\n"
               "   <width>250</width>\n"
               "   <height>120</height>\n"
               "  </rect>\n"
               " </property>\n"
               " <property name=\"toolTip\" >\n"
               "  <string>Set the lower and upper threshold value</string>\n"
               " </property>\n"
               "</widget>\n";

}

QString ThresholdWidgetPlugin::group() const {
    return "Voreen Widgets";
}

QString ThresholdWidgetPlugin::includeFile() const {
    return "voreen/qt/thresholdwidget.h";
}

QString ThresholdWidgetPlugin::name() const {
    return "ThresholdWidget";
}

QString ThresholdWidgetPlugin::toolTip() const {
    return "";
}

QString ThresholdWidgetPlugin::whatsThis() const {
    return "";
}

QWidget *ThresholdWidgetPlugin::createWidget(QWidget *parent) {
    return new ThresholdWidget(parent);
}

void ThresholdWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
    if(initialized) {
        return;
    }
    initialized = true;
}

Q_EXPORT_PLUGIN2(thresholdwidgetplugin, ThresholdWidgetPlugin)
} // namespace voreen

