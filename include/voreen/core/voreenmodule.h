/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_VOREENMODULE_H
#define VRN_VOREENMODULE_H

#include "voreen/core/voreencoreapi.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/utils/exception.h"

#include <string>
#include <vector>

namespace voreen {

class VoreenApplication;

class Processor;
class ProcessorWidget;
class ProcessorWidgetFactory;

class Property;
class PropertyWidget;
class PropertyWidgetFactory;
class LinkEvaluatorFactory;

class Port;
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
 */
class VRN_CORE_API VoreenModule : public PropertyOwner {

    friend class VoreenApplication;

public:
    /**
     * Instantiate and register the module's classes
     * in the derived class's constructor.
     *
     * @param modulePath absolute path to the module directory
     */
    VoreenModule(const std::string& modulePath);

    /**
     * Deletes the registered objects.
     */
    virtual ~VoreenModule();

    /**
     * Returns the module's name.
     *
     * @see setName
     */
    std::string getName() const;

    /**
     * Returns the name of the module's base directory (without parent dir).
     */
    std::string getDirName() const;

    /**
     * Returns true, if the module has been successfully initialized,
     * and has not yet been deinitialized.
     */
    bool isInitialized() const;

    /**
     * Returns true, if the module has been successfully OpenGL initialized
     * and has not yet been deinitialized.
     */
    bool isInitializedGL() const;

    /**
     * Module documentation that is intended
     * to be presented to the user.
     */
    virtual std::string getDescription() const = 0;

    /// Sets the description
    void setDescription(const std::string& description) const;

    /**
     * Constructs an absolute path consisting of the module's base directory
     * and the given path suffix.
     */
    std::string getModulePath(const std::string& suffix = "") const;

    /**
     * Returns the module's processors.
     *
     * @see registerProcessor
     */
    const std::vector<Processor*>& getProcessors() const;

    /**
     * Returns the module's properties.
     *
     * @see registerProperty
     */
    const std::vector<Property*>& getRegisteredProperties() const;

    /**
     * Returns the module's VolumeReaders.
     *
     * @see registerVolumeReader
     */
    const std::vector<VolumeReader*>& getVolumeReaders() const;

    /**
     * Returns the module's VolumeWriters.
     *
     * @see registerVolumeWriter
     */
    const std::vector<VolumeWriter*>& getVolumeWriters() const;

    /**
     * Returns the module's SerializerFactories.
     *
     * @see registerSerializerFactory
     */
    const std::vector<SerializableFactory*>& getSerializerFactories() const;

    /**
     * Returns the module's ProcessorWidgetFactories.
     *
     * @see registerProcessorWidgetFactory
     */
    const std::vector<ProcessorWidgetFactory*>& getProcessorWidgetFactories() const;

    /**
     * Returns the module's PropertyWidgetFactories.
     *
     * @see registerPropertyWidgetFactory
     */
    const std::vector<PropertyWidgetFactory*>& getPropertyWidgetFactories() const;

    /**
     * Returns the module's LinkEvaluatorFactories.
     *
     * @see registerLinkEvaluatorFactory
     */
    const std::vector<LinkEvaluatorFactory*>& getLinkEvaluatorFactories() const;

    /**
     * Returns the GLSL shader search used by the module.
     *
     * @see registerShaderPath
     */
    const std::vector<std::string>& getShaderPaths() const;

    // --- Documentation ---

    /**
     * Returns the description of the passed \sa Processor.
     * \param className The processor class whose description is wanted
     * \return The description of the processor
     */
    std::string getDocumentationDescription(const std::string& className) const;

    /**
     * Returns the descriptions for the \sa Ports of the passed class. The return will be of the form
     * <portname,description> and the vector will contain only those ports which actually do have a
     * description.
     * \param className The class name of the \sa Processor for which the documentation is wanted
     */
    std::vector<std::pair<std::string, std::string> > getDocumentationPorts(const std::string& className) const;

    /**
     * Returns the description for a specific \sa Port of the given \sa Processor. The return value will be the
     * description of the port.
     * \param className The class name of the processor which has the port
     * \param portName The name of the port
     * \return The description of the port
     */
    std::string getDocumentationPort(const std::string& className, const std::string& portName) const;

    /**
     * Returns the descriptions for the \sa Properties of the passed class. The return will be of the form
     * <propertyname,description> and the vector will contain only those properties which actually do have a
     * description.
     * \param className The class name of the \sa Processor for which the documentation is wanted
     */
    std::vector<std::pair<std::string, std::string> > getDocumentationProperties(const std::string& className) const;

    /**
     * Returns the description for a specific \sa Property of the given \sa Processor. The return value will be the
     * description of the Property.
     * \param className The class name of the processor which has the property
     * \param propertyID The ID of the property
     * \return The description of the property
     */
    std::string getDocumentationProperty(const std::string& className, const std::string& propertyID) const;

protected:
    /**
     * Time-consuming initializations that do not access
     * the OpenGL API should be performed here. Registration
     * of resources, however, should be done in the constructor.
     *
     * @throw tgt::Exception to indicate an initialization failure
     */
    virtual void initialize()
        throw (tgt::Exception);

    /**
     * OpenGL-dependent initializations should be performed here.
     * However, it is usually not necessary to override this function.
     *
     * @throw tgt::Exception to indicate an initialization failure
     */
    virtual void initializeGL()
        throw (tgt::Exception);

    /**
     * Time-consuming deinitializations that do not access
     * the OpenGL API should be performed here.
     * However, it is usually not necessary to override this function.
     *
     * @throw tgt::Exception to indicate a deinitialization failure
     */
    virtual void deinitialize()
        throw (tgt::Exception);

    /**
     * OpenGL-dependent deinitializations should be performed here.
     * However, it is usually not necessary to override this function.
     *
     * @throw tgt::Exception to indicate an initialization failure
     */
    virtual void deinitializeGL()
        throw (tgt::Exception);

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
    void registerProcessor(Processor* processor);

    /**
     * Registers the passed property. To be called in the
     * derived class's constructor.
     */
    void registerProperty(Property* property);

    /**
     * Registers the passed VolumeReader. To be called in the
     * derived class's constructor.
     */
    void registerVolumeReader(VolumeReader* reader);

    /**
     * Registers the passed VolumeWriter. To be called in the
     * derived class's constructor.
     */
    void registerVolumeWriter(VolumeWriter* writer);

    /**
     * Registers the passed SerializableFactory. To be called in the
     * derived class's constructor.
     */
    void registerSerializerFactory(SerializableFactory* factory);

    /**
     * Registers the passed ProcessorWidgetFactory. To be called in the
     * derived class's constructor.
     */
    void registerProcessorWidgetFactory(ProcessorWidgetFactory* factory);

    /**
     * Registers the passed PropertyWidgetFactory. To be called in the
     * derived class's constructor.
     */
    void registerPropertyWidgetFactory(PropertyWidgetFactory* factory);

    /**
     * Registers the passed LinkEvaluatorFactory. To be called in the
     * derived class's constructor.
     */
    void registerLinkEvaluatorFactory(LinkEvaluatorFactory* factory);

    /**
     * Adds the passed directory to the shader search path.
     * To be called in the derived class's constructor.
     *
     * @note Use getModulePath() for determining the module's directory.
     */
    void addShaderPath(const std::string& path);

    bool initialized_;    ///< set by VoreenApplication after initialize() has been successfully called
    bool initializedGL_;  ///< set by VoreenApplication after initializeGL() has been successfully called

    static const std::string loggerCat_;

private:
    std::string name_;          //< unique identifier of the module
    std::string dirName_;       //< name of the module's base directory (without parent dir)
    std::string modulePath_;    //< absolute path to module directory (set by registration header)
    std::vector<Processor*> processors_;
    std::vector<Property*> properties_;
    std::vector<VolumeReader*> volumeReaders_;
    std::vector<VolumeWriter*> volumeWriters_;
    std::vector<SerializableFactory*> serializerFactories_;
    std::vector<std::string> shaderPaths_;

    std::vector<ProcessorWidgetFactory*> processorWidgetFactories_;
    std::vector<PropertyWidgetFactory*> propertyWidgetFactories_;
    std::vector<LinkEvaluatorFactory*> linkEvaluatorFactories_;
};

} // namespace

#endif // VRN_VOREENMODULE_H
