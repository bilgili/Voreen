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

#ifndef VRN_PROXYGEOMETRY_H
#define VRN_PROXYGEOMETRY_H

#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/volume/volumeatomic.h"

namespace voreen {

/**
 * Abstract base class for all proxy geometries. Derived classes create
 * their proxy geometry.
 * */
class ProxyGeometry : public VolumeRenderer {
public:
    ProxyGeometry();
    virtual ~ProxyGeometry() { }
    virtual const Identifier getClassName() const {return "ProxyGeometry.ProxyGeometry";}
    virtual tgt::vec3 getVolumeSize();

    //virtual void setVolumeHandle(VolumeHandle* const handle);

    /**
     * Should called when OpenGL is initialized.
     */
    virtual int initializeGL();

    static const Identifier setUseClipping_;
    static const Identifier setLeftClipPlane_;
    static const Identifier setRightClipPlane_;
    static const Identifier setTopClipPlane_;
    static const Identifier setBottomClipPlane_;
    static const Identifier setFrontClipPlane_;
    static const Identifier setBackClipPlane_;
    static const Identifier resetClipPlanes_;
    static const Identifier getVolumeSize_;

    virtual void render() {}

    virtual void process(LocalPortMapping* portMapping);
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

protected:
    /**
     * Transforms the given vertex into texture coordinates used for entry/exit point
     * calculation.
     */
    virtual tgt::vec3 vertexToTexCoord(const tgt::vec3& vertex) const;

    bool needsBuild_;

    /**
     * Contains the size of the volume along each axis. The longest axis will be from -0.5 to 0.5
     * and each other axis is proportionally shorter*
     */
    tgt::vec3 volumeSize_;

    /**
     * Contains the center of the proxygeometry around which the volume will be created. Normally,
     * a volume will have lower left front (and upper right back) value of
     * <code>volumeCenter_ +/- volumeSize_/2.f</code>.
     *
     * This is defaulted to vec3::zero, so it doesn't invoke conflicts somewhere.
     */
    tgt::vec3 volumeCenter_;
    Volume* volume_;

    /** Determines wether the transformation matrix assigned to a dataset
     *  is considered when rendering the proxygeometry.
     */
    BoolProp useDatasetTransformationMatrix_;
    tgt::mat4 datasetTransformationMatrix_;

};

} // namespace voreen

#endif // VRN_PROXYGEOMETRY_H
