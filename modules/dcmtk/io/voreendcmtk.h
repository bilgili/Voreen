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

/**
 * Include this file *before* including any dcmtk-dependant header, e.g. dicomvolumereader.h.
 */

#ifdef WIN32

#define HAVE_SSTREAM_H 1
#define HAVE_CLASS_TEMPLATE 1
#define HAVE_FUNCTION_TEMPLATE 1

#pragma warning(disable:4005) //< 'macro redefinition' (some availability macros are also defined in Python headers)
#include <dcmtk/config/osconfig.h>
#pragma warning(default:4005)

#else

#define HAVE_CONFIG_H

// prevent warnings
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef HAVE_STAT

#include <dcmtk/config/osconfig.h>
#endif

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimgle/didocu.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdatset.h>
