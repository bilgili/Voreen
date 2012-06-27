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

#ifndef VRN_CUBECUTPROXYGEOMETRY_H
#define VRN_CUBECUTPROXYGEOMETRY_H

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

namespace voreen {

/**
 * Provides a simple cube proxy. The resulting geometry depends
 * on the ratio of the values in dim.
 */
class CubeCutProxyGeometry : public ProxyGeometry {
public:
    CubeCutProxyGeometry();
    virtual ~CubeCutProxyGeometry();

    virtual const Identifier getClassName() const { return "ProxyGeometry.CubeCutProxyGeometry"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new CubeCutProxyGeometry(); }

    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

protected:
    virtual void render();
    void revalidateCubeGeometry();
    void renderCubeWithCutting();
    void renderCuttedCube();
    void onSettingsChange() { needsBuild_ = true; }

    /// clipping cube
    BoolProp cutCube_;
    FloatVec3Prop cubeSize_;

    GLuint dl_;
};

} // namespace

#endif // VRN_CUBECUTPROXYGEOMETRY_H
