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

#ifndef VRN_DEPTHPEELINGENTRYEXITPOINTS_H
#define VRN_DEPTHPEELINGENTRYEXITPOINTS_H

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/opengl/texunitmapper.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Calculates the entry and exit points for Multi-volume raycasting using
 * Depth Peeling.
 */
class DepthPeelingEntryExitPoints  : public VolumeRenderer {
public:

    DepthPeelingEntryExitPoints();
    virtual ~DepthPeelingEntryExitPoints();

    virtual std::string getCategory() const { return "EntryExitPoints"; }
    virtual std::string getClassName() const { return "DepthPeelingEntryExitPoints"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor* create() const { return new DepthPeelingEntryExitPoints(); }

    virtual void initialize() throw (VoreenException);

    /*
     * Renders Proxy Geometries of the volumes in the scene and produces entry/exit params in a
     * depth sorted order.
     */
    virtual void process();

    /**
     * Lookup into the map vector to get the
     * unique ID from the object ID.
     */
    virtual int lookup(int objectid);

    /**
     * Create the lookup map to get the
     * unique ID from the object ID.
     */
    virtual void createMap(std::vector<ProxyGeometry*> portData);

    // Convert from int to string
    virtual std::string itos(int i);

    int getNumLayers();
    std::vector<int> getMap();

protected:
    tgt::Shader* shaderProgram_;

    CameraProperty camera_;

    static const std::string entryPointsTexUnit_;
    static const std::string entryPointsDepthTexUnit_;
    static const std::string exitPointsTexUnit_;

    // vector containing the object IDs of all the volumes present in a scene
    std::vector<int> volumeMap_;

    // Bias Probe that applies z-bias on the scene inorder to solve the Z-fighting problem
    FloatProperty biasProp_;

    int layersNumber;
    static const std::string loggerCat_;
    GenericCoProcessorPort<ProxyGeometry> pgPort_;
    GenericCoProcessorPort<DepthPeelingEntryExitPoints> outPort_;

    RenderPort outport1_;
    RenderPort outport2_;
    RenderPort outport3_;
    RenderPort outport4_;
    RenderPort outport5_;
    RenderPort outport6_;
    RenderPort initialImagePort_;
};

} // namespace voreen

#endif //VRN_ENTRYEXITPOINTS_H
