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

#ifndef VRN_WORKSPACE_H
#define VRN_WORKSPACE_H

#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/processornetwork.h"

namespace voreen {

class VoreenCamera : public Serializable {
public:
    VoreenCamera(tgt::Camera* camera)
        : Serializable(),
          camera_(camera)
    {}

    std::string getXmlElementName() const { return "Camera"; }

    TiXmlElement* serializeToXml() const {
        TiXmlElement* cameraElem = new TiXmlElement(getXmlElementName());

        tgt::quat tripod = camera_->getQuat();
        tgt::vec3 pos    = camera_->getPosition();
        tgt::vec3 focus  = camera_->getFocus();
        tgt::vec3 upVector = camera_->getUpVector();

        TiXmlElement* posElem = new TiXmlElement("position");
        cameraElem->LinkEndChild(posElem);
        posElem->SetDoubleAttribute("x", pos.x);
        posElem->SetDoubleAttribute("y", pos.y);
        posElem->SetDoubleAttribute("z", pos.z);

        TiXmlElement* focusElem = new TiXmlElement("focus");
        cameraElem->LinkEndChild(focusElem);
        focusElem->SetDoubleAttribute("x", focus.x);
        focusElem->SetDoubleAttribute("y", focus.y);
        focusElem->SetDoubleAttribute("z", focus.z);

        TiXmlElement* upVectorElem = new TiXmlElement("upVector");
        cameraElem->LinkEndChild(upVectorElem);
        upVectorElem->SetDoubleAttribute("x", upVector.x);
        upVectorElem->SetDoubleAttribute("y", upVector.y);
        upVectorElem->SetDoubleAttribute("z", upVector.z);

        return cameraElem;
    }

    void updateFromXml(TiXmlElement* elem) {
            tgt::vec3 pos(0.0);
            TiXmlElement* posElem = elem->FirstChildElement("position");
            posElem->QueryFloatAttribute("x", &(pos.x));
            posElem->QueryFloatAttribute("y", &(pos.y));
            posElem->QueryFloatAttribute("z", &(pos.z));

            tgt::vec3 focus(0.0f, 0.0f, -1.0f);
            TiXmlElement* focusElem = elem->FirstChildElement("focus");
            focusElem->QueryFloatAttribute("x", &(focus.x));
            focusElem->QueryFloatAttribute("y", &(focus.y));
            focusElem->QueryFloatAttribute("z", &(focus.z));

            tgt::vec3 upVector(0.0f, 1.0f, 0.0f);
            TiXmlElement* upVectorElem = elem->FirstChildElement("upVector");
            upVectorElem->QueryFloatAttribute("x", &(upVector.x));
            upVectorElem->QueryFloatAttribute("y", &(upVector.y));
            upVectorElem->QueryFloatAttribute("z", &(upVector.z));

            camera_->setPosition(pos);
            camera_->setFocus(focus);
            camera_->setUpVector(upVector);
    }    

protected:
    tgt::Camera* camera_;
};

class VoreenWorkspace {
public:
    VoreenWorkspace(ProcessorNetwork* network, VolumeSetContainer* volumeSetContainer, 
                    tgt::Camera* camera, VoreenMainWindow* mainwindow)
        : version_(1), network_(network), volumeSetContainer_(volumeSetContainer), camera_(camera), 
          mainwindow_(mainwindow), readOnly_(false)

    {
    }

    void serializeToXml(const std::string& filename)
        throw (SerializerException)
    {

        TiXmlDocument doc;
        TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
        doc.LinkEndChild(declNode);

        TiXmlElement* workspaceElem = new TiXmlElement("Workspace");
        workspaceElem->SetAttribute("version", version_);

        // general Settings for the workspace
        TiXmlElement* settingsElem = new TiXmlElement("Settings");
        workspaceElem->LinkEndChild(settingsElem);

        VoreenCamera cam(camera_);
        workspaceElem->LinkEndChild(cam.serializeToXml());        
        
        // Serialize the network
        Serializable::setIgnoreIsSerializable(true);
        TiXmlElement* networkElem = network_->serializeToXml();
        workspaceElem->LinkEndChild(networkElem);
        Serializable::setIgnoreIsSerializable(false);

        // Serialize the volumes
        VolumeHandle::Origin::setBasePath(tgt::FileSystem::dirName(filename));
        TiXmlElement* volumesElem = volumeSetContainer_->serializeToXml();
        workspaceElem->LinkEndChild(volumesElem);        
        VolumeHandle::Origin::setBasePath("");
        
        doc.LinkEndChild(workspaceElem);
        
        if (!doc.SaveFile(filename))
            throw SerializerException("Could not write to file " + filename);
    }

    void loadFromXml(const std::string& filename)
        throw (SerializerException)
    {
        TiXmlDocument doc(filename);
        if (!doc.LoadFile())
            throw SerializerException("Could not load file " + filename);

        TiXmlHandle documentHandle(&doc);

        TiXmlElement* workspaceElem = documentHandle.FirstChildElement("Workspace").Element();
        if (!workspaceElem)
            throw SerializerException("Did not find workspace element!");

        // read only means do not overwrite this file
        const char* readonly = workspaceElem->Attribute("readonly");
        readOnly_ = (readonly != 0 && std::string(readonly) == "true");
       
        // Get the network
        TiXmlElement* processorElem = workspaceElem->FirstChildElement("ProcessorNetwork");
        if (processorElem) {
            ProcessorNetwork* net = new ProcessorNetwork();
            net->updateFromXml(processorElem);

            // Load the volumes
            TiXmlElement* volumesElem = workspaceElem->FirstChildElement(VolumeSetContainer::XmlElementName);

            // Some backward compatiblity
            if (!volumesElem)
                volumesElem = processorElem->FirstChildElement(VolumeSetContainer::XmlElementName);
  

            if (volumesElem) {
                if (!volumeSetContainer_)
                    volumeSetContainer_ = new VolumeSetContainer();

                try {
                    VolumeHandle::Origin::setBasePath(tgt::FileSystem::dirName(filename));
                    volumeSetContainer_->updateFromXml(volumesElem);
                    VolumeHandle::Origin::setBasePath("");
                }
                catch (std::exception& e) {
                    VolumeHandle::Origin::setBasePath("");
                    delete net;
                    //TODO: make this non-fatal. joerg
                    throw SerializerException(e.what());
                }
            }
            
            mainwindow_->setVolumeSetContainer(volumeSetContainer_);
            mainwindow_->setNetwork(net);
        }
       
        VoreenCamera cam(camera_);
        TiXmlElement* cameraElem = workspaceElem->FirstChildElement("Camera");
        if (cameraElem)
            cam.updateFromXml(cameraElem);
    }

    // flag for app to not overwrite this file, not used by serializeToXml()
    bool readOnly() const { return readOnly_; }
    
protected:
    int version_;
    ProcessorNetwork* network_;
    VolumeSetContainer* volumeSetContainer_;
    tgt::Camera* camera_;    
    VoreenMainWindow* mainwindow_;
    bool readOnly_;
};

} // namespace

#endif //VRN_WORKSPACE_H
