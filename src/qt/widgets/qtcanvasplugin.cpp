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

#include "tgt/qt/qtcanvas.h"
#include "qtcanvasplugin.h"

#include <QtPlugin>

QtCanvasPlugin::QtCanvasPlugin(QObject *parent) : QObject(parent) {
	initialized = false;
}

bool QtCanvasPlugin::isContainer() const {
	return false;
}

bool QtCanvasPlugin::isInitialized() const {
	return initialized;
}

QIcon QtCanvasPlugin::icon() const {
	return QIcon();
}

QString QtCanvasPlugin::codeTemplate() const {
	return "";
}

QString QtCanvasPlugin::domXml() const {
	return "<widget class=\"QtGLCanvas\" name=\"QtGLCanvas\">\n"
               " <property name=\"geometry\">\n"
               "  <rect>\n"
               "   <x>0</x>\n"
               "   <y>0</y>\n"
               "   <width>512</width>\n"
               "   <height>512</height>\n"
               "  </rect>\n"
               " </property>\n"
			   "</widget>\n";
}

QString QtCanvasPlugin::group() const {
	return "Voreen Widgets";
}

QString QtCanvasPlugin::includeFile() const {
	return "tgt/qt/qtcanvas.h";
}

QString QtCanvasPlugin::name() const {
	return "QTCanvas";
}

QString QtCanvasPlugin::toolTip() const {
	return "";
}

QString QtCanvasPlugin::whatsThis() const {
	return "";
}

QWidget *QtCanvasPlugin::createWidget(QWidget *parent) {
	return new QtGLCanvas(parent);
}

void QtCanvasPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
	if(initialized) {
		return;
	}
	initialized = true;
}

Q_EXPORT_PLUGIN2(QtCanvasPlugin, QtCanvasPlugin)
} // namespace voreen

