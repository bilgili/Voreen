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

#ifndef VRN_VOLUME_H
#define VRN_VOLUME_H

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volumederiveddata.h"

#include "voreen/core/datastructures/volume/modality.h"
#include "voreen/core/utils/observer.h"

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/datastructures/meta/metadatacontainer.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"

#include <set>
#include <map>
#include <string>
#include <stdexcept>

namespace voreen {

class VolumeBase;
class Volume;

/**
 * Interface for volume observers.
 */
class VRN_CORE_API VolumeObserver : public Observer {
public:
    /**
     * This method is called by the observed Volume's destructor.
     *
     * @param source the calling Volume
     */
    virtual void volumeDelete(const VolumeBase* source) = 0;

    /**
     * This method is called by the observed Volume
     * after its member Volume object has changed.
     *
     * When this function is called, the new Volume object has
     * already been assigned. The former Volume object is still
     * valid at this point, but it is deleted immediately after
     * this function has been called.
     *
     * @param source the calling Volume
     */
    virtual void volumeChange(const VolumeBase* source) = 0;

    /**
     * This method is called by the observed Volume after a VolumeDerivedDataThread has finished.
     */
    virtual void derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData) {}
};

//-------------------------------------------------------------------------------------------------

/**
 * A VolumeURL encapsulates a URL that specifies the location of a \em single volume.
 *
 * The structure of a origin URL is as follows:
 * \verbatim
 *     protocol://filepath?key1=value&key2=value2...
 * \endverbatim
 * where only the filepath component is obligatory. The optional protocol string
 * specifies the data type of the referenced volume. The search string consisting
 * of key/value pairs may be used to encode additional information necessary for
 * distinctly identifying the referenced volume within a container file.
 * Some examples for valid origin URLs are:
 * - path/to/myvolume.dat
 * - dat://path/to/myvolume.dat
 * - dicom://path/to/DICOMDIR?SeriesInstanceUID=1.3.12.2
 *
 * The VolumeURL's MetaDataContainer may be used to provide optional information
 * that can be presented in a user interface. The MetaDataContainer is not persisted.
 *
 */
struct VRN_CORE_API VolumeURL : public Serializable {

    VolumeURL();

    /**
     * Constructs the origin from the passed URL.
     *
     * @note Do not use this constructor, if the URL string contains search values with unescaped special chars (?&=\<space>).
     *  Use addSearchParameter() instead in this case.
     */
    VolumeURL(const std::string& URL);

    /**
     * Constructs the origin from the specified protocol string, filepath and optional search string.
     *
     * @note Do not use this constructor, if the search values contain unescaped special chars (?&=\<space>).
     *  Use addSearchParameter() instead in this case.
     */
    VolumeURL(const std::string& protocol, const std::string& filepath, const std::string& searchString = "");

    virtual ~VolumeURL();

    VolumeURL& operator=(const VolumeURL& rhs);
    VolumeURL(const VolumeURL& rhs);
    bool operator==(const VolumeURL& rhs) const;

    /// Returns the complete URL where volume is loaded from.
    std::string getURL() const;

    /// Returns the protocol portion of the URL, which specifies the data format. May be empty.
    std::string getProtocol() const;

    /// Returns the path portion of the URL, without the protocol specifier and the trailing search string.
    std::string getPath() const;

    /// Returns the file name component of the URL.
    std::string getFilename() const;

    /// Returns the search string portion of the URL. May be empty.
    std::string getSearchString() const;

    /**
     * Appends the given search parameter to the URL in the form: "key=value"
     * If the key is already present, its value is overridden.
     */
    void addSearchParameter(const std::string& key, const std::string& value);

    /**
     * Removes the passed search key from the URL. If no parameter with this key exists,
     * the call is ignored.
     */
    void removeSearchParameter(const std::string& key);

    /**
     * Returns the value corresponding to the passed key in the URL's search string,
     * or an empty string, if the key is not found.
     *
     * @param key name of the search string attribute to extract
     * @param caseSensitive if true, the key name is compared case-sensitively
     */
    std::string getSearchParameter(const std::string& key, bool caseSensitive = true) const;

    /**
     * Return the VolumeURL's MetaDataContainer, which may be used to store
     * additional information about the referenced volume that is not required for
     * distinctly identifying it.
     *
     * @note The MetaDataContainer is not serialized.
     */
    MetaDataContainer& getMetaDataContainer();

    /// @overload
    const MetaDataContainer& getMetaDataContainer() const;

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

    static std::string convertURLToRelativePath(const std::string& url, const std::string& basePath);
    static std::string convertURLToAbsolutePath(const std::string& url, const std::string& basePath);

    /// Replaces backslashes.
    static std::string cleanupPath(const std::string& path);

private:
    std::string constructURL(const std::string& protocol, const std::string& path, const std::map<std::string, std::string>& searchParameters) const;
    void parseURL(const std::string& url, std::string& protocol, std::string& path, std::map<std::string, std::string>& searchParameters) const;

    std::string constructSearchString(const std::map<std::string, std::string>& searchParameterMap) const;
    std::map<std::string, std::string> parseSearchString(std::string searchString) const;

    std::string escapeString(const std::string& str) const;
    std::string unescapeString(const std::string& str) const;

    std::string protocol_;  //< protocol portion of the URL (may be empty)
    std::string path_;      //< path portion of the URL
    std::map<std::string, std::string> searchParameterMap_; //< search parameters as key/value pairs

    /// May contain additional meta information about the volume (not serialized).
    MetaDataContainer metaDataContainer_;

    static const std::string loggerCat_;
};

//-------------------------------------------------------------------------------------------------

template <class T>
class VolumeDerivedDataThread : public VolumeDerivedDataThreadBase {
public:
    virtual void run() {
        T dummy;
        VolumeDerivedData* tmp = 0;
        try {
            tmp = dummy.createFrom(volume_);
        }
        catch(boost::thread_interrupted&)
        {
            tmp = 0;
            resultMutex_.lock();
            result_ = 0;
            resultMutex_.unlock();
            return;
        }
        resultMutex_.lock();
        result_ = tmp;
        resultMutex_.unlock();

        notifyVolume();
    }

protected:
    void notifyVolume();
};

//-------------------------------------------------------------------------------------------------

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API Observable<VolumeObserver>;
#endif

class VRN_CORE_API VolumeBase : public Observable<VolumeObserver> {
public:
    virtual ~VolumeBase();

    virtual std::vector<std::string> getMetaDataKeys() const = 0;
    virtual const MetaDataBase* getMetaData(const std::string& key) const = 0;
    virtual bool hasMetaData(const std::string& key) const = 0;

    /*
     * @param def Default return value in case metadata could not be found.
     */
    template<typename T, typename U>
    U getMetaDataValue(const std::string& key, U def) const {
        if(hasMetaData(key)) {
            const MetaDataBase* mdb = getMetaData(key);
            const T* md = dynamic_cast<const T*>(mdb);
            if(md)
                return static_cast<U>(md->getValue());
            else
                return def;
        }
        else
            return def;
    }

    /**
     * Returns the derived data item of the specified type T,
     * which must be a concrete subtype of VolumeDerivedData.
     *
     * If no derived data item of the type T exists, a new item is created
     * and stored if possible. Otherwise, 0 is returned.
     *
     * @see hasDerivedData
     */
    template<class T>
    T* getDerivedData() const;

    /**
     * Launches a thread to calculate derived data of type T.
     * Observe this volume to get notified when calculations finish (@see VolumeObserver::derivedDataThreadFinished())
     *
     * @return If a derived data of type T has already been calculated it is returned, otherwise 0.
     */
    template<class T>
    T* getDerivedDataThreaded() const;

    /**
     * Returns whether there exists a derived data item of the specified type T,
     * which must be a concrete subtype of VolumeDerivedData.
     */
    template<class T>
    T* hasDerivedData() const;

    /**
     * Adds the given data item to the derived data associated with this handle.
     * The template type T must be a concrete subtype of VolumeDerivedData.
     *
     * @note The handle takes ownership of the passed data item.
     * @note An existing item of the type T is replaced and deleted.
     */
    template<class T>
    void addDerivedData(T* data) {
        addDerivedDataInternal<T>(data);
    }

    void addDerivedData(VolumeDerivedData* data) {
        derivedDataMutex_.lock();
        for (std::set<VolumeDerivedData*>::const_iterator it=derivedData_.begin(); it!=derivedData_.end(); ++it) {
            if (typeid(**it) == typeid(*data)) {
                derivedData_.erase(it);
                break;
            }
        }
        derivedData_.insert(data);
        derivedDataMutex_.unlock();
    }

    /**
     * Removes and deletes the derived data item with the specified type T,
     * which must be a concrete subtype of VolumeDerivedData.
     * If no item with the specified type T exists, the call has no effect.
     */
    template<class T>
    void removeDerivedData() {
        removeDerivedDataInternal<T>();
    }

    /**
     * Deletes all derived data items associated with this handle.
     */
    void clearDerivedData();

    /// Computes the MD5 hash of the raw volume data.
    virtual std::string getRawDataHash() const;
    /// Computes the MD5 hash of the meta data.
    virtual std::string getMetaDataHash() const;
    /// Concatenation of raw data and meta data hash (getRawDataHash() + "-" + getMetaDataHash())
    virtual std::string getHash() const;

     /*
      * Returns a representation (@see VolumeRepresentation) of type T, performing an automatic conversion between representation if necessory.
      * Cannot be used to convert between formats (e.g., from 16- to 8-bit), call using the base class (i.e., VolumeRAM).
      */
    template <class T>
    const T* getRepresentation() const;

    virtual size_t getNumRepresentations() const = 0;
    virtual const VolumeRepresentation* getRepresentation(size_t i) const = 0;
    virtual const VolumeRepresentation* useConverter(const RepresentationConverterBase* converter) const = 0;
    virtual void addRepresentation(VolumeRepresentation* representation) = 0;
    virtual void removeRepresentation(size_t i) = 0;

    template <class T>
    bool hasRepresentation() const {
        for(size_t i=0; i<getNumRepresentations(); i++) {
            if(dynamic_cast<const T*>(getRepresentation(i)))
                return true;
        }
        return false;
    }

    template <class T>
    void removeRepresentation() {
        for(size_t i=0; i<getNumRepresentations(); i++) {
            if (dynamic_cast<const T*>(getRepresentation(i))) {
                removeRepresentation(i);
            }
        }
    }

    /// Returns the format of the volume as string (e.g., "uint8" or "Vector3(float)", @see VolumeFactory).
    std::string getFormat() const;

    /// Returns the base type (e.g.,    "float" for a representation of format "Vector3(float)").
    std::string getBaseType() const;

    size_t getNumChannels() const;
    tgt::svec3 getDimensions() const;
    size_t getNumVoxels() const;
    size_t getBytesPerVoxel() const;

    virtual Volume* clone() const throw (std::bad_alloc);

    // Metadata shortcuts:
    /// Returns the associated timestep of this volume.
    virtual float getTimestep() const;
    tgt::vec3 getSpacing() const;
    tgt::vec3 getOffset() const;
    virtual Modality getModality() const;
    RealWorldMapping getRealWorldMapping() const;

    /// Returns the 8 cube vertices in physical coordinates.
    virtual std::vector<tgt::vec3> getCubeVertices() const;

    /**
     * Returns volumes bounding box as MeshGeometry.
     *
     * @param applyTransformation if true, the bounding box
     *  is transformed into world coordinates. Otherwise,
     *  the bounding box is returned in the physical coordinates.
     *  @see getVoxelToWorldMatrix
     *
     * @note The mesh is internally created on each call.
     */
    virtual MeshGeometry getBoundingBox(bool applyTransformation = true) const;

    /// Returns the size of the cube in physical coordinates..
    virtual tgt::vec3 getCubeSize() const;

    /// Returns the lower left front in physical coordinates..
    virtual tgt::vec3 getLLF() const;

    /// Returns the upper right back in physical coordinates.
    virtual tgt::vec3 getURB() const;

    /**
     * Returns the matrix mapping from voxel coordinates (i.e. [0; dim-1])
     * to world coordinates.
     *
     * @note The matrix is internally created on each call.
     */
    virtual tgt::mat4 getVoxelToWorldMatrix() const;

    /**
     * Returns the matrix mapping from world coordinates
     * to voxel coordinates (i.e. [0; dim-1]).
     *
     * @note The matrix is internally created on each call.
     */
    virtual tgt::mat4 getWorldToVoxelMatrix() const;

    /**
     * Returns the matrix mapping from world coordinates
     * to texture coordinates (i.e. [0.0; 1.0]).
     *
     * @note The matrix is internally created on each call.
     */
    virtual tgt::mat4 getWorldToTextureMatrix() const;

    /**
     * Returns the matrix mapping from texture coordinates (i.e. [0.0; 1.0])
     * to world coordinates.
     *
     * @note The matrix is internally created on each call.
     */
    virtual tgt::mat4 getTextureToWorldMatrix() const;

    virtual tgt::mat4 getVoxelToPhysicalMatrix() const;
    virtual tgt::mat4 getPhysicalToVoxelMatrix() const;

    virtual tgt::mat4 getPhysicalToWorldMatrix() const;
    virtual tgt::mat4 getWorldToPhysicalMatrix() const;

    virtual tgt::mat4 getTextureToPhysicalMatrix() const;
    virtual tgt::mat4 getPhysicalToTextureMatrix() const;

    virtual tgt::mat4 getTextureToVoxelMatrix() const;
    virtual tgt::mat4 getVoxelToTextureMatrix() const;

    /**
     * Returns the origin the volume has been loaded from,
     * usually a file path.
     */
    const VolumeURL& getOrigin() const;

    /// @overload
    VolumeURL& getOrigin();

    /**
     * Sets the origin the volume has been loaded from,
     * usually a file path.
     */
    void setOrigin(const VolumeURL& origin);

    /**
     * Notifies the registered VolumeObservers about the pending
     * deletion of the Volume.
     */
    void notifyDelete();

    /**
     * Notifies the registered VolumeObservers that a reload
     * of the volume was done.
     */
    void notifyReload();

    template<class T>
    void derivedDataThreadFinished(VolumeDerivedDataThreadBase* ddt) const;
protected:
    template<class T>
        void addDerivedDataInternal(T* data) const;

    template<class T>
        void removeDerivedDataInternal() const;

    void clearFinishedThreads();
    void stopRunningThreads();

    VolumeURL origin_;

    mutable std::set<VolumeDerivedData*> derivedData_; // use mutex derivedDataMutex_ to make derived data thread safe!
    mutable boost::mutex derivedDataMutex_;

    mutable std::set<VolumeDerivedDataThreadBase*> derivedDataThreads_;
    mutable std::set<VolumeDerivedDataThreadBase*> derivedDataThreadsFinished_;
    mutable boost::mutex derivedDataThreadMutex_;

    static const std::string loggerCat_;
};

template <class T>
const T* VolumeBase::getRepresentation() const {
    if (getNumRepresentations() == 0) {
        LWARNING("Found no representations for this volumehandle!" << this);
        return 0;
    }

    //Check if rep. is available:
    for (size_t i=0; i<getNumRepresentations(); i++) {
        if (dynamic_cast<const T*>(getRepresentation(i))) {
            return static_cast<const T*>(getRepresentation(i));
        }
    }

    // try to convert from VolumeRAM
    ConverterFactory fac;
    if (hasRepresentation<VolumeRAM>()) {
        const VolumeRAM* volumeRam = getRepresentation<VolumeRAM>();
        RepresentationConverter<T>* converter = fac.findConverter<T>(volumeRam);
        if (converter) {
            const T* rep = static_cast<const T*>(useConverter(converter)); //we can static cast here because we know the converter returns T*
            if (rep)
                return rep;
        }

    }

    // try to convert from other representations
    for (size_t i=0; i<getNumRepresentations(); i++) {
        RepresentationConverter<T>* converter = fac.findConverter<T>(getRepresentation(i));
        if (converter) {
            const T* rep = static_cast<const T*>(useConverter(converter)); //we can static cast here because we know the converter returns T*
            if (rep)
                return rep;
        }
    }

    LERROR("Failed to create representation of the requested type!");
    return 0;

}

template <>
VRN_CORE_API const VolumeRAM* VolumeBase::getRepresentation() const;

/**
 * Class for handling different types and needs for volumes.
 *
 * Besides the hardware volume
 * this class holds information about the volume's origin, modality and timestep.
 * It is designed for being the only class which has to take care of
 * what kind of hardware volumes are used.
 */
class VRN_CORE_API Volume : public VolumeBase {
public:
    /**
     * Constructor.
     *
     * @note No hardware specific volume data like VolumeGL are created initially. If you want
     *  to use hardware specific volume data / textures, call getRepresentation<T>() with the desired type.
     *
     * @param   volume  The volume data for this Volume.
     */
    Volume(VolumeRepresentation* const volume, const tgt::vec3& spacing, const tgt::vec3& offset, const tgt::mat4& transformation = tgt::mat4::identity);
    ///Copy metadata from other volumehande:
    Volume(VolumeRepresentation* const volume, const VolumeBase* vh);
    Volume(VolumeRepresentation* const volume, const MetaDataContainer* mdc);
    Volume(VolumeRepresentation* const volume, const MetaDataContainer* mdc, const std::set<VolumeDerivedData*>& derivedData);
private:
    Volume(const Volume&) {} // private copy-constructor to prevent copying, use clone() instead.
public:

    /**
     * Delete all Volume pointers and the hardware specific ones, if they have been generated.
     */
    virtual ~Volume();

    /**
     * Gives up ownership of associated volumes without deleting them.
     * Calls this in order to prevent deletion of the volumes on destruction
     * of the handle.
     */
    void releaseVolumes();

    /**
     * (Re)Sets the volume for this handle and deletes the previous one.
     * Usually there should be no need for using this method as the volume
     * is initialized within the ctor, but some VolumeReaders need to modify
     * the read data.
     */
    void setVolume(VolumeRAM* const volume);

    /**
     * Returns a container storing the meta data items
     * attached to this volume.
     */
    virtual const MetaDataContainer& getMetaDataContainer() const;

    /**
     * @overload
     */
    virtual MetaDataContainer& getMetaDataContainer();

    virtual std::vector<std::string> getMetaDataKeys() const {
        return metaData_.getKeys();
    }

    virtual const MetaDataBase* getMetaData(const std::string& key) const {
        return metaData_.getMetaData(key);
    }

    virtual bool hasMetaData(const std::string& key) const {
        return metaData_.hasMetaData(key);
    }

    template<typename T, typename U>
    void setMetaDataValue(const std::string& key, U value) {
        MetaDataContainer& mdc = getMetaDataContainer();
        if(mdc.hasMetaData(key)) {
            MetaDataBase* mdb = mdc.getMetaData(key);
            T* md = dynamic_cast<T*>(mdb);

            if(md)
                md->setValue(value);
            else {
                LWARNING("MetaData type mismatch! Replacing.");
                mdc.removeMetaData(key);

                T* md = new T();
                md->setValue(value);
                mdc.addMetaData(key, md);
            }
        }
        else {
            T* md = new T();
            md->setValue(value);
            mdc.addMetaData(key, md);
        }
    }

    /**
     * Reloads the volume from its origin, usually from the
     * hard disk, and regenerates the dependent hardware volumes.
     *
     * @note The Volume object as well as the dependent hardware volume objects
     *       are replaced during this operation.
     *
     * After a successful reload, volumeChanged() is called on the registered observers.
     * In case the reloading failed, the Volume's state remains unchanged.
     *
     * @return true, if the volume could be successfully reloaded.
     */
    bool reloadVolume();

    ///Set the MD5 hash. Should only be called by a reader.
    virtual void setHash(const std::string& hash) const;

    virtual void addRepresentation(VolumeRepresentation* rep) {
        //TODO: check for duplicates using RTI
        representations_.push_back(rep);
    }

    virtual void removeRepresentation(size_t i) {
        if (i >= representations_.size())
            return;

        stopRunningThreads();
        delete representations_.at(i);
        representations_.erase(representations_.begin() + i);
    }

    template<class T>
    void removeRepresentation() {
        stopRunningThreads();

        //Check if rep. is available:
        for(size_t i=0; i<representations_.size(); i++) {
            T* test = dynamic_cast<T*>(representations_[i]);

            if(test) {
                representations_.erase(representations_.begin() + i);
                delete test;
            }
        }
    }

    ///Delete all other representations.
    template<class T>
    void makeRepresentationExclusive() {
        if(!hasRepresentation<T>()) {
            //we would be without representations if we delete all...
            if(!VolumeBase::getRepresentation<T>())
                return;
        }
        stopRunningThreads();

        for(size_t i=0; i<representations_.size(); i++) {
            T* test = dynamic_cast<T*>(representations_[i]);

            if(!test) {
                delete representations_[i];
                representations_.erase(representations_.begin() + i);
            }
        }
    }

    virtual const VolumeRepresentation* useConverter(const RepresentationConverterBase* converter) const {
        for(size_t i=0; i<representations_.size(); i++) {
            if(converter->canConvert(representations_[i])) {
                VolumeRepresentation* rep = converter->convert(representations_[i]);

                if(rep) {
                    representations_.push_back(rep);
                    return rep;
                }
            }
        }
        return 0;
    }

    virtual size_t getNumRepresentations() const {
        return representations_.size();
    }

    virtual const VolumeRepresentation* getRepresentation(size_t i) const {
        return representations_[i];
    }

    template <class T>
    const T* getRepresentation() const {
        return VolumeBase::getRepresentation<T>();
    }

    template <class T>
    T* getWritableRepresentation() {
        stopRunningThreads();
        T* rep = const_cast<T*>(getRepresentation<T>());
        makeRepresentationExclusive<T>();
        clearDerivedData();
        return rep;
    }

    void deleteAllRepresentations() {
        stopRunningThreads();
        while(!representations_.empty()) {
            delete representations_.back();
            representations_.pop_back();
        }
    }

    void releaseAllRepresentations() {
        stopRunningThreads();
        representations_.clear();
    }

    /// Specifies the voxel dimensions of the volume.
    virtual void setSpacing(const tgt::vec3 spacing);

    virtual void setOffset(const tgt::vec3 offset);

    virtual void setPhysicalToWorldMatrix(const tgt::mat4& transformationMatrix);

    void setModality(Modality modality);
    void setRealWorldMapping(RealWorldMapping rwm);

    /**
     * Sets the timestep for this Volume.
     */
    void setTimestep(float timestep);

protected:
    mutable std::vector<VolumeRepresentation*> representations_;

    MetaDataContainer metaData_;

    static const std::string loggerCat_;

private:
    friend class KeyValueFactory;
};


//---------------------------------------------------------------------------
// template definitions

template <class T>
void VolumeDerivedDataThread<T>::notifyVolume() {
    volume_->derivedDataThreadFinished<T>(this);
}

//---------------------------------------------------------------------------

template<class T>
T* VolumeBase::getDerivedData() const {
    T* test = hasDerivedData<T>();
    if(test)
        return test;

    // Look for running threads calculating this type of derived data:
    derivedDataThreadMutex_.lock();
    for (std::set<VolumeDerivedDataThreadBase*>::iterator it=derivedDataThreads_.begin(); it!=derivedDataThreads_.end(); ++it) {
        if (typeid(**it) == typeid(VolumeDerivedDataThread<T>)) {
            VolumeDerivedDataThreadBase* ddt = *it;
            derivedDataThreadMutex_.unlock();
            if(ddt->isRunning())
                ddt->join();
            return hasDerivedData<T>();
        }
    }

    // no running thread...start a new one:
    VolumeDerivedDataThread<T>* thread = new VolumeDerivedDataThread<T>();
    derivedDataThreads_.insert(thread);
    derivedDataThreadMutex_.unlock();

    thread->startThread(this);

    thread->join(); // ...and wait for it to finish
    return hasDerivedData<T>();
}

template<class T>
T* VolumeBase::getDerivedDataThreaded() const {
    T* test = hasDerivedData<T>();
    if(test)
        return test;

    // Look for running threads calculating this type of derived data:
    derivedDataThreadMutex_.lock();
    for (std::set<VolumeDerivedDataThreadBase*>::iterator it=derivedDataThreads_.begin(); it!=derivedDataThreads_.end(); ++it) {
        if (typeid(**it) == typeid(VolumeDerivedDataThread<T>)) {
            derivedDataThreadMutex_.unlock(); // there is a thread calculating this type of derived data
            return 0;
        }
    }

    VolumeDerivedDataThread<T>* thread = new VolumeDerivedDataThread<T>();
    thread->startThread(this);
    derivedDataThreads_.insert(thread);
    derivedDataThreadMutex_.unlock();

    return 0;
}

template<class T>
void VolumeBase::derivedDataThreadFinished(VolumeDerivedDataThreadBase* ddt) const {
    VolumeDerivedData* result = ddt->getResult();
    if (result)
        addDerivedDataInternal<T>(static_cast<T*>(result));

    derivedDataThreadMutex_.lock();
    derivedDataThreads_.erase(ddt);
    derivedDataThreadMutex_.unlock();

    // notify observers
    std::vector<VolumeObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->derivedDataThreadFinished(this, result);

    derivedDataThreadMutex_.lock();
    derivedDataThreadsFinished_.insert(ddt);
    derivedDataThreadMutex_.unlock();
}


template<class T>
T* VolumeBase::hasDerivedData() const {
    derivedDataMutex_.lock();
    for (std::set<VolumeDerivedData*>::const_iterator it=derivedData_.begin(); it!=derivedData_.end(); ++it) {
        if (typeid(**it) == typeid(T)) {
            T* tmp = static_cast<T*>(*it);
            derivedDataMutex_.unlock();
            return tmp;
        }
    }
    derivedDataMutex_.unlock();
    return 0;
}

template<class T>
void VolumeBase::addDerivedDataInternal(T* data) const {
    if (!dynamic_cast<VolumeDerivedData*>(data)) {
        LERROR("derived data item is not of type VolumeDerivedData");
        throw std::invalid_argument("passed data item is not of type VolumeDerivedData");
    }

    if (hasDerivedData<T>())
        removeDerivedDataInternal<T>();

    derivedDataMutex_.lock();
    derivedData_.insert(static_cast<VolumeDerivedData*>(data));
    derivedDataMutex_.unlock();
}

template<class T>
void VolumeBase::removeDerivedDataInternal() const {
    if (!hasDerivedData<T>())
        return;

    derivedDataMutex_.lock();
    //T* data = getDerivedData<T>();
    for (std::set<VolumeDerivedData*>::iterator it=derivedData_.begin(); it!=derivedData_.end(); ++it) {
        if (dynamic_cast<T*>(*it)) {
            delete *it;
            derivedData_.erase(it);
            derivedDataMutex_.unlock();
            return;
        }
    }
    derivedDataMutex_.unlock();
}

/*
 * Position volume centered at (0,0,0), max edge length = 1
 * WARNING: Destroys correct spacing!
 */
void VRN_CORE_API oldVolumePosition(Volume* vh);

///Center volume in world coordinates by modifying the offset.
void VRN_CORE_API centerVolume(Volume* vh);

} // namespace

#endif // VRN_VOLUME_H
