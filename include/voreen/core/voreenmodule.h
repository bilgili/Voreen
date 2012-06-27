/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#ifndef VRN_VOREENMODULE_H
#define VRN_VOREENMODULE_H

#include <string>
#include <vector>
#include "voreen/core/utils/exception.h"

namespace voreen {

class VoreenApplication;
class Processor;
class SerializableFactory;
class VolumeReader;
class VolumeWriter;

/**
 * Abstract base class for module classes.
 *
 * A module class is responsible for registering the
 * module's resources at runtime:
 *  - Processors
 *  - VolumeReaders and VolumeWriters
 *  - SerializableFactories
 *  - Shader search path
 *
 * The registration is to be done in the constructor.
 *
 * @note The derived module class has to be instantiated in:
 *       include/voreen/modules/moduleregistration.h
 *
 * @see http://www.voreen.org/402-Adding-a-Module.html
 *
 */
class VoreenModule {

    friend class VoreenApplication;

public:
    /**
     * Instantiate and register the module's classes
     * in the derived class's constructor.
     */
    VoreenModule();

    /**
     * Deletes the registered objects.
     */
    virtual ~VoreenModule();

    /**
     * Module documentation that is intended
     * to be presented to the user.
     */
    virtual std::string getDescription() const = 0;

    /**
     * Returns the module's name.
     *
     * @see setName
     */
    std::string getName() const;

    /**
     * Returns the module's processors.
     *
     * @see addProcessor
     */
    const std::vector<Processor*>& getProcessors() const;

    /**
     * Returns the module's VolumeReaders.
     *
     * @see addVolumeReader
     */
    const std::vector<VolumeReader*>& getVolumeReaders() const;

    /**
     * Returns the module's VolumeWriters.
     *
     * @see addVolumeWriter
     */
    const std::vector<VolumeWriter*>& getVolumeWriters() const;

    /**
     * Returns the module's SerializerFactories.
     *
     * @see addSerializerFactory
     */
    const std::vector<SerializableFactory*>& getSerializerFactories() const;

    /**
     * Returns the GLSL shader search used by the module.
     *
     * @see addShaderPath
     */
    const std::vector<std::string>& getShaderPaths() const;

protected:
    /**
     * OpenGL-dependent or time-consuming initializations
     * should be performed here. However, it is usually
     * not necessary to override this function.
     *
     * @throw VoreenException to indicate an initialization failure
     */
    virtual void initialize()
        throw (VoreenException);

    /**
     * OpenGL-dependent or time-consuming deinitializations
     * should be performed here. However, it is usually
     * not necessary to override this function.
     *
     * @throw VoreenException to indicate a deinitialization failure
     */
    virtual void deinitialize()
        throw (VoreenException);

    /**
     * Specifies the module's name. To be called in the
     * derived class's constructor.
     *
     * @note Setting a name is mandatory.
     */
    void setName(const std::string& name);

    /**
     * Registers the passed processor. To be called in the
     * derived class's constructor.
     */
    void addProcessor(Processor* processor);

    /**
     * Registers the passed VolumeReader. To be called in the
     * derived class's constructor.
     */
    void addVolumeReader(VolumeReader* reader);

    /**
     * Registers the passed VolumeWriter. To be called in the
     * derived class's constructor.
     */
    void addVolumeWriter(VolumeWriter* writer);

    /**
     * Registers the passed SerializableFactory. To be called in the
     * derived class's constructor.
     */
    void addSerializerFactory(SerializableFactory* factory);

    /**
     * Adds the passed directory to the shader search path.
     * To be called in the derived class's constructor.
     *
     * @note Use getModulesPath() for determining the module's directory.
     */
    void addShaderPath(const std::string& path);

    /**
     * Constructs an absolute path consisting of the modules source directory
     * (typically <VOREEN_ROOT>/src/modules) and the given path suffix.
     *
     * @note You have to pass the name of the module's directory,
     *  e.g. getModulesPath("mymodule") yields <VOREEN_ROOT>/src/modules/mymodule
     */
    std::string getModulesPath(const std::string& suffix = "") const;

    static const std::string loggerCat_;

private:
    std::string name_;
    std::vector<Processor*> processors_;
    std::vector<VolumeReader*> volumeReaders_;
    std::vector<VolumeWriter*> volumeWriters_;
    std::vector<SerializableFactory*> serializerFactories_;
    std::vector<std::string> shaderPaths_;
};

} // namespace

#endif // VRN_VOREENMODULE_H
