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

#ifndef VRN_GENERICFRAGMENTPP_H
#define VRN_GENERICFRAGMENTPP_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/postprocessor.h"

namespace voreen {

/**
 * Base class for fragment shader based post processors.
 *
 * Normally in a derived class you have only to call the ctor with the
 * appropriate shader name und overwrite the render method.
 */
class GenericFragment : public PostProcessor {
public:
    /**
     * Constructor.
     *
     * @param shaderFilename The filename of the shader that will be used.
     */
    GenericFragment(const std::string& shaderFilename = "");
    virtual ~GenericFragment();

    virtual const Identifier getClassName() const { return "PostProcessor.GenericFragment"; }   
    virtual int initializeGL();

protected:
    void compileShader();

    /**
     * Load the needed shader.
     */
    virtual void compile();

    tgt::Shader* program_;

    std::string shaderFilename_;
    bool needRecompileShader_;

    static const Identifier shadeTexUnit_;
    static const Identifier depthTexUnit_;
};

} // namespace voreen

#endif //VRN_GENERICFRAGMENTPP_H
