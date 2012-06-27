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

#ifndef VRN_RENDERPORT_H
#define VRN_RENDERPORT_H

#include "tgt/shadermanager.h"
#include "voreen/core/vis/processors/ports/genericport.h"

namespace tgt {
    class FramebufferObject;
}

namespace voreen {

class RenderTarget;

class RenderPort : public GenericPort<RenderTarget> {
    friend class RenderProcessor;
public:
    explicit RenderPort(PortDirection direction, const std::string& name, bool allowMultipleConnections = false,
                        Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);
    ~RenderPort();

    void initialize();

    void sizeOriginChanged(void* so);
    void* getSizeOrigin() const;
    void resize(tgt::ivec2 newsize);

    virtual bool connect(Port* inport);
    virtual bool testConnectivity(const Port* inport) const;
    virtual void disconnect(Port* other);

    bool hasValidResult() const;
    void validateResult();
    void invalidateResult();

    /**
     * Returns true, if the port is connected, has a RenderTarget object and (if this is an inport) a valid rendering.
     */
    virtual bool isReady() const;

    tgt::ivec2 getSize() const;
    /**
     * @brief Activate the RenderTarget in this port.
     *
     * @param debugLabel An additional description shown in the STC widget.
     */
    void activateTarget(const std::string& debugLabel = "");
    void deactivateTarget();

    /**
     * @brief Set a TEXTURE_PARAMETERS struct in glsl.
     *
     * @param uniform The name of the uniform.
     */
    void setTextureParameters(tgt::Shader* shader, std::string uniform);
    /**
     * @brief Bind the color texture of this rendertarget to the currently active textureunit.
     */
    void bindColorTexture() const;
    /**
     * @brief Bind the color texture of this rendertarget to a specific textureunit.
     *
     * @param texUnit The textureunit to activate beforce binding.
     */
    void bindColorTexture(GLint texUnit) const;

    /**
     * @brief Bind the depth texture of this rendertarget to the currently active textureunit.
     */
    void bindDepthTexture() const;

    /**
     * @brief Bind the depth texture of this rendertarget to a specific textureunit.
     *
     * @param texUnit The texture unit to activate beforce binding.
     */
    void bindDepthTexture(GLint texUnit) const;

    ///Equivalent to bindColorTexture(colorUnit); bindDepthTexture(depthUnit);
    void bindTextures(GLint colorUnit, GLint depthUnit) const;

    tgt::Texture* getColorTexture() const;
    tgt::Texture* getDepthTexture() const;

    /**
     * Writes the currently stored rendering to an image file.
     *
     * @note This function requires Voreen to be built with DevIL support.
     *
     * @param filename the filename of the output file. Must have an
     *      extension known by the DevIL library. *.jpg and *.png
     *      should work fine.
     */
    void saveToImage(const std::string &filename);

protected:
    virtual void setProcessor(Processor* p);
private:
    bool validResult_;
    tgt::ivec2 size_; //neccessary for inports
    void* sizeOrigin_;

    static const std::string loggerCat_; ///< category used in logging
};

/**
 * @brief This class groups RenderPorts to allow easy rendering to multiple targets.
 */
class PortGroup {
public:
    /**
     * @param ignoreConnectivity If this is true all ports are attached to the FBO, otherwise only the connected ones.
     */
    PortGroup(bool ignoreConnectivity = false);

    ///Initializes the FBO
    void initialize();

    ///Add a port to the group.
    void addPort(RenderPort* rp);
    void addPort(RenderPort& rp);
    //void removePort(RenderPort* rp);

    /**
     * @brief Start rendering to all attached ports.
     *
     * @param debugLabel @see RenderPort::activateTarget
     */
    void activateTargets(const std::string& debugLabel = "");
    ///Resize all attached ports.
    void resize(tgt::ivec2 newsize);

    /**
     * @brief Defines OP0, OP1, ... OPn to adress targets in shader.
     *
     * Example:
     * CPP:
     * addPort(p1); addPort(p2); addPort(p3);
     *
     * glsl:
     * gl_FragData[OP0] = vec4(1.0); //write to p1
     * gl_FragData[OP1] = vec4(1.0); //write to p2
     * gl_FragData[OP2] = vec4(1.0); //write to p3
     *
     * using the following code would not work if some ports are disconnected and ignoreConnectivity is false.
     * gl_FragData[0] = vec4(1.0); //write to p1
     * //do not write to p2 because it is disconnected
     * gl_FragData[2] = vec4(1.0); //write to p3 (would not work)
     *
     * The defines can also be used to test if a port is connected:
     * #ifdef OP0
     *    gl_FragData[OP0] = result;
     * #endif
     *
     * @return A string containing the defines.
     */
    std::string generateHeader();

    /**
     * Re-attach all rendertargets to the FBO.
     * If ignoreConnectivity is true only connected ports are attached.
     */
    void reattachTargets();
protected:

    std::vector<RenderPort*> ports_;
    tgt::FramebufferObject* fbo_;
    bool ignoreConnectivity_;
};


} // namespace

#endif // VRN_RENDERPORT_H
