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

/*
 * Precompiled Header File for voreenapp
 *
 * The precompiled header must contain code which is *stable* and *static*
 * throughout the project.
 * 
 */

/* Add C includes here */

#ifdef VRN_WITH_PYTHON
/* include this at very first */
#include <Python.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>

#ifndef __APPLE__
	#include <GL/gl.h>
	#include <GL/glu.h>
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#endif

#ifdef VRN_WITH_DEVIL
#include <IL/il.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif


#if defined __cplusplus
/* Add C++ includes here */

#include <iostream>
#include <string>
#include <vector>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGLWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QThread>
#include <QToolBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "tgt/matrix.h"
#include "tgt/quaternion.h"
#include "tgt/shadermanager.h"
#include "tgt/texturemanager.h"
#include "tgt/types.h"
#include "tgt/vector.h"

#endif
