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

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumehash.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/datastructures/volume/modality.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "voreen/core/utils/hashing.h"
#include "voreen/core/utils/stringutils.h"

#include "tgt/filesystem.h"

#include <algorithm>
#include <string>
#include <cctype>

using std::string;
using tgt::vec3;
using tgt::mat4;

namespace {
int lower_case(int c) {
    return tolower(c);
}
}
namespace voreen {

const std::string VolumeBase::loggerCat_("voreen.VolumeBase");
const std::string Volume::loggerCat_("voreen.Volume");
const std::string VolumeURL::loggerCat_("voreen.VolumeURL");

VolumeURL::VolumeURL()
    : protocol_("")
    , path_("")
{}

VolumeURL::VolumeURL(const VolumeURL& rhs)
    : Serializable()
{
    protocol_ = rhs.getProtocol();
    path_ = rhs.getPath();
    searchParameterMap_ = rhs.searchParameterMap_;

    metaDataContainer_ = rhs.getMetaDataContainer();
}

VolumeURL::VolumeURL(const std::string& url) {
    parseURL(url, protocol_, path_, searchParameterMap_);
    path_ = tgt::FileSystem::cleanupPath(path_);
}

VolumeURL::~VolumeURL() {
}

VolumeURL& VolumeURL::operator=(const VolumeURL& rhs) {
    protocol_ = rhs.getProtocol();
    path_ = rhs.getPath();
    searchParameterMap_ = rhs.searchParameterMap_;
    metaDataContainer_ = rhs.getMetaDataContainer();
    return *this;
}

bool VolumeURL::operator==(const VolumeURL& rhs) const {
    return (getURL() == rhs.getURL());
}


VolumeURL::VolumeURL(const std::string& protocol, const std::string& filepath, const std::string& searchString) {
    // TODO: validate parameters
    protocol_ = protocol;
    path_ = tgt::FileSystem::cleanupPath(filepath);
    searchParameterMap_ = parseSearchString(searchString);
}

void VolumeURL::serialize(XmlSerializer& s) const {
   std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
   VolumeURL originConv;
   if (!basePath.empty()) {
        VolumeSerializerPopulator serializerPopulator;
        try {
            originConv = serializerPopulator.getVolumeSerializer()->convertOriginToRelativePath(*this, basePath);
        }
        catch (tgt::UnsupportedFormatException& e) {
            throw SerializationException(std::string(e.what()));
        }
    }
    else {
        originConv = *this;
    }

    // serialize with unix path separators for platform consistency
    std::string url = constructURL(originConv.getProtocol(), tgt::FileSystem::cleanupPath(originConv.getPath(), false), originConv.searchParameterMap_);
    s.serialize("url", url);
}

void VolumeURL::deserialize(XmlDeserializer& s) {
    bool found = false;
    try {
        std::string url;
        s.deserialize("url", url);
        parseURL(url, protocol_, path_, searchParameterMap_);
        found = true;
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }
    // look for alternative attribute name 'filename' (legacy)
    if (!found) {
        try {
            std::string url;
            s.deserialize("filename", url);
            parseURL(url, protocol_, path_, searchParameterMap_);
            found = true;
        }
        catch (XmlSerializationNoSuchDataException&) {
            s.removeLastError();
        }
    }
    if (!found)
        throw XmlSerializationNoSuchDataException("VolumeURL: neither attribute 'filename' nor attribute 'url' found");

    std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
    if (!basePath.empty()) {
        VolumeSerializerPopulator serializerPopulator;

        VolumeURL originConv;
        try {
            originConv = serializerPopulator.getVolumeSerializer()->convertOriginToAbsolutePath(*this, basePath);
        }
        catch (tgt::UnsupportedFormatException& e) {
            throw SerializationException(std::string(e.what()));
        }
        path_ = originConv.getPath();
    }

    path_ = tgt::FileSystem::cleanupPath(path_);
}

std::string VolumeURL::getURL() const {
    return constructURL(protocol_, path_, searchParameterMap_);
}

std::string VolumeURL::getPath() const {
    return path_;
}

std::string VolumeURL::getFilename() const {
    return tgt::FileSystem::fileName(getPath());
}

std::string VolumeURL::getSearchString() const {
    return constructSearchString(searchParameterMap_);
}

std::string VolumeURL::getProtocol() const {
    return protocol_;
}

void VolumeURL::addSearchParameter(const std::string& key, const std::string& value) {
    if (key.empty() || value.empty()) {
        LWARNING("Search key or value empty.");
        return;
    }

    searchParameterMap_[key] = value;
}

void VolumeURL::removeSearchParameter(const std::string& key) {
    if (key == "")
        return;

    std::map<std::string, std::string>::iterator it = searchParameterMap_.find(key);
    if (it != searchParameterMap_.end())
        searchParameterMap_.erase(it);
}

std::string VolumeURL::getSearchParameter(const std::string& k, bool caseSensitive) const {

    std::string key = k;
    if (caseSensitive) {
        std::map<std::string, std::string>::const_iterator it = searchParameterMap_.find(key);
        if (it != searchParameterMap_.end())
            return it->second;
        else
            return "";
    }
    else {
        transform(key.begin(), key.end(), key.begin(), lower_case);
        for (std::map<std::string, std::string>::const_iterator it = searchParameterMap_.begin();
                it != searchParameterMap_.end(); ++it) {
            std::string curKey = it->first;
            transform(curKey.begin(), curKey.end(), curKey.begin(), lower_case);
            if (key == curKey)
                return it->second;
        }
        return "";
    }
}

MetaDataContainer& VolumeURL::getMetaDataContainer() {
    return metaDataContainer_;
}

const MetaDataContainer& VolumeURL::getMetaDataContainer() const {
    return metaDataContainer_;
}

std::string VolumeURL::convertURLToRelativePath(const std::string& url, const std::string& basePath) {
    if (basePath.empty())
        return url;

    VolumeSerializerPopulator serializerPopulator;
    VolumeURL origin(url);
    VolumeURL originConv;
    try {
        originConv = serializerPopulator.getVolumeSerializer()->convertOriginToRelativePath(origin, basePath);
    }
    catch (tgt::UnsupportedFormatException& e) {
        LWARNING(std::string(e.what()));
        originConv = origin;
    }

    // serialize with unix path separators for platform consistency
    std::string result = constructURL(originConv.getProtocol(), tgt::FileSystem::cleanupPath(originConv.getPath(), false), originConv.searchParameterMap_);
    return result;
}

std::string VolumeURL::convertURLToAbsolutePath(const std::string& url, const std::string& basePath) {
    if (basePath.empty())
        return url;

    VolumeSerializerPopulator serializerPopulator;
    VolumeURL origin(url);
    VolumeURL originConv;
    std::string result;
    try {
        originConv = serializerPopulator.getVolumeSerializer()->convertOriginToAbsolutePath(origin, basePath);
        result = originConv.getURL();
    }
    catch (tgt::UnsupportedFormatException& e) {
        LWARNING(std::string(e.what()));
        result = url;
    }

    return result;
    //return tgt::FileSystem::cleanupPath(result);
}

std::string VolumeURL::constructURL(const std::string& protocol, const std::string& path, const std::map<std::string, std::string>& searchParameters) {
    std::string url = path;

    if (!protocol.empty())
        url = protocol + "://" + url;

    if (!searchParameters.empty())
        url += "?" + constructSearchString(searchParameters);

    return url;
}

std::string VolumeURL::constructSearchString(const std::map<std::string, std::string>& searchParameters) {
    std::string searchString;
    for (std::map<std::string, std::string>::const_iterator it = searchParameters.begin(); it != searchParameters.end(); ++it) {
        if (!searchString.empty())
            searchString += "&";
        searchString += escapeString(it->first) + "=" + escapeString(it->second);
    }
    return searchString;
}

void VolumeURL::parseURL(const std::string& url, std::string& protocol, std::string& path, std::map<std::string, std::string>& searchParameters) {
    // protocol
    string::size_type sep_pos = url.find("://");
    if (sep_pos == std::string::npos) {
        // URL does not contain protocol specifier
        protocol = "";
    }
    else {
        // return substring before protocol separator
        protocol = url.substr(0, sep_pos);
    }


    // path and searchString
    std::string fullPath;
    if (sep_pos == std::string::npos) {
        fullPath = url;
    }
    else {
        fullPath = url.substr(sep_pos + 3);
    }

    // get search string
    sep_pos = fullPath.find("?");
    std::string searchString;
    if (sep_pos == std::string::npos) {
        // URL does not contain search string
        path = fullPath;
        searchString = "";
    }
    else {
        // separate path from searchstring
        path = fullPath.substr(0, sep_pos);
        searchString = fullPath.substr(sep_pos + 1);
    }

    searchParameters = parseSearchString(searchString);
}

std::map<std::string, std::string> VolumeURL::parseSearchString(std::string searchString)  {
    std::map<std::string, std::string> searchParameters;

    // temporarily replace escaped '&' by '#1#' and '=' by '#2#' in order to allow splitting
    // of the search string by these chars
    searchString = strReplaceAll(searchString, "\\&", "#1#");
    searchString = strReplaceAll(searchString, "\\=", "#2#");

    std::vector<std::string> searchElems = strSplit(searchString, '&');
    for (size_t i=0; i<searchElems.size(); i++) {
        std::vector<std::string> keyValuePair = strSplit(searchElems.at(i), '=');
        if (keyValuePair.size() == 2) {
            std::string key = keyValuePair.at(0);
            std::string value = keyValuePair.at(1);

            key = strReplaceAll(key, "#1#", "\\&");
            key = strReplaceAll(key, "#2#", "\\=");
            key = unescapeString(key);

            value = strReplaceAll(value, "#1#", "\\&");
            value = strReplaceAll(value, "#2#", "\\=");
            value = unescapeString(value);

            searchParameters.insert(std::make_pair(key, value));
        }
    }

    return searchParameters;
}

std::string VolumeURL::escapeString(const std::string& str) {
    std::string result = str;

    result = strReplaceAll(result, "\\", "\\\\");
    result = strReplaceAll(result, "?", "\\?");
    result = strReplaceAll(result, "&", "\\&");
    result = strReplaceAll(result, "=", "\\=");
    result = strReplaceAll(result, " ", "\\ ");

    return result;
}

std::string VolumeURL::unescapeString(const std::string& str) {
    std::string result = str;

    result = strReplaceAll(result, "\\?", "?");
    result = strReplaceAll(result, "\\&", "&");
    result = strReplaceAll(result, "\\=", "=");
    result = strReplaceAll(result, "\\ ", " ");
    result = strReplaceAll(result, "\\\\", "\\");

    return result;
}

// ----------------------------------------------------------------------------

VolumeBase::~VolumeBase() {
    notifyDelete();
    clearFinishedThreads();
    stopRunningThreads();
    clearDerivedData();
}

Volume* VolumeBase::clone() const throw (std::bad_alloc) {
    VolumeRAM* v = getRepresentation<VolumeRAM>()->clone();
    return new Volume(v, this);
}

std::string VolumeBase::getHash() const {
    return getRawDataHash() + "-" + getMetaDataHash();
}

std::string VolumeBase::getRawDataHash() const {
    return getDerivedData<VolumeHash>()->getHash();
}

std::string VolumeBase::getMetaDataHash() const {
    MetaDataContainer metaData;

    std::vector<std::string> keys = getMetaDataKeys();
    for(size_t i=0; i<keys.size(); i++) {
        const MetaDataBase* md = getMetaData(keys[i]);
        if(md) {
            MetaDataBase* cl = md->clone();
            if(cl)
                metaData.addMetaData(keys[i], cl);
            else {
                LERROR("Failed to clone metadata!");
            }
        }
    }

    XmlSerializer s;

    s.serialize("metaData", metaData);
    //p->serializeValue(s);

    std::stringstream stream;
    s.write(stream);

    return VoreenHash::getHash(stream.str());
}

tgt::vec3 VolumeBase::getCubeSize() const {
    vec3 cubeSize = vec3(getDimensions()) * getSpacing();
    return cubeSize;
}

vec3 VolumeBase::getLLF() const {
    return getOffset();
}

vec3 VolumeBase::getURB() const {
    return getOffset()+getCubeSize();
}

tgt::mat4 VolumeBase::getVoxelToWorldMatrix() const {
    return getPhysicalToWorldMatrix() * getVoxelToPhysicalMatrix();
}

tgt::mat4 VolumeBase::getWorldToVoxelMatrix() const {
    tgt::mat4 result = tgt::mat4::identity;
    getVoxelToWorldMatrix().invert(result);
    return result;
}

tgt::mat4 VolumeBase::getWorldToTextureMatrix() const {
    tgt::mat4 result = tgt::mat4::identity;
    getTextureToWorldMatrix().invert(result);
    return result;
}

tgt::mat4 VolumeBase::getTextureToWorldMatrix() const {
    return getVoxelToWorldMatrix() * getTextureToVoxelMatrix();
}

tgt::mat4 VolumeBase::getVoxelToPhysicalMatrix() const {
    // 1. Multiply by spacing 2. Apply offset
    return tgt::mat4::createTranslation(getOffset()) * tgt::mat4::createScale(getSpacing());
}

tgt::mat4 VolumeBase::getPhysicalToVoxelMatrix() const {
    return tgt::mat4::createScale(1.0f/getSpacing()) * tgt::mat4::createTranslation(-getOffset());
}

tgt::mat4 VolumeBase::getPhysicalToWorldMatrix() const {
    return getMetaDataValue<Mat4MetaData>("Transformation", mat4::identity);
}

tgt::mat4 VolumeBase::getWorldToPhysicalMatrix() const {
    tgt::mat4 result = tgt::mat4::identity;
    getPhysicalToWorldMatrix().invert(result);
    return result;
}

tgt::mat4 VolumeBase::getTextureToPhysicalMatrix() const {
    return getVoxelToPhysicalMatrix() * getTextureToVoxelMatrix();
}

tgt::mat4 VolumeBase::getPhysicalToTextureMatrix() const {
    return getVoxelToTextureMatrix() * getPhysicalToVoxelMatrix();
}

tgt::mat4 VolumeBase::getTextureToVoxelMatrix() const {
    return tgt::mat4::createScale(getDimensions());
}

tgt::mat4 VolumeBase::getVoxelToTextureMatrix() const {
    return tgt::mat4::createScale(1.0f/vec3(getDimensions()));
}

std::vector<tgt::vec3> VolumeBase::getCubeVertices() const {
    std::vector<tgt::vec3> cubeVertices;
    vec3 llf = getLLF();
    vec3 urb = getURB();

    cubeVertices.push_back(vec3(llf.x, llf.y, urb.z));// llb 0
    cubeVertices.push_back(vec3(urb.x, llf.y, urb.z));// lrb 1
    cubeVertices.push_back(vec3(urb.x, urb.y, urb.z));// urb 2
    cubeVertices.push_back(vec3(llf.x, urb.y, urb.z));// ulb 3

    cubeVertices.push_back(vec3(llf.x, llf.y, llf.z));// llf 4
    cubeVertices.push_back(vec3(urb.x, llf.y, llf.z));// lrf 5
    cubeVertices.push_back(vec3(urb.x, urb.y, llf.z));// urf 6
    cubeVertices.push_back(vec3(llf.x, urb.y, llf.z));// ulf 7
    return cubeVertices;
}

MeshGeometry VolumeBase::getBoundingBox(bool applyTransformation) const {
    MeshGeometry boundingBox = MeshGeometry::createCube(getLLF(), getURB(), tgt::vec3(0.f), tgt::vec3(1.f));
    if (applyTransformation)
        boundingBox.transform(getPhysicalToWorldMatrix());
    return boundingBox;
}

void VolumeBase::stopRunningThreads() {
    // copy set of threads because they are removed from derivedDataThreads when they finish
    derivedDataThreadMutex_.lock();
    std::set<VolumeDerivedDataThreadBase*> copy = derivedDataThreads_;
    derivedDataThreadMutex_.unlock();

    for (std::set<VolumeDerivedDataThreadBase*>::iterator it=copy.begin(); it!=copy.end(); ++it) {
        VolumeDerivedDataThreadBase* tmp = *it;
        if(tmp->isRunning()) {
            tmp->interrupt();
            tmp->join();
        }
    }
}

void VolumeBase::clearFinishedThreads() {
    derivedDataThreadMutex_.lock();
    for (std::set<VolumeDerivedDataThreadBase*>::iterator it=derivedDataThreadsFinished_.begin(); it!=derivedDataThreadsFinished_.end(); ++it) {
        if((*it)->isRunning()) {
            (*it)->interrupt();
            (*it)->join();
        }
        delete *it;
    }
    derivedData_.clear();
    derivedDataThreadMutex_.unlock();
}

void VolumeBase::clearDerivedData() {
    derivedDataMutex_.lock();
    for (std::set<VolumeDerivedData*>::iterator it=derivedData_.begin(); it!=derivedData_.end(); ++it) {
        delete *it;
    }
    derivedData_.clear();
    derivedDataMutex_.unlock();
}

const VolumeURL& VolumeBase::getOrigin() const {
    return origin_;
}

VolumeURL& VolumeBase::getOrigin() {
    return origin_;
}

void VolumeBase::setOrigin(const VolumeURL& origin) {
    origin_ = origin;
}

float VolumeBase::getTimestep() const {
    return getMetaDataValue<FloatMetaData>("Timestep", 0.0f);
}

std::string VolumeBase::getFormat() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getFormat();
    else {
        tgtAssert(false, "Volume has no representation!");
        return 0;
    }
}

std::string VolumeBase::getBaseType() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getBaseType();
    else {
        tgtAssert(false, "Volume has no representation!");
        return 0;
    }
}

size_t VolumeBase::getNumChannels() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getNumChannels();
    else {
        tgtAssert(false, "Volume has no representation!");
        return 0;
    }
}

tgt::svec3 VolumeBase::getDimensions() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getDimensions();
    else {
        tgtAssert(false, "Volume has no representation!");
        return tgt::svec3(0, 0, 0);
    }
}

size_t VolumeBase::getNumVoxels() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getNumVoxels();
    else {
        tgtAssert(false, "Volume has no representation!");
        return 0;
    }
}

size_t VolumeBase::getBytesPerVoxel() const {
    const VolumeRepresentation* rep = getRepresentation(0);
    if(rep)
        return rep->getBytesPerVoxel();
    else {
        tgtAssert(false, "Volume has no representation!");
        return 0;
    }
}

vec3 VolumeBase::getSpacing() const {
    return getMetaDataValue<Vec3MetaData>("Spacing", vec3(1.0f));
}

RealWorldMapping VolumeBase::getRealWorldMapping() const {
    return getMetaDataValue<RealWorldMappingMetaData>("RealWorldMapping", RealWorldMapping());
}

vec3 VolumeBase::getOffset() const {
    return getMetaDataValue<Vec3MetaData>("Offset", vec3(0.0f));
}

Modality VolumeBase::getModality() const {
    return Modality(getMetaDataValue<StringMetaData, std::string>("Modality", "unknown"));
}

void VolumeBase::notifyDelete() {
    std::vector<VolumeObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeDelete(this);
}

void VolumeBase::notifyChanged() {
    std::vector<VolumeObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeChange(this);
}

template <>
const VolumeRAM* VolumeBase::getRepresentation() const {
    if(getNumRepresentations() == 0) {
        LWARNING("Found no representations for this volumehandle!" << this);
        return 0;
    }

    // check if rep. is available:
    for(size_t i=0; i<getNumRepresentations(); i++) {
        if(dynamic_cast<const VolumeRAM*>(getRepresentation(i))) {
            return static_cast<const VolumeRAM*>(getRepresentation(i));
        }
    }

    // check if conversion is possible:
    ConverterFactory fac;
    for(size_t i=0; i<getNumRepresentations(); i++) {
        RepresentationConverter<VolumeRAM>* converter = fac.findConverter<VolumeRAM>(getRepresentation(i));
        if(converter) {
            const VolumeRAM* rep = static_cast<const VolumeRAM*>(useConverter(converter)); //we can static cast here because we know the converter returns VolumeRAM*
            if (rep)
                return rep;
        }
    }

    return 0;
}

// ----------------------------------------------------------------------------

Volume::Volume(VolumeRepresentation* const volume, const tgt::vec3& spacing, const tgt::vec3& offset, const tgt::mat4& transformation)
{
    setSpacing(spacing);
    setOffset(offset);
    setPhysicalToWorldMatrix(transformation);
    addRepresentation(volume);
}

Volume::Volume(VolumeRepresentation* const volume, const VolumeBase* vh)
{
    std::vector<std::string> keys = vh->getMetaDataKeys();
    for(size_t i=0; i<keys.size(); i++) {
        const MetaDataBase* md = vh->getMetaData(keys[i]);
        if(md) {
            metaData_.addMetaData(keys[i], md->clone());
        }
    }
    addRepresentation(volume);
}

Volume::Volume(VolumeRepresentation* const volume, const MetaDataContainer* mdc) {
    std::vector<std::string> keys = mdc->getKeys();
    for(size_t i=0; i<keys.size(); i++) {
        const MetaDataBase* md = mdc->getMetaData(keys[i]);
        if(md) {
            metaData_.addMetaData(keys[i], md->clone());
        }
    }
    addRepresentation(volume);
}

Volume::Volume(VolumeRepresentation* const volume, const MetaDataContainer* mdc, const std::set<VolumeDerivedData*>& derivedData) {
    std::vector<std::string> keys = mdc->getKeys();
    for(size_t i=0; i<keys.size(); i++) {
        const MetaDataBase* md = mdc->getMetaData(keys[i]);
        if(md) {
            metaData_.addMetaData(keys[i], md->clone());
        }
    }

    for(std::set<VolumeDerivedData*>::const_iterator i = derivedData.begin(); i != derivedData.end(); i++)
       addDerivedData(*i);

    addRepresentation(volume);
}

Volume::~Volume() {
    notifyDelete();
    deleteAllRepresentations();
}

void Volume::releaseVolumes() {
    stopRunningThreads();
   representations_.clear();
}

void Volume::setVolume(VolumeRAM* const volume) {
    if(!volume) {
        LERROR("Tried to set null volume!");
        tgtAssert(false, "Tried to set null volume!");
        return;
    }

    if(VolumeBase::hasRepresentation<VolumeRAM>()) {
        const VolumeRAM* v = VolumeBase::getRepresentation<VolumeRAM>();

        if(v != volume) {
            if(v->getDimensions() != volume->getDimensions()) {
                LERROR("Tried to set volume with different dimensions!");
                tgtAssert(false, "Tried to set volume with different dimensions!");
                return;
            }

            deleteAllRepresentations();
            addRepresentation(volume);
        }
    }
    else {
        addRepresentation(volume);
        makeRepresentationExclusive<VolumeRAM>();
    }
}

bool Volume::reloadVolume() {

    ProgressBar* progressDialog = VoreenApplication::app()->createProgressDialog();
    if (progressDialog) {
        progressDialog->setTitle("Loading volume");
        progressDialog->setProgressMessage("Loading volume ...");
    }
    VolumeSerializerPopulator populator(progressDialog);

    // try to load volume from origin
    VolumeBase* handle = 0;
    try {
        handle = populator.getVolumeSerializer()->read(origin_);
    }
    catch (tgt::FileException& e) {
        LWARNING(e.what());
    }
    catch (std::bad_alloc&) {
        LWARNING("std::Error BAD ALLOCATION");
    }
    delete progressDialog;
    progressDialog = 0;

    if (!handle || !handle->getRepresentation<VolumeRAM>()) {
        delete handle;
        return false;
    }

    Volume* vh = static_cast<Volume*>(handle);
    if(VolumeBase::hasRepresentation<VolumeRAM>()) {
        deleteAllRepresentations();

        addRepresentation(vh->getWritableRepresentation<VolumeRAM>());
        vh->releaseAllRepresentations();
        delete handle;
    }

    // inform observers
    notifyChanged();
    return true;
}

void Volume::setHash(const std::string& hash) const {
    addDerivedDataInternal<VolumeHash>(new VolumeHash(hash));
}

const MetaDataContainer& Volume::getMetaDataContainer() const {
    return metaData_;
}

MetaDataContainer& Volume::getMetaDataContainer() {
    return metaData_;
}

void Volume::setModality(Modality modality) {
    setMetaDataValue<StringMetaData>("Modality", modality.getName());
}

void Volume::setRealWorldMapping(RealWorldMapping rwm) {
    setMetaDataValue<RealWorldMappingMetaData>("RealWorldMapping", rwm);
}

void Volume::setTimestep(float timestep) {
    setMetaDataValue<FloatMetaData>("Timestep", timestep);
}

void Volume::setSpacing(const tgt::vec3 spacing) {
    setMetaDataValue<Vec3MetaData>("Spacing", spacing);
}

void Volume::setOffset(const tgt::vec3 offset) {
    setMetaDataValue<Vec3MetaData>("Offset", offset);
}

void Volume::setPhysicalToWorldMatrix(const tgt::mat4& transformationMatrix) {
    return setMetaDataValue<Mat4MetaData>("Transformation", transformationMatrix);
}

//---------------------------------------------------------------

void oldVolumePosition(Volume* vh) {
    //correct old spacing:
    vec3 sp = vh->getSpacing();
    vec3 cubeSize = sp * vec3(vh->getDimensions());

    float scale = 2.0f / max(cubeSize);
    sp *= scale;
    vh->setSpacing(sp);

    //set origin to center volume:
    cubeSize = sp * vec3(vh->getDimensions());
    vh->setOffset(-cubeSize/2.0f);
}

void centerVolume(Volume* vh) {
    vec3 sp = vh->getSpacing();
    vec3 cubeSize = sp * vec3(vh->getDimensions());

    //set origin to center volume:
    vh->setOffset(-cubeSize/2.0f);
}

} // namespace
