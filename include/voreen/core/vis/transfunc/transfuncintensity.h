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

#ifndef VRN_TRANSFUNCINTENSITYKEYS_H
#define VRN_TRANSFUNCINTENSITYKEYS_H

#include "tgt/gpucapabilities.h"
#include "tgt/vector.h"

#include "voreen/core/vis/transfunc/transfunc.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

namespace voreen {

class TransFuncMappingKey;
/*
class TransFuncIntensity : public TransFunc {
    virtual std::string getShaderDefines();
};
*/
//---------------------------------------------------------------------------

/**
 * One dimensional transfer function based on intensity value.
 * \sa TransFuncMappingKey, TransFuncIntensity
 */
class TransFuncIntensity : public TransFunc {
public:
    /** Tries to generate Texture of size width, testing for max. tex size, NPOT support, etc.
     * Warning: Needs opengl context!
     * \param width The resulting texture width
     */
    TransFuncIntensity(int width = 256);

    /** Copy constructor.
     * Warning: Needs opengl context!
     * \param tf The adress of the TransFuncIntensity instance, which is to be copied.
     */
    TransFuncIntensity(const TransFuncIntensity& tf);
    
    virtual ~TransFuncIntensity();

    /**
     * Generates a texture with the appropriate width.
     * \param width The width of the texture being created
     */
    void createTex(int width);

    /**
     * Creates a default function.
     * Generates two keys:
     * One at intensity 0 with the color (0,0,0,0) ; 
     * another one at intensity 1 with the color (255,255,255,255)
     */
    void createStdFunc();

    /**
     * Creates the default function for alpha.
     * Generates two keys:
     * One at intensity 0 with the color (91 173 255 255)
     * another one at intensity 1 with the color (91 173 255 255)
     */
    void createAlphaFunc();

    /**
     * Calculates the average for the segment [segStart,segEnd).
     * \param segStart The start of the segment
     * \param segEnd The end of the segment; the value itself is not part of the calculation
     * \return A vector containing the average
     */
    virtual tgt::vec4 getMeanValue(float segStart, float segEnd) const;

    /**
     * Returns the value to which the input value is being mapped.
     * The procedures handles missing keys and out-of-range values gracefully.
     * \param value The intensity value for which the mapping is requested
     * \return The value the input value is mapped to.
     */
    tgt::col4 getMappingForValue(float value) const;
    
    /**
     * Re-generates the texture from the already existing keys.
     * If the texture was empty before the call, a new texture is created automatically
     */
    virtual void updateTexture();

    /**
     * Re-generates the texture from the already existing keys, but restricts the output to an
     * area between leftValue and rightValue. That means, the transfer function will be limited
     * to this area.
     * \param leftValue The lower boundary
     * \param rightValue The upper boundary
     * \sa updateTexture()
     */
    virtual void updateTexture(int leftValue, int rightValue);

    /**
     * Returns the number of keys in this transfer function.
     * \return the number of keys
     */
    int getNumKeys() const;

    /**
     * Returns the key at i-th position. Keys are sorted by their intensities in ascending order.
     * If a value outside of [0, getNumKeys] is passed, it will be clamped to the appropirate values.
     * \param the i-th key will be returned
     * \return the pointer to the appropriate key
     */
    TransFuncMappingKey* getKey(int i) const;

    /**
     * Returns all keys. Keys are sorted by their intensities in ascending order.
     * \return a vector containing all the keys
     */
    const std::vector<TransFuncMappingKey*> getKeys() const;

    /**
     * Adds a key to the property mapping function.
     * It will be automatically inserted into the correct position.
     * \param The key to be added.
     */
    void addKey(TransFuncMappingKey* key);

    /**
     * Updates a key within the property mapping function.
     * Call this method when intensity of a key is changed.
     * \param The key to be updated.
     */
    void updateKey(TransFuncMappingKey* key);

    /**
     * Remove a key from the property mapping function.
     * Also deletes the passed key.
     * \param The key to be removed.
     */
    void removeKey(TransFuncMappingKey* key);

    /**
     * Removes all keys from the property mapping function.
     */
    void clearKeys();

    /**
     * This method returns whether the mapping function is not constant,
     * that is if any other key is not the same as the first one.
     * \return Whether the mapping is significant.
     */
    bool isSignificant() const;

    /**
     * This method returns whether the mapping function is empty.
     * i.e., it contains no mapping keys.
     * \return Is the mapping function empty?
     */
    bool isEmpty() const;

    virtual std::string getShaderDefines();

    /**
     * Saves the mapping function as a xml file.
     * Use extension .tfi as extension.   Any data in the file will be overwritten
     * \param filename The location of the file
     * \return true, if the operation was successfull ; false otherwise
     */
    bool save(const std::string& filename);

    /**
     * Saves TF as png image.
     * Any data in the file will be overwritten.
     * \param width The width of the resulting png-images
     * \return true, if the operation was successful ; false otherwise
     */
    bool savePNG(const std::string& filename, int width);

    /**
     * The central entry point for loading a transfer function. The file extension is extracted
     * and based on that, explicit, private or protected load-procedures are called.
     * If there is no extension, loading will be unsuccessful.
     *
     * Currently supported extensions include:
     * tfi , lut , table , plist , bmp , png
     *
     * \param filename The filename, which should be opened
     * \return true, if loading succeeds ; false otherwise
     * \sa loadXML(),loadImageJ(), loadTextTable(), loadOsirixCLUT()
     */
    bool load(const std::string& filename);

protected:
    /**
     * The internal representation of the set of keys
     */
    std::vector<TransFuncMappingKey*> keys_;
    static const std::string loggerCat_;

    /**
     * Loads a transfer function out of an ordinary image file. For this method, DevIL is required.
     * \param filename The filename, which should be opened
     * \return true, if loading was successful ; false otherwise
     */
    bool loadWithDevIL(const std::string& filename);

    /**
     * Loads an mapping function from an xml file.
     * \param filename The location of the file, which should be opened
     * \return true, if the operation was successfull ; false otherwise
     */
    bool loadFromXML(const std::string& filename);

    /**
     * Loads a transfer function from a text file.
     * This format is used by Klaus Engel in his preintegration volume renderer.
     * Its just 256 rows of 4 entries each. -> RGBA
     * \param filename The filename of the text table
     * \return true, if the operation was successful ; false otherwise
     */
    bool loadTextTable(const std::string& filename);

    /**
     * Loads a transfer function from an Osirix CLUT file.
     * Osirix CLUT doesn't support an alpha channel. So all values will be set opaque.
     * \param filename The filename of the CLUT file
     * \return true, if the operation was successful, false otherwise
     */
    bool loadOsirixCLUT(const std::string& filename);

    /**
     * Loads a transfer function from a LUT used by ImageJ (http://rsbweb.nih.gov/ij/)
     * Those Lookup-Tables might come in three different kinds:
     * i)   binary LUT's coming from the National Institutes of Health (NIH)
     *      those files include a header containing additional information about the table
     * ii)  binary LUT's saved in a raw format, lacking additional data.
     * iii) a LUT in a simple text format, also with a 'missing' header
     *
     * \param filename The sourcefile which is to be opened
     * \return true, if the load was successful ; false otherwise
     */
    bool loadImageJ(const std::string& filename);

    /**
    * Opens a binary LUT-File. There a two possibilities for a binary file; raw and NIH
    * the NIH type includes additional data about version information and the number of colors.
    * No alpha-channel information is included in a LUT file. All values will be opaque.
    * 
    * \param fileStream The already opened file stream used to extract the data
    * \param raw Should the file be treated as raw data?
    * \return 256 if the load was successful,  0 otherwise
    */
    int openImageJBinary(std::ifstream& fileStream, bool raw);

    /**
     * Opens a LUT file containing textual information. Currently, two types are supported;
     * i)  256 rows , 3 columns  ; one column for each color
     * ii) 256 rows , 4 columns  ; the first column is an index from 0 to 255. The others like i)
     * Each entry should be seperated by a tabstop.
     * In both cases rows beginning with a non-integer character will be ignored.
     * No alpha-channel information is included in a the file. All values will be opaque.
     *
     * \param fileStream The already opened file stream used to extract the data
     * \return 256 if the load was successful,  0 otherwise
     */
    int openImageJText(std::ifstream& fileStream);

    /**
     * This method generates keys out of the given data.
     * The given data has to have the shape RGBA, otherwise the creation will be unsuccessful.
     * The method extrapolates the extrema of the colorchannels and puts a key in those places,
     * where the difference between neighboring entries is not linear.
     *
     * \param data An array of width*4 entries of bytes
     * \param width How many pairs of RGBA are given? width must be >= 2
     */
    void generateKeys(char* data, int width = 256);

    /** 
     * Used not only to extract neccessary information but also to fast-forward through the header
     * Buffersize is given static, because the procedure shouldn't be edited if 64bit compiler is used
     * (and therefor the sizes i.e. int, double might change)
     * These functions are 'inline', because it might be faster if they are inlined by the compiler
     * (okay, not quite _that_ convincing i admit).
     */
    inline int readInt(std::ifstream& stream);

    /**
     * \sa TransFuncIntensity::readInt(std::ifstream& stream)
     */
    inline short readShort(std::ifstream& stream);

    /**
     * \sa TransFuncIntensity::readInt(std::ifstream& stream)
     */
    inline double readDouble(std::ifstream& stream);
};
}
#endif //VRN_TRANSFUNCINTENSITYKEYS_H
