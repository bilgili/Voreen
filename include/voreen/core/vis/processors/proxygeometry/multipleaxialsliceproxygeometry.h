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

#ifndef VRN_MULTIPLEAXIALSLICEPROXYGEOMETRY_H
#define VRN_MULTIPLEAXIALSLICEPROXYGEOMETRY_H

#include "voreen/core/vis/processors/proxygeometry/axialsliceproxygeometry.h"

namespace voreen {

/**
 * This class manages multiple renderings of \sa AxialSliceProxyGeometry in order to create more
 * slice proxygeometrys alongside each other. It inherites the properties from AxialSliceProxyGeometry
 * and uses them to set the same parameters.
 *
 * \sa axis_ Determines, along which axis the slices will be generated
 * \sa begin_ Sets the front end of the first slice (in percentage of the volume)
 * \sa thickness_ Sets the thickness of each slice (also in percentage)
 * \sa nSlicesPerRow_ The number of slices within each row
 * \sa nSlicesPerCol_ The number of slices within each column
 * \sa Proxygeometry, AxialSliceProxyGeometry
 */
class MultipleAxialSliceProxyGeometry : public AxialSliceProxyGeometry {
public:
    MultipleAxialSliceProxyGeometry();

    virtual const Identifier getClassName() const { return "ProxyGeometry.MultipleAxialSliceProxyGeometry"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new MultipleAxialSliceProxyGeometry(); }

    static const Identifier setNSlicesPerRow_;
    static const Identifier setNSlicesPerCol_;

    virtual void render();

protected:
    // The base class uses the same two "onChange"-methods, but we want to use our own
    virtual void setBegin();
    virtual void setThickness();
    void setNSlicesPerRow();
    void setNSlicesPerCol();

    // Renders the current slice with the preedefined size and center (only used internally)
    void build();

    IntProp nSlicesPerRow_;
    IntProp nSlicesPerCol_;

    // This flag is necessary, because we don't want to trigger an onChange-method for each change to
    // the begin_ - property
    bool isInBuildMethod;
};

} // namespace

#endif // VRN_MULTIPLEAXIALSLICEPROXYGEOMETRY_H
