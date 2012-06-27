#ifndef VRN_CACHEINDEX_H
#define VRN_CACHEINDEX_H

#include "voreen/core/vis/identifier.h"
#include "voreen/core/xml/serializable.h"

#include <map>
#include <string>

namespace voreen {

class Port;
class Processor;
class LocalPortMapping;

/**
 * Class for managing files for cache entries and storing/loading a database-like
 * representation into/from a XML-file 'cacheindex.xml'.
 * The underlying data of the cache entries are the names of the files where
 * the cached data can be found on hard-disk, associated with <i>keys</i>.
 * An object in the cache is associated
 * with a <i>key</i> which consists of the concatenation of a <i>top-level key</i>
 * and a <i>sub-level key</i>, separated by a dot ("."). The cached data are written
 * to disk by the <code>Cache<T></code> class,
 * but the file name is mapped to that key by <code>CacheIndex</code>.
 * Besides, CacheIndex checks the existence of the directory the cache uses,
 * and creates it if necessary.
 * CacheIndex is designed using the singleton design pattern, i.e. that there
 * is only one CacheIndex, managing all cached objects. Destinctions between
 * different kinds of object may be made by the key, 
 *
 * The <code>CacheIndex</code> has a two-level hierarchy: it holds 0..*
 * <code>CacheIndexEntry</code> objects, which on their parts consist of a 
 * top-level key and may contain 0..* objects of the 
 * <code>CacheIndexSubEntry</code> class.
 * <code>CacheIndexSubEntry</code> objects hold one <i>sub-level key</i>
 * (<i>sub-keys</i>) each and the actual file name for the cached data.
 */
class CacheIndex {
public:
    /**
     * <code>IndexKey</code> contains the top-level key at first position of the pair,
     * and the sub-level key at first position.
     */
    typedef std::pair<std::string, std::string> IndexKey;

private:
    /**
     * A <code>CacheIndexEntry</code> contains conceptionally of one top-level key and
     * multiple <code>CacheIndexSubEntry</code> objects. The top-level key is generated 
     * by the attributes.
     */
    class CacheIndexEntry : public Serializable {
    friend class CacheIndex;    // The only class which needs full access to this one.

    public:
        /**
         * Functor used to sort the CacheIndexEntry objects in ascending order via 
         * std::priority_queue taking pointers. The operator> has been overloaded
         * for CacheIndexEntry class.
         */
        struct EntryComparator {
            bool operator()(const CacheIndexEntry* const lhs, const CacheIndexEntry* const rhs) const {
                if ((lhs == 0) || (rhs == 0 ))
                    return false;

                return (*lhs > *rhs);
            }
        };

    private:
        /**
         * <code>CacheIndexSubEntry</code> objects conceptually map one <i>sub-level key</i>
         * to one file name in which the cache data are stored on hard disk.
         */
        class CacheIndexSubEntry : public Serializable {
        friend class CacheIndexEntry;   // The only class which needs full access to this one

        public:
            /**
             * Functor used to sort CacheIndexSubEntry object in ascending order via
             * std::priority_queue which takes CacheIndexSubEntry*.
             */
            struct SubEntryComparator {
                bool operator()(const CacheIndexSubEntry* const lhs, const CacheIndexSubEntry* const rhs) const {
                    if ((lhs == 0) || (rhs == 0 ))
                        return false;

                    return (*lhs > *rhs);
                }
            };

        public:
            /**
             * Ctor. All parameters should not be empty, but they may. If any of the string
             * passed as parametersis empty, the entire CacheIndex would probably not work
             * as expected.
             *
             * @param   processorState  The string representation of the internal state of
             *                          the concerned processor.
             * @param   porcessorInputConfig    The string representation of the configuration 
             *                                  of the inports of the concerned processor.
             * @param   filename    The name of the file under which the cached data are
             *                      stored to hard disk.
             */
            CacheIndexSubEntry(const std::string& processorState, 
                const std::string& processorInportConfig, const std::string& filename);

            /**
             * Ctor for convenience intializing the members from the passed TiXmlElement*. 
             * Internally, <code>updateFromXml()</code> is called.
             * If the argument is NULL or some of the xml attributes are invalid, the 
             * object may remain partially undefinied and cause the CacheIndex not to 
             * work as expected.
             */
            CacheIndexSubEntry(TiXmlElement* const xml);

            /**
             * Returns the name of the file which is held by this object.
             * @return  Name of the file which stores a cached object on hard disk.
             */
            const std::string& getFilename() const { return filename_; }

            /**
             * Generates the sub-key from the object based on its attributes.
             * The format of this key defined as follows:
             *
             * "InportConfig{" + processorInputConfig_ + "}.State{" + processorState_ + "}"
             *
             * @return  The sub-key used to identify this object within the CacheIndexEnty
             *          containing it and which is used as the second part of
             *          <code>CacheIndex::IndexKey</code>
             */
            std::string makeKey() const;

            /**
             * Inherited from <code>Serializable</code>.
             */
            std::string getXmlElementName() const { return "CacheIndexSubEntry"; }

            /**
             * Inherited from <code>Serializable</code>.
             */
            virtual TiXmlElement* serializeToXml() const;

            /**
             * Inherited from <code>Serializable</code>.
             */
            virtual void updateFromXml(TiXmlElement* elem);
            
            /**
             * CacheIndexSubEntry objects are defined to be greater to other
             * ones if their respective reference counters are greater compared
             * to the each other, or if the reference counters are equal, when
             * the times of the object's creations are greater.
             */
            bool operator>(const CacheIndexSubEntry& rhs) const {
                if (refCounter_ != rhs.refCounter_)
                    return (refCounter_ > rhs.refCounter_);
                else
                    return (time_ > rhs.time_);
            }
            
        private:
            /** 
             * The string representation of the internal state of the concerned 
             * processor
             */
            std::string processorState_;

            /**
             * The string representation of the configuration of the inports of the
             * concerned processor. This can be regarded as a conceptual part of the 
             * processor's state.
             */
            std::string processorInportConfig_;

            /** The file name for the file containing the cached data. */
            std::string filename_;

            /** The unix timestamp of the time when this object has been created. */
            unsigned long time_;

            /** 
             * Reference counter which is increased everty time, the dataset associated 
             * with this CacheIndexSubEntry objectis accessed by the cache.
             */
            size_t refCounter_;
        };

    public:
        /**
         * Convenience ctor copying the values from the passed object, except
         * its CacheIndexSubEntry objects. These are taken from the passed
         * second argument.
         * @param   cie 'template' for creation of the new object
         * @param   subEntries  The CacheIndexSubEntry objects which will be used
         *                      instead of the ones from cie.
         */
        CacheIndexEntry(const CacheIndexEntry& cie, 
            const std::vector<CacheIndexSubEntry>& subEntries);

        /**
         * Ctor. None of the passed strings should be empty, or the entire
         * CacheIndex using this object would probably not work as expected.
         *
         * @param   processorClassName  Name of the processor class for which
         *                              an entry is made.
         * @param   processorName   Name of a certain processor object of the
         *                          class in processorClassName.
         * @param   portName    Name of the port, including its 'type' which
         *                      is concerned by the caching. This is usually
         *                      identical to the ports identifier, e.g.
         *                      "volumhandle.input"
         * @param   objectClassName Name of the class the cached object is an 
         *                          instance of.
         */
        CacheIndexEntry(const std::string& processorClassName, 
            const std::string& processorName, const std::string& portName, 
            const std::string& objectClassName);

        /**
         * Ctor for convenience intializing the members from the passed TiXmlElement*. 
         * Internally, <code>updateFromXml()</code> is called.
         * If the argument is NULL or some of the xml attributes are invalid, the 
         * object may remain partially undefinied and cause the CacheIndex not to 
         * work as expected.
         */
        CacheIndexEntry(TiXmlElement* const xml);

        /**
         * Returns the file name stored in a CacheIndexSubEntry object with
         * the given key, if one exists. Otherwise the returned string is
         * empty.
         *
         * @param   subKey  The sub-key of the CacheIndexSubEntry object held
         *                  by this entry. This parameter may be an empty string,
         *                  but it should not.
         * @return  The name of the file which has been associated for the given
         *          sub-key, if it exists, or an empty string otherwise.
         */
        std::string findFilename(const std::string& subKey);

        /**
         * Return all stored CacheIndexSubEntry objects of this object.
         */
        std::vector<CacheIndexEntry::CacheIndexSubEntry> getAllSubEntries();

        /**
         * Increase the reference counter for the CacheIndexSubEntry object
         * associated with the given sub-key, if it exists.
         */
        void incrementRefCounter(const std::string& subKey);

        /**
         * Inserts the passed CacheIndexSubEntry object into this object.
         *
         * @return  true if the insertion was successful, false otherwise.
         */
        bool insert(const CacheIndexSubEntry& cid);

        /**
         * Convenience method inserted a new CacheIndexSubEntry object, generated
         * from the passed arguments, into this object and returning the sub-key
         * which has been generated for that new object.
         *
         * @param   processorState  The string representation of the internal state of
         *                          the concerned processor.
         * @param   porcessorInputConfig    The string representation of the configuration 
         *                                  of the inports of the concerned processor.
         * @param   filename    The name of the file under which the cached data are
         *                      stored to hard disk.
         *
         * @return  Sub-key of the created CacheIndexSubEntry object, if the insertion
         *          was successful, of en empty string otherwise.
         */
        std::string insert(const std::string& processorState, 
            const std::string& processorInportConfig, const std::string& filename);

        /**
         * Returns the top-level key generated for this object from its attributes.
         * This is also the first part of <code>CacheIndex::IndexKey</code>.
         * The format of the top-level key is definied as follows:
         *
         * "Processor{" + processorClassName_ + "}.Outport{" + portName_ + "}"
         *
         * @return Top-level key in the format above, used to identify this 
         *          CacheIndexEntry object within the <code>CacheIndex</code>.
         */
        std::string makeKey() const;

        /**
         * Checks whether a CacheIndexSubEntry object of the passed sub-key
         * exisits within this object.
         *
         * @return  true, if an object exists for the passed key, false otherwise.
         */
        bool subEntryExists(const std::string& subKey) const;

        /**
         * Inherited from <code>Serializable</code>.
         */
        virtual std::string getXmlElementName() const { return "CacheIndexEntry"; }

         /**
          * Inherited from <code>Serializable</code>.
          */
        virtual TiXmlElement* serializeToXml() const;

        /**
         * Inherited from <code>Serializable</code>.
         */
        virtual void updateFromXml(TiXmlElement* elem);

        /**
         * CacheIndexEntry objects are defnied to be greater to each other
         * if their respective reference counters are greater to each other.
         */
        bool operator>(const CacheIndexEntry& rhs) const { 
            return refCounter_ > rhs.refCounter_;
        }

    private:
        /**
         * Removes CacheIndexSubEntry objects from this CacheIndexEntry object until the number 
         * of data is less than the user-defined limit of data to be held for this entry. 
         * The strategy used to determine the sub entries to be removed is reference counting.
         */
        size_t freeDataRefCount();

    private:
        static const std::string loggerCat_;

        /** Name of the processor class for which this entry is created. */
        std::string processorClassName_;

        /** Name of the processor object for which this entry is created. */
        std::string processorName_;

        /**
         * Name / identifier of the port belonging to the processor for which
         * this entry is created.
         */
        std::string portName_;

        /** Name of the object class which is cached for that port and processor. */
        std::string objectClassName_;

        size_t refCounter_;

        typedef std::map<std::string, CacheIndexSubEntry> SubEntryMap;
        /**
         * Internal map holding the CacheIndexSubEntry objects and associating them
         * with their sub-keys.
         */
        SubEntryMap sub_;

        /**
         * Contains all sub entries which may have been displaced by inserting other ones,
         * if the storage limits has been reached. Then the reference counting is used to
         * determine which sub entries shall be displaced.
         * This vector and the contained objects are freed, when 
         * <code>CacheIndex::cleanup()</code> is called and therefore has to be called!
         */
        std::vector<CacheIndexSubEntry> displacedSubEntries_;
    };

public:
    /** 
     * The only possibility to access an instance of this class, for it has been
     * implemented by using the Singleton design pattern.
     */
    static CacheIndex& getInstance() {
        static CacheIndex instance_ = CacheIndex();
        return instance_;
    }

    /**
     * Dtor saving the content of this object (and especially the one of all
     * containing CacheIndexEntry objects and the therein contained 
     * CacheIndexSubEntry objects) to an XML file by calling 
     * <code>writeIndexFile()</code>.
     */
    ~CacheIndex();

    /**
     * Returns a pair of two strings containing the key of the object which
     * has to be deleted from the cache's object map on first position and
     * the name of the corresponding file which has to be deleted, too, on
     * second position.
     */
    std::vector<std::pair<std::string, std::string> > cleanup();

    /**
     * Checks whether a <code>CacheIndexEntry</code> for the given top-level key
     * exists.
     *
     * @param   top-level key which shall be found
     * @return  true, if the CacheIndexEntry exists, false otherwise
     */
    bool entryExists(const std::string& key) const;

    /**
     * Returns the file name stored in the CacheIndex for the given processor,
     * port and the input configuration of that processor port.
     *
     * @param   processor  The processor of which is concerned
     * @param   port    The port of the processor which is concerned
     * @param   inportConfig The string representation of the input configuration
     *                       of that port.
     * @return  The file name which holds the cached object for that processor
     *          and that port when the port has the given inport configuration,
     *          or and empty string, if no data are cached for that combination
     *          of processor, port and input data on that port.
     */
    std::string findFilename(Processor* const processor, Port* const port,
        const std::string& inportConfig);

    /**
     * Returns all the CacheIndexEntry which might have been displaced by insertions
     * of new CacheIndexEntry objects into the CacheIndex, and copies of the
     * CacheIndexEntry objects in which CacheIndexSubEntries might have been replaced
     * by insertions of new CacheIndexSubEntries.
     * This data are used by the <code>Cache&lt;T&gt;</code> to remove files no longer 
     * needed.
     */
    std::vector<CacheIndexEntry>& getDisplacedEntries() { return displacedEntries_; }

    /**
     * Increments the reference counter for the CacheIndexEntry and the CacheIndexSubEntry
     * associated with the respective parts of the given IndexKey. If one of those objects
     * does not exist, nothing happens.
     *
     * @param   key IndexKey (composed of top-level and sub-level) of the entries whose
     *              reference counters shall be increased to indicate an access by the 
     *              Cache.
     */
    void incrementRefCounter(const IndexKey& key);

    /**
     * Creates a new CacheIndexEntry and an encapsulated CacheIndexSubEntry object based
     * on the given parameters and inserts it. This method returns the concatenation
     * of the top-level key from the created CacheIndexEntry and the sub-leve key of
     * the created CacheIndexSubEntry, separated by a dot (".").
     *
     * @param   processor   The processor for which an entry shall be created
     * @param   port    The port on that processor for which the entry shall be created
     * @param   objectClassName Name of the class of the object which is cached.
     * @param   inportConfig    String representation of the Configuration of the inports 
     *                          of the port on the processor.
     * @param   filename    Name of the file where the cached object is stored on hard disk.
     *
     * @return  The concatenation of the top-level key from the created CacheIndexEntry and 
     *          the sub-level key of the created CacheIndexSubEntry, separated by a dot 
     *          (".") if the insertsion is successful. Otherwise an empty string ist retruned.
     */
    std::string insert(Processor* const processor, Port* const port, 
        const std::string& objectClassName, const std::string& inportConfig, const std::string& filename);

    /**
     * Generates and returns and entire key for the given configuration of processor, 
     * port and inportConfig as an <code>IndexKey</code> object, which is actually a
     * pair of strings.
     * The top-level key is located in the first position, the sub-level key in the second
     * one.
     *
     * @param   processor   Processor for which the key shall be generated.
     * @param   port    Port on that processor for which the key shall be generated.
     * @param   inportConfig    String representation of the configuration of the 
     *                          inports of that processor for key generation.
     * @return  The entire key as IndexKey object with the top-level key in the first,
     *          and the sub-level key in the second position.
     */
    static IndexKey generateCacheIndexKey(Processor* const processor, Port* const port,
        const std::string& inportConfig);

private:
    CacheIndex();   // for the Singleton design pattern, only private ctors are necessary
    CacheIndex(const CacheIndex&);              // made private to prevent exterior access
    CacheIndex& operator=(const CacheIndex&);   // made private to prevent exterior access

    /**
     * Frees the returned number of CacheIndexEntry object, if the storage thresholf for
     * them is reached. The strategy for determining which entry to displace is reference
     * counting. The displaced entries are stored in <code>displacedEntries_</code> and
     * need to be cleaned up by calling <code>cleanup()</code> what is done by the
     * <code>Cache&lt;T&gt;</code> class.
     */
    size_t freeEntriesRefCount();

    /**
     * Prepares the folder in which the cache may write and where the index file
     * cacheindex.xml will be located. If the directory does not exist, the method
     * creates it and returns the result of the folder creation.
     *
     * @return  true if the cache folder already exists or has been created successfully,
     *          false if the folder does not exist and could not be created.
     */
    bool prepareCacheFolder();

    /**
     * Reads and parses the XML index file 'cacheindex.xml' from the cache directory.
     * 
     * @return false if reading or parsing of that file failed, true otherwise.
     */
    bool readIndexFile();

    /**
     * Serializes this object to an XML file 'cacheindex.xml' in the cache directory.
     * This file is used to initialize the CacheIndex on next creation, which will be
     * probably on a new execution of the programm using this class.
     * This method will be called by the dtor, but the cache might be configured for 
     * instant writing. Then the file will be written every time the cache's content
     * changes.
     */
    bool writeIndexFile();

private:
    static const std::string loggerCat_;
    static const std::string indexFilename_;

    /**
     * Determines whether to write the index file every time th cache content changes
     * or only on destruction of this (singleton) object.
     */
    static const bool instantWrite_ = false;

    const std::string cacheFolder_;

    typedef std::map<std::string, CacheIndexEntry> EntryMap;
    EntryMap entries_;

    /**
     * Vector holding all eventually displaced entries which need to be cleaned up.
     * This is done by calling <code>cleanup</code> which is called by the method
     * of the same name in class <code>Cache&lt;T&gt;</code>.
     */
    std::vector<CacheIndexEntry> displacedEntries_;
};

}   // namespace

#endif
