/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include <qapplication.h>
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtcanvas.h"

namespace tgt {

QtApplication::QtApplication(int& argc, char** argv)
    : GUIApplication(argc, argv)
{
    qapplication_ = new QApplication(*argc_, argv_);
}

QtApplication::~QtApplication(){
    deinitGL();
//FIXME: Handled by Qt or not?
//    delete qapplication_;
    deinit();
}

int QtApplication::init(InitFeature::Features featureset) {
    return GUIApplication::init(featureset);
}

int QtApplication::run() {
    if (hasRun) {
        std::cout << "Failure: QtApplication::run() should be called only once.\n";
        return -1;
    }
    hasRun = true;
    return qapplication_->exec();
}

void QtApplication::quit(int status) {
    qapplication_->exit(status);
}

}
