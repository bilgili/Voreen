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

#ifndef VRN_AXIALSLICEPROXYGEOMETRY_H
#define VRN_AXIALSLICEPROXYGEOMETRY_H

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

namespace voreen {

/**
 * Creates a single slice along one of the main axes. It is possible to set the thickness,
 * and the starting point of the slice (both in percentage of the complete volume).
 *
 * - #alignment_ and #alignmentProp_ determine along which axis the slice will be generated.
 * - #begin_ sets the front end of the slice (in percentage of the volume).
 * - #thickness_ sets the thickness of the slice (also in percentage).
 *
 * \sa ProxyGeometry
 */
class AxialSliceProxyGeometry : public ProxyGeometry {
public:
    AxialSliceProxyGeometry();
    virtual ~AxialSliceProxyGeometry();

    virtual std::string getCategory() const { return "ProxyGeometry"; }
    virtual std::string getClassName() const { return "AxialSliceProxyGeometry"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    static const std::string setAxis_;
    static const std::string setBegin_;
    static const std::string setThickness_;

    virtual void render();

    /// Alignment of the slices
    enum SliceAlignment {
        SAGITTAL = 0,       /**< view from the volume's right to left (negative x-axis) */
        AXIAL = 1,          /**< view from the volume's front to back (negative z-axis) */
        CORONAL = 2         /**< view from the volume's top to bottom (negative y-axis) */
    };

protected:
    virtual void revalidateGeometry();

    virtual void setAxis();
    virtual void setBegin();
    virtual void setThickness();

    GLuint dl_;
    IntOptionProperty alignmentProp_;
    IntProperty begin_;
    IntProperty thickness_;
};

} // namespace

#endif // VRN_AXIALSLICEPROXYGEOMETRY_H
