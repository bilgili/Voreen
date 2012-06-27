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

#ifndef VRN_VOLUMERENDERER_H
#define VRN_VOLUMERENDERER_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

/**
 * All classes which render volumes should inherit from this class.
 * This class knows of the current dataset.
 */
class VolumeRenderer : public Processor {
public:
    VolumeRenderer(tgt::Camera* camera = 0, TextureContainer* tc = 0);

    /**
     * Handles:
     * - Identifier::setCurrentDataset
     * - Identifier::setVolumeContainer
     */
    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

    VolumeHandle* getVolumeHandle();
    const VolumeHandle* getVolumeHandle() const;

    /** Sets the given VolumeHandle* as the currently active VolumeHandle.
     * From the VolumeHandle, at least a Volume* can be obtained.
     * Further hardware specializations may be known, depending on the applications
     * settings.
     * Overwrite this method if you need to have additional Tasks to be done
     * on changes of the volume.
     * The EntryExitPoints for instance have to adjust transformation matrices on
     * changes of the current volume.
     */
    virtual void setVolumeHandle(VolumeHandle* const handle);

    /// returns view matrix of the camera, or identity matrix if no camera exists
    tgt::mat4 getModelViewMatrix() const;

    /**
     * Returns the current transfer function; if multiple transferfunctions exist,
     * the first one should be returned.
     * /return The first transfer function
     */
    virtual TransFunc* getTransFunc();

    /**
     * Returns the i-th transfer function of the volumerenderer.
     * getTransFunc(0) should always be equal to getTransFunc()
     * \param i The number of the transfer function
     * \return A pointer to the transfer function or null if i > count(transfer functions)
     */
    virtual TransFunc* getTransFunc(int i);

    float getLowerThreshold() const;
    float getUpperThreshold() const;
  
    // identifiers commonly used in volumeprocessors
    static const Identifier setLowerThreshold_;
    static const Identifier setUpperThreshold_;
    static const Identifier setIsoValue_;
    static const Identifier setTransFunc_;
    static const Identifier setTransFunc2_;
  	static const Identifier setTransFunc3_;

protected:
    /**
     * This struct contains information about a volume. It is exclusively used
     * as parameter type for the bindVolumes() function.
     */
    struct VolumeStruct {
        const VolumeGL* volume_;                        ///< the volume whose texture is to bound
        Identifier textureUnitIdent_;                   ///< specifies the texture unit the volume texture is bound to
        std::string samplerIdentifier_;                 ///< the sampler by which the
                                                        ///  volume is accessed in the shader
        std::string volumeParametersIdentifier_;        ///< the identifier of the volume parameter struct
                                                        ///  for this volume in the shader

        VolumeStruct();                          ///< standard constructor
        VolumeStruct(                            ///< constructor provided for convenience
            const VolumeGL* volume,
            Identifier textureUnitIdent,
            std::string samplerIdentifier,
            std::string volumeParametersIdentifier);
    };

    virtual std::string generateHeader();

    virtual void setGlobalShaderParameters(tgt::Shader* shader);

    /**
     * This function binds the volume textures used by the volume renderer and passes the
     * corrensponding samplers (texture units) to the shader.
     * \note This function also sets additional meta data like the volume's dimensions and spacing. 
     *       For this reason, every volume renderer that uses a volume texture must call this function 
     *       for all volumes accessed during a rendering pass. Do not bypass this function by binding
     *       volume textures directly unless you know exactly what you are doing!
     * @param shader the shader that performs the volume rendering
     * @param volumes vector of the volumes to bind
     */
    virtual void bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes);

    FloatProp lowerTH_; ///< The property that can be used to change the lower Treshold
    FloatProp upperTH_; ///< The property that can be used to change the upper Treshold

    VolumeHandle* currentVolumeHandle_;
};

} // namespace voreen

#endif // VRN_VOLUMERENDERER_H
