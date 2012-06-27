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

#include "voreen/qt/widgets/transferfuncwidget.h"
#include "transferfuncwidgetplugin.h"

#include <QtPlugin>

TransferFuncWidgetPlugin::TransferFuncWidgetPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

bool TransferFuncWidgetPlugin::isContainer() const {
	return false;
}

bool TransferFuncWidgetPlugin::isInitialized() const {
	return initialized;
}

QIcon TransferFuncWidgetPlugin::icon() const {
	return QIcon();
}

QString TransferFuncWidgetPlugin::codeTemplate() const {
	return "";
}

QString TransferFuncWidgetPlugin::domXml() const {
	return "<widget class=\"TransferFuncWidget\" name=\"TransferFuncWidget\">\n"
               " <property name=\"geometry\">\n"
               "  <rect>\n"
               "   <x>0</x>\n"
               "   <y>0</y>\n"
               "   <width>250</width>\n"
               "   <height>20</height>\n"
               "  </rect>\n"
               " </property>\n"
               " <property name=\"toolTip\" >\n"
               "  <string>The current time</string>\n"
               " </property>\n"
               " <property name=\"whatsThis\" >\n"
               "  <string>The analog clock widget displays "
               "the current time.</string>\n"
               " </property>\n"
               "</widget>\n";

}

QString TransferFuncWidgetPlugin::group() const {
	return "Voreen Widgets";
}

QString TransferFuncWidgetPlugin::includeFile() const {
	return "voreen/qt/transferfuncwidget.h";
}

QString TransferFuncWidgetPlugin::name() const {
	return "TransferFuncWidget";
}

QString TransferFuncWidgetPlugin::toolTip() const {
	return "";
}

QString TransferFuncWidgetPlugin::whatsThis() const {
	return "";
}

QWidget *TransferFuncWidgetPlugin::createWidget(QWidget *parent) {
	return new TransferFuncWidget(parent);
}

void TransferFuncWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
	if(initialized) {
		return;
	}
	initialized = true;
}

Q_EXPORT_PLUGIN2(transferfuncwidgetplugin, TransferFuncWidgetPlugin)
} // namespace voreen

