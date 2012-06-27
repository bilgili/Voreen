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

#include "voreen/core/version.h"

#ifdef VRN_WITH_SVNVERSION
  #include "voreen/core/svnversion.h"
#endif

#include "tgt/logmanager.h"
#include <sstream>

namespace voreen {

const std::string VoreenVersion::getCompilerVersion() {
    std::stringstream ver;
#if defined(WIN32) && defined(_MSC_VER)
    #if _MSC_VER == 1310
        ver << "Microsoft Visual C++ .NET 2003");
    #elif _MSC_VER == 1400
        ver << "Microsoft Visual C++ 2005";
    #elif _MSC_VER == 1500
        ver << "Microsoft Visual C++ 2008";
    #else
        ver << "Unknown Microsoft Visual C++ (_MSC_VER=" << _MSC_VER << ")";
    #endif
#elif defined(__GNUC__)
    ver << "gcc " << __VERSION__;
#else
    ver << "Unknown compiler";
#endif

    // detect CPU architecture the application was compiled with
#if defined(__amd64__) || defined(__x86_64__) || defined(_M_X64)
    ver << " [x86_64]";
#elif defined(__i386__) || defined(_M_IX86) || defined(_X86_) || defined(__INTEL__)
    ver << " [x86]";
#elif defined(__powerpc__)
    ver << " [PowerPC]";
#else
    ver << " [unknown CPU architecture]";
#endif

    return ver.str();
}

const std::string VoreenVersion::getVersion() {
    return "2.0";
}

const std::string VoreenVersion::getRevision() {
#if defined(VRN_WITH_SVNVERSION)
    return VRN_SVN_REVISION;
#elif defined(VRN_SNAPSHOT)
    return "snapshot-29";
#else
    return "";
#endif
}

void VoreenVersion::logAll(const std::string& loggerCat) {
    LINFOC(loggerCat, "Version: " << getVersion());
    if (!getRevision().empty())
        LINFOC(loggerCat, "SVN revision: " << getRevision());

    LINFOC(loggerCat, "Compiler: " << getCompilerVersion());
}

} // namespace voreen
