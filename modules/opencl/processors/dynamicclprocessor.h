/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_DYNAMICCLPROCESSOR_H
#define VRN_DYNAMICCLPROCESSOR_H

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/ports/volumeport.h"

#include "modules/opencl/utils/clwrapper.h"
#include "modules/opencl/properties/openclproperty.h"

namespace voreen {

class VRN_CORE_API DynamicCLProcessor : public VolumeRenderer {
public:

    enum Attribute {
        UNKNOWN,
        INPORT,
        OUTPORT,
        PROPERTY
    };

    enum AttributeType {
        UNSUPPORTED,
        IMAGE2D,
        IMAGE3D,
        INTPROPERTY,
        FLOATPROPERTY,
        BOOLPROPERTY,
        INT2PROPERTY,
        INT3PROPERTY,
        INT4PROPERTY,
        FLOAT2PROPERTY,
        FLOAT3PROPERTY,
        FLOAT4PROPERTY,
        MAT2PROPERTY,
        MAT3PROPERTY,
        MAT4PROPERTY,
        TRANSFUNCPROPERTY
    };

    struct ArgInfo {
        Attribute att_;
        AttributeType attType_;

        std::string arg_;
        std::string ant_;
        std::map<std::string, std::string> values_;

        Processor::InvalidationLevel invLevel_;

        ArgInfo(const std::string& ant)
            : att_(UNKNOWN), attType_(UNSUPPORTED), arg_(""), ant_(ant), values_(), invLevel_(Processor::INVALID_RESULT)
        {
            values_["name"] = "";
            values_["workdimssource"] = "false";
        }

        void registerAttribute(const std::string& mod, const std::string& type);
        void registerAnnotation(const std::string& tag, const std::string& value);
    };

    DynamicCLProcessor();
    virtual ~DynamicCLProcessor();

    virtual std::string getClassName() const    { return "DynamicCLProcessor"; }
    virtual std::string getModuleName() const   { return "opencl"; }
    virtual std::string getCategory() const     { return "OpenCL"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

    virtual Processor* create() const;
    virtual bool isReady() const;

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

    virtual void portResized();

    OpenCLProperty* getOpenCLProperty() {
        return &openclProp_;
    }

protected:
    virtual void setDescriptions() {
        setDescription("A dynamic processor using OpenCL.");
    }

    virtual void beforeProcess();
    virtual void process();

    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    void parseArguments(const std::string& programSource) throw (VoreenException);
    void assignAttributes() throw (VoreenException);
    void processAnnotations() throw (VoreenException);
    void setInvalidationLevels();
    void setWorkDimensions();

    void buildProgram();
    void initializePortsAndProperties(const std::string source = std::string(""));
    void buildAndInit();
    void removeOldPortsAndProperties();
    void refreshPortsAndProperties();
    void addNewInport(ArgInfo& arg);
    void addNewOutport(ArgInfo& arg);
    Property* generateNewProperty(ArgInfo& arg) throw (VoreenException);
    void updateProperty(Property* p, ArgInfo& arg) throw (VoreenException);
    std::map<std::string, float> getNumericPropertyValues(ArgInfo& arg) throw (VoreenException);

    void generateVolumeRepresentations();
    void generateTransferFunctions();
    void generateSharedTextures();

    VolumeRAM* generateVolumeFromTags(const std::string& dims, const std::string& type);

    /// Category used for logging.
    static const std::string loggerCat_;
    static std::vector<std::string> storeModsIgn_;
    static std::vector<std::string> storeMods_;
    static std::vector<std::string> clTypesIgn_;
    static std::vector<std::string> clTypes_;
    static std::vector<std::string> tagVals_;
    static std::vector<std::string> numericTagVals_;
    static std::map<std::string, AttributeType> propertyMap_;

private:

    void setupKeywords();

    OpenCLProperty openclProp_;

    cl::OpenCL* opencl_;
    cl::Context* context_;
    cl::CommandQueue* queue_;

    std::vector<ArgInfo> curArgs_;
    std::vector<cl::SharedTexture*> curSharedTexs_;
    std::vector<cl::ImageObject3D*> curVolTexs_;
    std::vector<cl::ImageObject2D*> curTFs_;
    std::vector<cl::VolumeWriteBuffer*> curVolBuffers_;

    bool regenerateSharedTextures_;
    std::map<Port*, ArgInfo*> portArgMap_;
    ArgInfo* curWorkDimensionSource_;
};

} // namespace voreen

#endif
