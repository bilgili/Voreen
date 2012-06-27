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

#include "tgt/glmath.h"
#include "tgt/camera.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/lightmaterial.h"

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

//---------------------------------------------------------------------------

const Identifier Processor::delete_("processor.delete");
const Identifier Processor::setVolumeContainer_("set.volumeContainer");
const Identifier Processor::setCurrentDataset_("set.currentDataset");
const Identifier Processor::setBackgroundColor_("set.backgroundColor");
const Identifier Processor::setCoarseness_("set.Coarseness");

const Identifier Processor::entryParamsTexUnitIdent_("texUnit.entryParams");
const Identifier Processor::entryParamsDepthTexUnitIdent_("texUnit.entryParamsDepth");
const Identifier Processor::exitParamsTexUnitIdent_("texUnit.exitParams");
const Identifier Processor::exitParamsDepthTexUnitIdent_("texUnit.exitParamsDepth");
const Identifier Processor::volTexUnitIdent_("texUnit.volume");
const Identifier Processor::volTexUnit2Ident_("texUnit.volume2");
const Identifier Processor::transferTexUnitIdent_("texUnit.transferFunc");
const Identifier Processor::segmentationTexUnitIdent_("texUnit.segmentation");
const Identifier Processor::shadowTexUnit1Ident_("texUnit.shadowTex1");
const Identifier Processor::shadowTexUnit2Ident_("texUnit.shadowTex2");
const Identifier Processor::shadowTexUnit3Ident_("texUnit.shadowTex3");
const Identifier Processor::shadowTexUnit4Ident_("texUnit.shadowTex4");
const Identifier Processor::ambTexUnitIdent_("texUnit.ambientOcclusion");
const Identifier Processor::ambLookupTexUnitIdent_("texUnit.ambientOcclusionLookup");
const Identifier Processor::firstHitNormalsTexUnitIdent_("texUnit.firstHitNormals");
const Identifier Processor::firstHitPointsTexUnitIdent_("texUnit.firstHitPoints");

const std::string Processor::loggerCat_("voreen.voreen.Processor");

std::vector<Processor*> Processor::registeredProcessors_;


Processor::Processor(tgt::Camera* camera, TextureContainer* tc)
    : MessageReceiver()
    , Serializable()
    , cachable_(false)
    , cached_(false)
    , isCoprocessor_(false)
    , isMultipass_(false)
    , camera_(camera)
    , tc_(tc)
    , tm_()
    , backgroundColor_(setBackgroundColor_, "Background color", tgt::Color(0.0f, 0.0f, 0.0f, 0.0f))
    , lightPosition_(LightMaterial::setLightPosition_, "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_(LightMaterial::setLightAmbient_, "Ambient light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_(LightMaterial::setLightDiffuse_, "Diffuse light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_(LightMaterial::setLightSpecular_, "Specular light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightAttenuation_(LightMaterial::setLightAttenuation_, "Attenutation", tgt::vec3(1.f, 0.f, 0.f))
    , materialAmbient_(LightMaterial::setMaterialAmbient_, "Ambient material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialDiffuse_(LightMaterial::setMaterialDiffuse_, "Diffuse material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialSpecular_(LightMaterial::setMaterialSpecular_, "Specular material color", tgt::Color(1.f, 1.f, 1.f, 1.f))
    , materialEmission_(LightMaterial::setMaterialEmission_, "Emissive material color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , materialShininess_(LightMaterial::setMaterialShininess_, "Shininess", 60.f, 0.1, 128.f)
    , initStatus_(VRN_OK)
{
    meta_ = MetaSerializer();
}



Processor::~Processor() {
    // FIXME: Why automatically remove a processor from MsgDistr at destruction? 
    // A processor is not inserted by its constructor!
    if (tgt::Singleton<voreen::MessageDistributor>::isInited()) {
        MsgDistr.remove(this);
        ProcessorPointerMsg* msg = new ProcessorPointerMsg(Processor::delete_ ,this);
	    MsgDistr.postMessage(msg, Message::all_);
    }

    for (size_t i=0; i<inports_.size(); i++) 
		delete inports_.at(i);
	for (size_t i=0; i<outports_.size(); i++) 
		delete outports_.at(i);
	for (size_t i=0; i<coProcessorInports_.size(); i++) 
		delete coProcessorInports_.at(i);
	for (size_t i=0; i<coProcessorOutports_.size(); i++) 
		delete coProcessorOutports_.at(i);
	for (size_t i=0; i<privatePorts_.size(); i++) 
		delete privatePorts_.at(i);
	//for (size_t i=0; i<props_.size(); i++)
	//    delete props_.at(i);
}

int Processor::initializeGL() {
    return VRN_OK;
}

void Processor::init() {
}

Message* Processor::call(Identifier /*ident*/,LocalPortMapping*) {
	return 0;
}

void Processor::deinit() {
}

void Processor::addProperty(Property* prop) {
    props_.push_back(prop);
    prop->setOwner(this);
    prop->setMsgDestination(getTag());
}

const Properties& Processor::getProperties() const {
	return props_;
}

bool Processor::connect(Port* outport, Port* inport) {
    if (!inport->isOutport()) {
        if ( (outport->getType().getSubString(0) == inport->getType().getSubString(0)) && (inport->getProcessor() != this) ) {
            if (inport->allowMultipleConnections() || inport->getConnected().size() == 0 ) {
                if (!inport->isConnectedTo(outport)) {
                    outport->addConnection(inport);
                    inport->addConnection(outport);
                    return true;
                }
            }
        }
    }
    return false;
}

bool Processor::testConnect(Port* outport, Port* inport) {
    if (!inport->isOutport()) {
        if ( (outport->getType().getSubString(0) == inport->getType().getSubString(0)) && (inport->getProcessor() != this) ) {
            if (inport->allowMultipleConnections() || inport->getConnected().size() == 0 ) {
                if (!inport->isConnectedTo(outport)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Processor::disconnect(Port* outport, Port* inport) {
    bool success = false;
    for (size_t i=0; i<inport->getConnected().size(); i++) {
        if (inport->getConnected()[i] == outport) {
            inport->getConnected().erase(inport->getConnected().begin() + i);
            success = true;
        }
    }
    if (!success) return false;
    for (size_t i=0; i<outport->getConnected().size(); i++) {
        if (outport->getConnected()[i] == inport) {
            outport->getConnected().erase(outport->getConnected().begin() + i);
            return true;
        }
    }
    
    return false;
}

Port* Processor::getPort(Identifier ident) {
	for (size_t i=0; i <inports_.size(); i++) {
		if (inports_.at(i)->getType() == ident) 
			return inports_.at(i);
	}

	for (size_t i=0; i <outports_.size(); i++) {
		if (outports_.at(i)->getType() == ident) 
			return outports_.at(i);
	}

	for (size_t i=0; i <coProcessorInports_.size(); i++) {
		if (coProcessorInports_.at(i)->getType() == ident) 
			return coProcessorInports_.at(i);
	}

	for (size_t i=0; i <coProcessorOutports_.size(); i++) {
		if (coProcessorOutports_.at(i)->getType() == ident) 
			return coProcessorOutports_.at(i);
	}

	for (size_t i=0; i<privatePorts_.size(); i++) {
		if (privatePorts_.at(i)->getType() == ident)
			return privatePorts_.at(i);
	}
	
	return 0;
}

void Processor::createInport(Identifier ident, bool allowMultipleConnectios) {
	inports_.push_back(new Port(ident,this,false,allowMultipleConnectios,false));
}

void Processor::createOutport(Identifier ident, bool isPersistent,Identifier inport) {
	Port* newOutport = new Port(ident,this,true,true,isPersistent);
    outports_.push_back(newOutport);
    if (inport != "dummy.port.unused") {
        if (getPort(inport) != 0) {
            outportToInportMap_.insert(std::pair<Port*,Port*>(newOutport,getPort(inport)));
        }
        else {
            LWARNING("Couldn't find the inport in Processor::createOutport(Identifier ident, Identifier ident)");
        }
    }
}
void Processor::createCoProcessorInport(Identifier ident, bool allowMultipleConnectios) {
	coProcessorInports_.push_back(new Port(ident,this,false,allowMultipleConnectios,false));
}

void Processor::createCoProcessorOutport(Identifier ident, FunctionPointer function, bool allowMultipleConnectios) {
	Port* newPort = new Port(ident,this,true,allowMultipleConnectios,false);
	newPort->setFunctionPointer(function);
	coProcessorOutports_.push_back(newPort);
}

void Processor::createPrivatePort(Identifier ident) {
	privatePorts_.push_back(new Port(ident,this,true,false,true));
}

void Processor::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);
    if (msg->id_ == setBackgroundColor_) {
        backgroundColor_.set(msg->getValue<tgt::Color>());
    }
    else if (msg->id_ == "set.viewport") {
        tgt::ivec2 v = msg->getValue<tgt::ivec2>();
        glViewport(0, 0, v.x, v.y);
        setSizeTiled(v.x, v.y);
        if (tc_) {
            tc_->setSize(v);
        }
        std::cout << "set.viewport to " << v << std::endl;
    }
    else if (msg->id_ == LightMaterial::setLightPosition_) {
        tgtAssert( typeid(*msg) == typeid(Vec4Msg), "msg is of wrong type!" );
        lightPosition_.set(msg->getValue<vec4>());
    }
    else if (msg->id_ == LightMaterial::setLightAmbient_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        lightAmbient_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightDiffuse_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        lightDiffuse_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightSpecular_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        lightSpecular_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setLightAttenuation_) {
        tgtAssert( typeid(*msg) == typeid(Vec3Msg), "msg is of wrong type!" );
        lightAttenuation_.set(msg->getValue<vec3>());
    }
    else if (msg->id_ == LightMaterial::setMaterialAmbient_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        materialAmbient_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialDiffuse_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        materialDiffuse_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialSpecular_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        materialSpecular_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialEmission_) {
        tgtAssert( typeid(*msg) == typeid(ColorMsg), "msg is of wrong type!" );
        materialEmission_.set(msg->getValue<Color>());
    }
    else if (msg->id_ == LightMaterial::setMaterialShininess_) {
        tgtAssert( typeid(*msg) == typeid(FloatMsg), "msg is of wrong type!" );
        materialShininess_.set(msg->getValue<float>());
    }


}

void Processor::setTextureContainer(TextureContainer* tc) {
    tc_ = tc;
}

void Processor::setCamera(tgt::Camera* camera) {
    camera_ = camera;
}

TextureContainer* Processor::getTextureContainer() {
    return tc_;
}

void Processor::setGeometryContainer(GeometryContainer* geoCont) {
    geoContainer_ = geoCont;
}

GeometryContainer* Processor::getGeometryContainer() const {
    return geoContainer_;
}

void Processor::setName(const std::string& name) {
    name_ = name;
}

std::string Processor::getName() const {
    return name_;
}

void Processor::setCoarseness(float factor) {
	if (coarsenessFactor_ != factor) {
		float oldfactor	= factor;
		coarsenessFactor_=factor;
        setSize(tgt::ivec2(tgt::iround(size_.x/coarsenessFactor_ * oldfactor), tgt::iround(size_.y/coarsenessFactor_*oldfactor) ));
	}
}

float Processor::getCoarsenessFactor() const {
	return coarsenessFactor_;
}

Port* Processor::getInport(Identifier type) {
    for (size_t i=0; i<inports_.size(); i++) {
        if (inports_.at(i)->getType() == type) {
            return inports_.at(i);
        }
    }
    return 0;
}

Port* Processor::getOutport(Identifier type) {
    for (size_t i=0; i<outports_.size(); i++) {
        if (outports_.at(i)->getType() == type) {
            return outports_.at(i);
        }
    }
    return 0;
}

void Processor::setSize(const tgt::ivec2& size) {
	if (size_ != size) {
		size_ = size;

		glViewport(0, 0, size.x, size.y);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

        camera_->getFrustum().setRatio((float)size.x/(float)size.y);
        camera_->updateFrustum();
		tgt::loadMatrix(camera_->getProjectionMatrix());

		glMatrixMode(GL_MODELVIEW);

        invalidate();
	}
}

void Processor::invalidate() {
    //It is ABSOLUTELY important to send this message only to "evaluator". If you send it to "all" or a voreenpainter,
    //the message will arrive too often.
	MsgDistr.postMessage(new ProcessorPointerMsg(NetworkEvaluator::unsetCachedForward_,this), "evaluator");
}

void Processor::renderQuad() {
    glBegin(GL_QUADS);
        glVertex2f(-1.0, -1.0);
        glVertex2f( 1.0, -1.0);
        glVertex2f( 1.0,  1.0);
        glVertex2f(-1.0,  1.0);
    glEnd();
}

std::string Processor::generateHeader() {
    std::string header = "";

    // #version needs to be on the very first line for some compilers (ATI)
    header += "#version 110\n";

    if (tc_) {
        if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_2D) {
            header += "#define VRN_TEXTURE_2D\n";
        } else if (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE) {
            header += "#define VRN_TEXTURE_RECTANGLE\n";
        }
    }

    if (GLEW_NV_fragment_program2) {
        GLint i = -1;
        glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOOP_COUNT_NV, &i);
        if (i > 0) {
            std::ostringstream o;
            o << i;
            header += "#define VRN_MAX_PROGRAM_LOOP_COUNT " + o.str() + "\n";
        }
    }

    return header;
}

// Parameters currently set:
// - screenDim_
// - screenDimRCP_
// - cameraPosition_ (camera position in world coordinates)
// - lightPosition_ (light source position in world coordinates)
void Processor::setGlobalShaderParameters(tgt::Shader* shader) {
    tgtAssert(shader != NULL, "No shader passed!");
    GLint location;
    if (tc_) {
        location = shader->getUniformLocation("screenDim_", true);
        if (location != -1)
            shader->setUniform(location, tgt::vec2(tc_->getSize()));

        location = shader->getUniformLocation("screenDimRCP_", true);
        if (location != -1)
            shader->setUniform(location, 1.f / tgt::vec2(tc_->getSize()));
    }

    // camera position in world coordinates
    location = shader->getUniformLocation("cameraPosition_", true);
    if ( location != -1) {
        shader->setUniform(location, camera_->getPosition());
    }

    // light source position in world coordinates
    location = shader->getUniformLocation("lightPosition_", true);
    if ( location != -1) {
        tgt::vec3 lightPos = tgt::vec3(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
        shader->setUniform("lightPosition_", lightPos);
    }

}

void Processor::setLightingParameters() {

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem );
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem );
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem );
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, lightAttenuation_.get().x );
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuation_.get().y );
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, lightAttenuation_.get().z );

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission_.get().elem);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());

}

void Processor::process(LocalPortMapping* /*portMapping*/)/* throw (std::exception)*/ {
	
}

void Processor::setSizeTiled(uint width, uint height) {
    setSize(tgt::ivec2(width, height));

	// gluPerspective replacement taken from
    // http://nehe.gamedev.net/data/articles/article.asp?article=11

    float fovY = 45.0;
    float aspect = static_cast<float>(width) / height;
    float zNear = 0.1f;
    float zFar = 50.0f;

    float fw, fh;
    fh = tan( fovY / 360 * (float)tgt::PI ) * zNear;
    fw = fh * aspect;

	tgt::Frustum frust_ = tgt::Frustum(-fw, fw, - fh, fh, zNear, zFar);
	camera_->setFrustum(frust_);
	camera_->updateFrustum();

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    tgt::loadMatrix(camera_->getProjectionMatrix());
	glMatrixMode(GL_MODELVIEW);

}

bool Processor::isEndProcessor() const {
    return (outports_.size() == 0 && coProcessorOutports_.size() == 0);
}

const std::string Processor::XmlElementName = "Processor";

std::string Processor::getXmlElementName() const {
    return XmlElementName;
}

const Identifier Processor::getClassName(TiXmlElement* processorElem) {
    if (!processorElem)
        throw XmlElementException("Can't get ClassName of Null-Pointer!");
    if (processorElem->Value() != XmlElementName)
        throw XmlElementException(std::string("Cant get ClassName of a ") + processorElem->Value() + " - need " + XmlElementName + "!");
    Identifier type(processorElem->Attribute("type"));
    return type;
}

TiXmlElement* Processor::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* processorElem = new TiXmlElement(XmlElementName);
    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    processorElem->LinkEndChild(metaElem);
    // misc settings
    processorElem->SetAttribute("type", getClassName().getName());
    //processorElem->SetAttribute("name", getName());
    //serialize the properties of the processor
    std::vector<Property*> properties = getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
        if( properties[i]->isSerializable() ) {
            TiXmlElement* propElem = properties[i]->serializeToXml();
            processorElem->LinkEndChild(propElem);
        }
    }
    
    return processorElem;
}

TiXmlElement* Processor::serializeToXml(const std::map<Processor*,int> idMap) const {
    TiXmlElement* processorElem = serializeToXml();
    Processor* self = const_cast<Processor*>(this); // For const-correctness - can't use 'this' in Map::find
    processorElem->SetAttribute("id", idMap.find(self)->second);
    // serialize the (in)ports a.k.a. connection info
    std::vector<Port*> outports = getOutports();
    std::vector<Port*> coprocessoroutports = getCoProcessorOutports();
    // append coprocessoroutports to outports because we can handle them identically
    outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());
    for (size_t i=0; i < outports.size(); i++) {
        // add (out)port
        TiXmlElement* outportElem = new TiXmlElement("Outport");
        outportElem->SetAttribute("type", outports[i]->getType().getName());
        processorElem->LinkEndChild(outportElem);
        // add processors connected to this (out)port via one of their (in)ports
        std::vector<Port*> connectedPorts = outports[i]->getConnected();
        for (size_t j=0; j < connectedPorts.size(); j++) {
            TiXmlElement* connectedProcessorElem = new TiXmlElement(XmlElementName);
            connectedProcessorElem->SetAttribute("id", idMap.find(connectedPorts[j]->getProcessor())->second);
            connectedProcessorElem->SetAttribute("port", connectedPorts[j]->getType().getName());
            // For inports that allow multiple connections
            // need to know the index of outport in the inports connected ports
            if (connectedPorts[j]->allowMultipleConnections())
                connectedProcessorElem->SetAttribute("order", connectedPorts[j]->getIndexOf(outports[i]));
            outportElem->LinkEndChild(connectedProcessorElem);
        }
    }
        
    return processorElem;
}

void Processor::updateFromXml(TiXmlElement* processorElem) {
    serializableSanityChecks(processorElem);
    // meta
    TiXmlElement* metaElem = processorElem->FirstChildElement(meta_.getXmlElementName());
    if (metaElem)
        meta_.updateFromXml(metaElem);
    else
        throw XmlElementException("Metadata missing!");
    
    // read properties
    std::vector<Property*> properties = getProperties();
    TiXmlElement* propertyElem;
    for (propertyElem = processorElem->FirstChildElement(Property::XmlElementName);
        propertyElem;
        propertyElem = propertyElem->NextSiblingElement(Property::XmlElementName))
    {
        //std::string property_text = propertyElem->Attribute("Property_text");
        for (size_t j = 0; j < properties.size(); j++) {
            if (properties.at(j)->getIdent() == Property::getIdent(propertyElem) ) {
                // Found a property in the current pipeline which can get
                // some attributes from xml file
                //if (property_text=="empty") { // FIXME: Ugly! - almost fixed...
                //    break;
                //}
                // first set autochange false
                //properties.at(j)->setAutoChange(false);
                properties.at(j)->updateFromXml(propertyElem);
                // set autochange
                //int autoChange;
                //propertyElem->QueryIntAttribute("Autochange", &autoChange);
                //properties.at(j)->setAutoChange(autoChange == 1 ? true : false);
            }
        }
    }
}

std::pair<int, ConnectionMap> Processor::getMapAndupdateFromXml(TiXmlElement* processorElem) {
    updateFromXml(processorElem);
    ConnectionSite out, in;
    ConnectionMap lcm;
    int id;
    if (processorElem->QueryIntAttribute("id", &id) != TIXML_SUCCESS)
    //if (processorElem->QueryIntAttribute("id", &out.processorId) != TIXML_SUCCESS)
        throw XmlAttributeException("Required attribute id of Processor missing!");
    out.processorId = id;
    
    // read outports
    TiXmlElement* outportElem;
    for (outportElem = processorElem->FirstChildElement("Outport");
        outportElem;
        outportElem = outportElem->NextSiblingElement("Outport"))
    {
        if (!outportElem->Attribute("type"))
            throw XmlAttributeException("Required attribute type of Port missing!");
        //std::string type = outportElem->Attribute("type");
        out.portId = outportElem->Attribute("type");
        //if (outportElem->QueryIntAttribute("order", &out.order) != TIXML_SUCCESS)
            out.order = 0; // Annotation: The order for the first Port is not really needed
        // read processors connected to outport
        TiXmlElement* connectedprocessorElem;
        for (connectedprocessorElem = outportElem->FirstChildElement(XmlElementName);
            connectedprocessorElem;
            connectedprocessorElem = connectedprocessorElem->NextSiblingElement(XmlElementName))
        {
            //int connectedprocessorid;
            //if (connectedprocessorElem->QueryIntAttribute("id", &connectedprocessorid) != TIXML_SUCCESS)
            if (connectedprocessorElem->QueryIntAttribute("id", &in.processorId) != TIXML_SUCCESS)
                throw XmlAttributeException("Required attribute id of Processor missing!");
            if (!connectedprocessorElem->Attribute("port"))
                throw XmlAttributeException("Required attribute port of Processor missing!");
            if (connectedprocessorElem->QueryIntAttribute("order", &in.order) != TIXML_SUCCESS)
                in.order = 0;
            //std::string connectedprocessorport = connectedprocessorElem->Attribute("port");
            in.portId = connectedprocessorElem->Attribute("port");
            // store this connection in the map
            //lcm.insert(std::make_pair(type, std::make_pair(connectedprocessorid, connectedprocessorport)));
            lcm.push_back(std::make_pair(out, in));
        }
    }

    return std::make_pair(id, lcm);
}

//---------------------------------------------------------------------------

DataSupplyProcessor::DataSupplyProcessor() 
    : Processor()
    , dataset_("datasetnumber.changed","Dataset", 0, 0, 10, false)
    , pass_(-1)
    , volumeContainer_(0)
{
    addProperty(&dataset_);
	createOutport("volume.dataset");
    getOutports()[0]->setData(dataset_.get());
}

void DataSupplyProcessor::process(LocalPortMapping* /*portMapping*/) {
    if(isMultipass()) {
        pass_++;
    }
}

void DataSupplyProcessor::processMessage(Message* msg,const Identifier &ident) {
	Processor::processMessage(msg,ident);
	if (msg->id_ == "datasetnumber.changed") {
        int number = msg->getValue<int>();
		(getOutports()[0])->setData(number);
		MsgDistr.postMessage(new ProcessorPointerMsg("do.portmapping",this), "evaluator");
	}
}

void DataSupplyProcessor::initFirstPass(LocalPortMapping* /*portMapping*/) {
    pass_ = -1;
    if(volumeContainer_) {
        VolumeGL* vgl = volumeContainer_->getVolumeGL(dataset_.get());
        if (vgl)
            sortedTextures_ = vgl->getSortedTextures(camera_->getViewMatrix());
    }
}

int DataSupplyProcessor::passesNeeded() {
    //ask the volume...
    if(volumeContainer_) {
        VolumeGL* vgl = volumeContainer_->getVolumeGL(dataset_.get());
        if (vgl)
            return vgl->getNumTextures();
    }
    return 1;
}

} // namespace voreen
