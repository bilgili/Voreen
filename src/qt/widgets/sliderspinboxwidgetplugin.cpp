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

#include "voreen/qt/sliderspinboxwidget.h"
#include "sliderspinboxwidgetplugin.h"

#include <QtPlugin>

SliderSpinBoxWidgetPlugin::SliderSpinBoxWidgetPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

bool SliderSpinBoxWidgetPlugin::isContainer() const {
	return false;
}

bool SliderSpinBoxWidgetPlugin::isInitialized() const {
	return initialized;
}

QIcon SliderSpinBoxWidgetPlugin::icon() const {
	return QIcon();
}

QString SliderSpinBoxWidgetPlugin::codeTemplate() const {
	return "";
}

QString SliderSpinBoxWidgetPlugin::domXml() const {
	return "<widget class=\"SliderSpinBoxWidget\" name=\"SliderSpinBoxWidget\">\n"
               " <property name=\"geometry\">\n"
               "  <rect>\n"
               "   <x>0</x>\n"
               "   <y>0</y>\n"
               "   <width>160</width>\n"
               "   <height>22</height>\n"
               "  </rect>\n"
               " </property>\n"
			   " <property name=\"sizePolicy\" >\n"
			   "  <sizepolicy>\n"
			   "   <hsizetype>7</hsizetype>\n"
			   "   <vsizetype>0</vsizetype>\n"
			   "   <horstretch>0</horstretch>\n"
			   "   <verstretch>0</verstretch>\n"
			   "  </sizepolicy>\n"
			   " </property>\n"
               " <property name=\"toolTip\" >\n"
               "  <string></string>\n"
               " </property>\n"
               "</widget>\n";

}

QString SliderSpinBoxWidgetPlugin::group() const {
	return "Voreen Widgets";
}

QString SliderSpinBoxWidgetPlugin::includeFile() const {
	return "voreen/qt/sliderspinboxwidget.h";
}

QString SliderSpinBoxWidgetPlugin::name() const {
	return "SliderSpinBoxWidget";
}

QString SliderSpinBoxWidgetPlugin::toolTip() const {
	return "";
}

QString SliderSpinBoxWidgetPlugin::whatsThis() const {
	return "";
}

QWidget *SliderSpinBoxWidgetPlugin::createWidget(QWidget *parent) {
	return new SliderSpinBoxWidget(parent);
}

void SliderSpinBoxWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
	if(initialized) {
		return;
	}
	initialized = true;
}

Q_EXPORT_PLUGIN2(sliderspinboxwidgetplugin, SliderSpinBoxWidgetPlugin)
} // namespace voreen

