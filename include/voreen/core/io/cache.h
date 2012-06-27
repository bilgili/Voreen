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

#ifndef VRN_CACHE_H
#define VRN_CACHE_H

#include "voreen/core/io/cacheindex.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

class Port;
class Processor;
class LocalPortMapping;

/**
 * This class hold all functions and member which are common to all derived cache classes
 * and do not depend on the concretely cached objects and are therefore independed from
 * template parameters. Basically, this are utility methods. 
 * Furthermode this class works as the "master control" for all caches and can globally 
 * disable the ability of caching via calls to <code>setCachingEnabled()</code>.
 *
 * @author  Dirk Feldmann, June/July 2009
 */
class CacheBase {
public:
    /**
     * Derived classed have to call this ctor in order to get "reserve" a port type and
     * to tell which kind of objects they will cache. The latter is for internal and
     * informational purpose only and could be anything, but it is strongly recommended
     * to use the real class Name or an appropriate one, e.g. 'VolumeHandle' for the
     * VolumeCaching which actually caches VolumeHandle objects.
     *
     * @param   chachedObjectsClassName The class name of the objects which will be cached
     *                                  by derived classes.
     * @param   assignedPortType    The type of ports which will be checked for data to be
     *                              cached by subclasses, e.g. 'volumehandle'.
     */
    CacheBase(const std::string& cachedObjectsClassName, const std::string& assignedPortType);

    virtual ~CacheBase() { /* dtor has nothing to do */ }

    /**
     * Returns the port type which has been assigned for derived cache classes.
     */
    const std::string& getAssignedPortType() const;

    /**
     * Returns all ports of the passed processor matching the port type which is
     * to be handled by derived classes.
     */
    std::vector<Port*> getCacheConcernedOutports(Processor* const processor) const;

    /**
     * Returns the class name of the object which can be handled from the derived
     * object.
     */
    const std::string& getCachedObjectsClassName() const;

    /**
     * Returns a textual representation of the data which are currently assigend to
     * the inports of the given processor. The format of the string depends on the
     * concrete port type and has to defined by the internal call to
     * <code></code>.
     * Not that empty strings can be returned, which is regarded as an error, 
     * indicating that there are not data on the inport.
     */
    std::string getProcessorsInportConfig(const Processor* processor, 
        LocalPortMapping* const localPortMapping) const;

    /**
     * Returns true, if the given processor is compatbile with this Cache object,
     * false otherwise.
     */
    bool isCompatible(Processor* const processor) const;

    /**
     * Returns whether caching is enable or not. The return value depends on the
     * setting set via <code>setEnabled()</code> of the object and the general
     * class setting set via <code>setCachingEnabled()</code>. If one of the
     * sets false, the return value of this function is false, too.
     */
    bool isEnabled() const;

    /**
     * Enables or disbaled caching for that particular <code>Cache</code> object.
     * Note that the object can be enabled to cache, but that it will not take
     * effect if <code>setCachingEnabled(false)</code> has been called.
     */
    void setEnabled(const bool enable);

    /**
     * Checks wether caching is enabled in general. Note that although the
     * return value might be true, a concrete cache can be disabled via
     * <code>setEnabled(false)</code>.
     */
    static bool isCachingEnabled() { return cachingEnabled_; }

    /**
     * Enables or disables the general abilitiy of caching by setting a static bool.
     * This values affects the return value of <code>isEnabled()</code>. If it is 
     * called with false, all caches will be generally disabled.
     */
    static void setCachingEnabled(const bool enable) { cachingEnabled_ = enable; }

protected:
    virtual std::string portContentToString(const Identifier& portID, 
        LocalPortMapping* const localPortMapping) const = 0;

protected:
    const std::string assignedPortType_;
    const std::string cachedObjectsClassName_;
    bool isEnabled_;

private:
    static bool cachingEnabled_;
};

// ============================================================================

/**
 * Class for loading, storing and finding cached objects. It basically defines
 * three pure virtual methods which have to be defined by a derived class for
 * caching concrete objects. All management for finding the entries for the
 * cached objects, finding, loading and deleting files and updating them
 * are done by this class. Authors of derived classes should have nothing
 * do with it, once they have determined <b>HOW</b> the cached objects have
 * to be stored and loaded from hard disk.
 *
 * @author  Dirk Feldmann, June/July 2009
 */
template<typename T>
class Cache : public CacheBase {
public:
    /**
     * Ctor with parameters which are forwarded to <code>CacheBase</code>
     */
    Cache(const std::string& cachedObjectsClassName, const std::string& assignedPortType);

    /**
     * The dtor frees all objects in the local objectMap_ if they have been loaded by
     * the cache. Objects which have not been loaded from files by the cache are
     * freed elsewhere (usually by the concerned processor or in case of VolumeHandles
     * by the VolumeSetContainer) and therefore <b>MUST NOT</b> be freed by this dtor.
     */
    virtual ~Cache();

    /**
     * For the given processor and the given port on it, this method returns
     * the data from the cache which are assigned. The method internally calls
     * <code>loadObject()</code> which has to be defined in the concerete subclass.
     * If not data have been found or the value returned from <code>loadObject()</code>
     * is equal to the return value of <code>getInvalidValue()</code>, the return value
     * of the latter will be returned, to indicate an error.
     *
     * @param   processor   The processor to be handled.
     * @param   outport The outport affected by the caching.
     * @param   localPortMapping    The current portmapping for that processor.
     *                              This is required to determine the current configuration
     *                              of the processors inports, which is regarded as logical
     *                              part of the processor's internal state.
     * @return  The data which are cached for the given parameters. A concerete class has
     *          define interpret this value. The returned value can also be the
     *          result of <code>getInvalidValue()</code>, which might depend on the concrete
     *          subclass, to indicate that no object could be found.
     */
    T find(Processor* const processor, Port* const outport, 
        LocalPortMapping* const localPortMapping);

    /**
     * Returns a value of type T which is used to indicate an error for data lookup. The
     * default value is the T representation of 0, because T can of course be a pointer.
     * If T was defined to be an int, for instance, in derived subclasses, other
     * values than 0, e.g. -1, might be suitable and this method should therefore become
     * overridden in the subclass.
     * This method is called by <code>find()</code> and <code>update()</code> and should
     * be used to compare the returned value of the former one to detect a missed look-up.
     */
    virtual T getInvalidValue() const { return T(0); }

    /**
     * Updates the state of the cache and thereby of the cache index with the data from
     * the given arguments by internally calling <code>getPortData()</code> and comparing
     * the result with the return value of <code>getInvalidValue()</code>.
     * As this facts may insert new data, and the limit for values to be cached might
     * become exceeded, this method can cause cache entries or / and sub-entries to become
     * displaced. Thererfore <code>cleanup()</code> is called to free the memory and delete
     * files which will be no longer needed.
     * If the insertion or replacement of the new data is successful, true will be returned,
     * otherwise false.
     *
     * @param   processor   The processor of which the data on ports affected by the concrete
     *                      cache subclass shall be cached. All data on all ports of the type
     *                      specified by the subclass'es ctor will be cached.
     * @param   localPortMapping    Used to determine the processors internal state.
     *
     * @return  True if the insertion / replacement of the data is successful, false otherwise.
     */
    bool update(Processor* const processor, LocalPortMapping* const localPortMapping);   
  
protected:
    /**
     * Intended to return the data from the given port mapping for the concrete
     * type T. Due to the fact that the PortMapping may map different data
     * types to the port, each cache for a concrete type has to take care of
     * obtaining the values from PortMapping itself.
     *
     * @param   portType    Name/'type' of the port of which the data shall be 
     *                      returned, e.g. 'volumehandle.outport'.
     * @param   localPortMapping    The port mapping structure for a certain
     *                              processor containing the ports actual data.
     *
     */
    virtual T getPortData(const Identifier& portType,
        LocalPortMapping* const localPortMapping) const = 0;

    /**
     * This method has to be implemented to load the object of type T from
     * the given file name.
     *
     * @param   filename    Name of the file, including the absolute path,
     *                      which shall be loaded.
     * @return  The object stored in that file.
     */
    virtual T loadObject(const std::string& filename) const = 0;

    /**
     * Method which has to save the given object to the file in the given file name
     * in the directory, specified by directory. If the file name is empty, a file
     * name has to be generated and returned. If the name is not empty, the file
     * shall assumed to be exisiting and shall be replaced.
     * The method must return either the generated name, if filename was empty, filename
     * itself, if it was not empty or an empty string, if an error has occured.
     * 
     * @param   object  The object to be saved.
     * @param   directory   The absolute path to the directory where the file will
     *                      be stored.
     * @param   filename    The name of the file in which the object will be stored.
     *                      This parameter may be an empty string (""), but then
     *                      a name (preferably unique for that kind of objects) has
     *                      to be generated and that name must be returned.
     *
     * @return  If an error occurs an empty string must be returned. Otherwise, if
     *          filename was not empty, filename must be returned. If filename was
     *          empty, the generated string must be returned.
     */
    virtual std::string saveObject(T const object, const std::string& directory,
        const std::string& filename) = 0;

protected:
    /**
     * Wrapper class to manage deletion of objects which have been loaded by the
     * cache and which therefore need to be deleted by the cache.
     */
    template<typename S = T>
    struct CacheObject {
        CacheObject(S object, const bool originsCache)
            : object_(object), originsCache_(originsCache)
        {
        }
        
        ~CacheObject() {
            if (originsCache_ == true)
                delete object_;
        }

        S object_;
        bool originsCache_;
    };

    /**
     * The real cache: maps the key used by CacheIndexEntry to the object
     * which are currently in memory. Objects which are loaded using 
     * <code>loadObject()</code> will be inserted into this map.
     */
    typedef std::map<std::string, CacheObject<T>* > ObjectMap;
    ObjectMap objectMap_;

    CacheIndex& cacheIndex_;

private:
    /**
     * This method delete all files which are not used any longer and
     * all objects from the local map by using the return values of
     * <code>CacheIndex::cleanup()</code>. It is called by 
     * <code>update()</code>.
     */
    void cleanup();

private:
    static const std::string loggerCat_;
    const std::string cacheFolder_;
};

}   // namespace

#endif
