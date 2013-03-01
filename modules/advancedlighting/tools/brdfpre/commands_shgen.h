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

#ifndef VRN_COMMANDS_SHGEN_H
#define VRN_COMMANDS_SHGEN_H

#include "voreen/core/utils/cmdparser/command.h"
#include "modules/sphericalharmonics/include/shbrdf.h"
#include "modules/sphericalharmonics/include/shlightfunc.h"

namespace voreen {

class CommandSHGenBrdfMerl : public Command {

public:
    CommandSHGenBrdfMerl();
    virtual ~CommandSHGenBrdfMerl() {
        delete brdf_;
    }

    bool execute(const std::vector<std::string>& parameters);
    virtual bool checkParameters(const std::vector<std::string>& parameters);

private:

    SHBrdfMerl* brdf_;
};

class CommandSHGenBrdfPhong : public Command {

public:
    CommandSHGenBrdfPhong();
    virtual ~CommandSHGenBrdfPhong() {
        delete brdf_;
    }

    bool execute(const std::vector<std::string>& parameters);
    virtual bool checkParameters(const std::vector<std::string>& parameters);

private:

    SHBrdfPhong* brdf_;
};

class CommandSHGenLightProbe : public Command {

public:
    CommandSHGenLightProbe();
    virtual ~CommandSHGenLightProbe() {
        delete lp_;
    }

    bool execute(const std::vector<std::string>& parameters);
    virtual bool checkParameters(const std::vector<std::string>& parameters);

private:

    SHLightProbe* lp_;
};

class CommandSHGenLightScalar: public Command {

public:
    CommandSHGenLightScalar();
    virtual ~CommandSHGenLightScalar() {
        delete ls_;
    }

    bool execute(const std::vector<std::string>& parameters);
    virtual bool checkParameters(const std::vector<std::string>& parameters);

private:

    SHScalarFunc* ls_;
};

}   //namespace voreen

#endif //VRN_COMMANDS_SHGEN_H
