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

#include "voreen/core/vis/transfunc/transfuncintensity.h"


#include <tinyxml/tinyxml.h>

#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

using tgt::col4;
using tgt::ivec3;
using tgt::ivec4;


namespace voreen {

const std::string TransFuncIntensity::loggerCat_("voreen.voreen.TransFunc.IntensityKeys");

TransFuncIntensity::TransFuncIntensity(int width/*=256*/)
  : TransFunc() {
	tex_ = 0;
    createTex(width);
    createStdFunc();
}

TransFuncIntensity::TransFuncIntensity(const TransFuncIntensity& tf) : TransFunc() {
    std::vector<TransFuncMappingKey*> keys_;
    for (size_t i = 0 ; i < tf.keys_.size() ; ++i) {
        TransFuncMappingKey* k = new TransFuncMappingKey(*(tf.keys_.at(i)));
        addKey(k);
    }
    
    if (tex_)
        delete tex_;

    createTex(tf.tex_->getWidth());
    updateTexture();
}

TransFuncIntensity::~TransFuncIntensity() {
    for (size_t i = 0 ; i < keys_.size() ; ++i)
        delete keys_[i];
}

void TransFuncIntensity::createTex(int width) {
    if (tex_)
        delete tex_;

    if ( !GpuCaps.isNpotSupported() ) {
        int k = 1;
        while (k < width)
            k <<= 1;
        if (k != width)
            width = k;
    }

    int maxw;
    if (GpuCaps.areSharedPalettedTexturesSupported())
        /*
            color table for the paletted textures may only have 256 entries
                (At least for GeForce 4. I didn't find a proper OpenGL getter).
        */
        maxw = 256;
    else
        maxw = GpuCaps.getMaxTextureSize();

    if (maxw < width)
        width = maxw;
    tex_ = new tgt::Texture(ivec3(width, 1, 1), GL_RGBA, (GLenum) GL_UNSIGNED_BYTE, tgt::Texture::NEAREST);
    tex_->setWrapping(tgt::Texture::CLAMP);
}


void TransFuncIntensity::createStdFunc() {
    clearKeys();
    keys_.push_back(new TransFuncMappingKey(0.0, tgt::col4(0,0,0,0)));
    keys_.push_back(new TransFuncMappingKey(1.0, tgt::col4(255,255,255,255)));
    updateTexture();
}

void TransFuncIntensity::createAlphaFunc(){
    clearKeys();
    keys_.push_back(new TransFuncMappingKey(0.0, tgt::col4(91,173,255,70)));
    keys_.push_back(new TransFuncMappingKey(1.0, tgt::col4(91,173,255,70)));
    updateTexture();
}

tgt::vec4 TransFuncIntensity::getMeanValue(float segStart, float segEnd) const {
    tgt::ivec4 result(0);
    float width = static_cast<float>(tex_->getWidth());
    for (int i = int(segStart*width) ; i < segEnd*width ; ++i)
        result += ivec4( tex_->texel<col4>(i) );

    return static_cast<tgt::vec4>(result)/(segEnd*width-segStart*width);
}

col4 TransFuncIntensity::getMappingForValue(float value) const {
    // If there are no keys, any further calculation is meaningless
    if (keys_.empty())
        return col4(0,0,0,0);

    // Restrict value to [0,1]
    value = (value < 0.0) ? 0 : value;
    value = (value > 1.0) ? 1 : value;

    // iterate through all keys until we get to the correct position
    std::vector<TransFuncMappingKey*>::const_iterator keyIterator = keys_.begin();

    while ((keyIterator != keys_.end()) && (value > (*keyIterator)->getIntensity()))
        keyIterator++;

    if (keyIterator == keys_.begin())
        return keys_[0]->getColorL();
    else if (keyIterator == keys_.end())
        return (*(keyIterator-1))->getColorR();
    else{
        // calculate the value weighted by the destination to the next left and right key
        TransFuncMappingKey* leftKey = *(keyIterator-1);
        TransFuncMappingKey* rightKey = *keyIterator;
        float fraction = (value - leftKey->getIntensity()) / (rightKey->getIntensity() - leftKey->getIntensity());
        col4 leftDest = leftKey->getColorR();
        col4 rightDest = rightKey->getColorL();
        col4 result = leftDest;
        result.r += static_cast<uint8_t>((rightDest.r - leftDest.r) * fraction);
        result.g += static_cast<uint8_t>((rightDest.g - leftDest.g) * fraction);
        result.b += static_cast<uint8_t>((rightDest.b - leftDest.b) * fraction);
        result.a += static_cast<uint8_t>((rightDest.a - leftDest.a) * fraction);
        return result;
    }
}

void TransFuncIntensity::updateTexture() {
/*    if (!tex_)
        createTex(256);
    int imageWidth = tex_->getWidth();

    for (int x = 0 ; x < imageWidth ; ++x)
        tex_->texel<col4>(x) = col4( getMappingForValue( static_cast<float>(x) / imageWidth) );
    tex_->uploadTexture();
*/
    if (!tex_)
        createTex(256);

    int imageWidth = tex_->getWidth();

    updateTexture(0, imageWidth);
    
    // updateTexture( 50 , 150);
}

void TransFuncIntensity::updateTexture(int leftValue, int rightValue) {
    if (!tex_)
        createTex(256);
    
    for (int x = leftValue ; x < rightValue ; ++x)
        tex_->texel<col4>(x) = col4( getMappingForValue( static_cast<float>(x) / rightValue) );
    
    tex_->uploadTexture();
}

int TransFuncIntensity::getNumKeys() const{
    return keys_.size();
}

TransFuncMappingKey* TransFuncIntensity::getKey(int i) const{
    return keys_.at(i);
}

const std::vector<TransFuncMappingKey*> TransFuncIntensity::getKeys() const{
    return keys_;
}

void TransFuncIntensity::addKey(TransFuncMappingKey* key) {
    // insert key at appropriate location
    if (keys_.empty()) {
        keys_.push_back(key);
        return;
    }
    std::vector<TransFuncMappingKey *>::iterator keyIterator = keys_.begin();
    // Fast-forward to the correct position
    while ((keyIterator != keys_.end()) && (key->getIntensity() > (*keyIterator)->getIntensity()))
        keyIterator++;
    keys_.insert(keyIterator, key);
}

bool sortFunction(TransFuncMappingKey* a, TransFuncMappingKey* b) {
    return a->getIntensity() < b->getIntensity();
}

void TransFuncIntensity::updateKey(TransFuncMappingKey* /*key*/) {
    std::sort(keys_.begin(), keys_.end(), sortFunction);
}

void TransFuncIntensity::removeKey(TransFuncMappingKey* key) {
    std::vector<TransFuncMappingKey *>::iterator keyIterator = find(keys_.begin(), keys_.end(), key);
    if (keyIterator != keys_.end())
        keys_.erase(keyIterator);
    delete key;
}

void TransFuncIntensity::clearKeys() {
    std::vector<TransFuncMappingKey *>::iterator keyIterator = keys_.begin();
    // First delete all the referenced objects in the heap
    while (keyIterator != keys_.end()) {
        delete (*keyIterator);
        ++keyIterator;
    }
    // then delete the entries in the vector
    keys_.clear();
}

bool TransFuncIntensity::isSignificant() const {
    // mapping cannot be significant if there is no key
    if (isEmpty())
        return false;

    // test every possible destination value against the first
    // left destination value
    uint8_t epsilon = 1;
    uint8_t oneValue = keys_[0]->getColorL().a;
    if (keys_[0]->isSplit() && (abs(keys_[0]->getColorR().a - oneValue) > epsilon))
        return true;
    for (size_t i=1 ; i < keys_.size() ; ++i) {
        if (abs(keys_[i]->getColorL().a - oneValue) > epsilon)
            return true;
        if (keys_[i]->isSplit() && (abs(keys_[i]->getColorR().a - oneValue) > epsilon))
            return true;
    }
    return false;
}

bool TransFuncIntensity::isEmpty() const {
    return keys_.empty();
}

std::string TransFuncIntensity::getShaderDefines() {
    return "#define TF_INTENSITY\n";
}

bool TransFuncIntensity::save(const std::string& filename) {
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement* root = new TiXmlElement("TransFuncIntensity");
    doc.LinkEndChild( root );

    for (int i = 0 ; i < getNumKeys() ; ++i) {
        TransFuncMappingKey* k = getKey(i);

        TiXmlElement* e = new TiXmlElement( "key" );

        e->SetDoubleAttribute("intensity", k->getIntensity());
        e->SetAttribute("split", k->isSplit());

        saveXml(e, k->getColorL());
        if (k->isSplit())
            saveXml(e, k->getColorR());
        root->LinkEndChild(e);
    }
    doc.SaveFile(filename);

    return true;
}

#ifdef VRN_WITH_DEVIL
bool TransFuncIntensity::savePNG(const std::string& filename, int width) {
    //IL does _NOT_ overwrite files by default
    ilEnable(IL_FILE_OVERWRITE);
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);

    GLubyte* im = new GLubyte[width*4];

    for (int x = 0 ; x < width ; ++x) {
        tgt::col4 c = getMappingForValue( static_cast<float>(x)/width );
        im[(x*4)+0] = c.r;
        im[(x*4)+1] = c.g;
        im[(x*4)+2] = c.b;
        im[(x*4)+3] = c.a;
    }

    ilTexImage(width, 1, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, im);
    ilSave(IL_PNG, (ILstring)filename.c_str());

    ilDeleteImages(1, &img);
    delete[] im;
    return true;
}

#else
bool TransFuncIntensity::savePNG(const std::string& /*filename*/, int /*width*/) {
    LERROR("Saving as PNG failed: No DevIL support.");
    return false;
}
#endif // VRN_WITH_DEVIL

bool TransFuncIntensity::load(const std::string& filename) {
    // Extract the file extension 
    std::string fileExtension;
    size_t dotPosition = filename.rfind(".");
    if (dotPosition != std::string::npos)
        // => the last (seperating) dot was found
        fileExtension = filename.substr(dotPosition+1);
    else
        return false;

    // switch-statement is not possible, because the compiler doesn't allow a switch by std::string
    // If further extensions shall be added, the pattern should be obvious.
    if (fileExtension.compare("tfi") == 0)
        return loadFromXML(filename);
    if (fileExtension.compare("lut") == 0)
        return loadImageJ(filename);
    if (fileExtension.compare("table") == 0)
        return loadTextTable(filename);
    if (fileExtension.compare("plist") == 0)
        return loadOsirixCLUT(filename);
    // "else"   
    #ifdef VRN_WITH_DEVIL
        return loadWithDevIL(filename);
    #endif
    
    return false;
}

#ifdef VRN_WITH_DEVIL
bool TransFuncIntensity::loadWithDevIL(const std::string& filename) {
    ILuint imageID;
    ILboolean success;
    
    // Load and bind the image
    ilGenImages(1 , &imageID);
    ilBindImage(imageID);
    success = ilLoadImage(const_cast<char*>(filename.c_str()));

    // If the load was unsuccessful => jump out of the method
    if (!success)
        return false;

    // Convert every color component to unsigned byte value
    success = ilConvertImage(IL_RGB , IL_UNSIGNED_BYTE);

    // If the conversion was unsuccessful => jump out of the method
    if (!success)
        return false;

    // Get the width of the image file
    int imageWidth = ilGetInteger(IL_IMAGE_WIDTH);

    // Get a pointer onto the color component data. Although all rows of the image are accessible,
    // we'll need only the first row. The rest will be ignored
    ILubyte* imageData = ilGetData();

    // Now, we have to add an alpha value of 255 at every fourth position
    // In order to do this, we'll go through both arrays at different paces and add the
    // required 255 in the 'newData' array
    ILubyte* newImageData = new ILubyte[imageWidth*4];
    for (int i = 0 ; i < imageWidth ; ++i) {
        newImageData[i*4 + 0] = imageData[i*3 + 0];
        newImageData[i*4 + 1] = imageData[i*3 + 1];
        newImageData[i*4 + 2] = imageData[i*3 + 2];
        newImageData[i*4 + 3] = (char)(255);
    }

    LINFO("opened transfer function: " << filename);
    
    // Generate the keys from the given data
    generateKeys(reinterpret_cast<char*>(newImageData) , imageWidth);

    // Little cleanup
    delete[] newImageData;
    ilDeleteImages(1, &imageID);
    
    // If we got up to here, everything was fine
    return true;
}

#else
bool TransFuncIntensity::loadWithDevIL(const std::string& filename) {
    LERROR("Loading failed: No DevIL support.");
    return false;
}
#endif // VRN_WITH_DEVIL

bool TransFuncIntensity::loadFromXML(const std::string& filename) {
    // Create a new XML document-object
    TiXmlDocument doc(filename);
    
    if (!doc.LoadFile())
        return false;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlElement* pElemC;
    TiXmlElement* hRoot;

    hRoot=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it does
    if (!hRoot)
        return false;

    keys_.clear();

    pElem=hRoot->FirstChildElement("key");
    for ( ; pElem ; pElem=pElem->NextSiblingElement("key")) {
        TransFuncMappingKey* k = new TransFuncMappingKey(0.0, tgt::col4(255,255,255,255));
        pElemC = pElem->FirstChildElement("col4");
        if (pElemC) {
            loadXml(pElemC, k->getColorL());
            loadXml(pElemC, k->getColorR());
        }

        int split;
        if (pElem->QueryIntAttribute("split", &split) == TIXML_SUCCESS) {
            if (split) {
                k->setSplit(true);
                pElemC=pElemC->NextSiblingElement("col4");
                if (pElemC)
                    loadXml(pElemC, k->getColorR());
            }
        }

        double intensity;
        if (pElem->QueryDoubleAttribute("intensity", &intensity) == TIXML_SUCCESS) {
            k->setIntensity(static_cast<float>(intensity));
        }
        addKey(k);
    }
    return true;
}

bool TransFuncIntensity::loadTextTable(const std::string& filename) {
    if (filename == "") {
        createStdFunc();
        return false;
    }
   
    FILE *file = fopen(filename.c_str(), "rt");

    if (file) {
        char* data = new char[256*4];
        for (int i = 0 ; i < 256 ; ++i) {
            int _red, _green, _blue, _alpha;
            fscanf(file, "%d %d %d %d", &_red, &_green, &_blue, &_alpha);

            data[4*i + 0] = _red;
            data[4*i + 1] = _green;
            data[4*i + 2] = _blue;
            data[4*i + 3] = _alpha;
        }
        fclose(file);
        LINFO("opened transfer function: " << filename);
        generateKeys(data);
        delete[] data;
        
        return true;
    } else {
        fclose(file);
        LWARNING("unable to open transfer function: " << filename);
        createStdFunc();
        return false;
    }
}

bool TransFuncIntensity::loadOsirixCLUT(const std::string& filename) {
    LINFO("Opening Osirix CLUT: " << filename);

    TiXmlDocument doc(filename.c_str());

    if (doc.LoadFile()) {
        // read and check version of plist file
        TiXmlNode* currNode = doc.FirstChild("plist");
        TiXmlElement* currElement = currNode->ToElement();

        currNode = currNode->FirstChild("dict");
        currNode = currNode->FirstChild("key");
        currElement = currNode->ToElement();

        // get reference to red, green and blue channel
        TiXmlElement* blueElement = 0;
        TiXmlElement* greenElement = 0;
        TiXmlElement* redElement = 0;
        TiXmlNode* blueNode = currElement->NextSibling();
        TiXmlNode* greenNode = ((blueNode->NextSibling())->NextSibling());
        TiXmlNode* redNode = ((greenNode->NextSibling())->NextSibling());
        blueNode = blueNode->FirstChild("integer");
        greenNode = greenNode->FirstChild("integer");
        redNode = redNode->FirstChild("integer");

        char* data = new char[256*4];

        for (int i = 0 ; i < 255 ; ++i) {
            blueNode = blueNode->NextSibling("integer");
            greenNode = greenNode->NextSibling("integer");
            redNode = redNode->NextSibling("integer");
            blueElement = blueNode->ToElement();
            greenElement = greenNode->ToElement();
            redElement = redNode->ToElement();

            data[4*i + 0] = atoi(redElement->GetText());
            data[4*i + 1] = atoi(greenElement->GetText());
            data[4*i + 2] = atoi(blueElement->GetText());
            data[4*i + 3] = (char)(255);
        }

        generateKeys(data);
        delete[] data;

        return true;
    } else
        return false;
}

bool TransFuncIntensity::loadImageJ(const std::string& filename) {
    std::ifstream fileStream(filename.c_str(), std::ios::in|std::ios::binary);

    if (fileStream.good()) {
        // Determine the length of file; necessary for typeswitch later
        size_t length;
        fileStream.seekg(0, std::ios::end);
        length = fileStream.tellg();
        fileStream.seekg(std::ios::beg);

        // If the length is that high, we can't use the data anyway
        if (length > 10000) {
            LWARNING("unable to open transfer function: " << filename);
            return false;
        }
        
        // The following procedures return 0 if the loading attempt was without success
        int size = 0;
        if (length > 768)
            // load NIH Image LUT
            size = openImageJBinary(fileStream, false);
        if ((size == 0) && ((length == 0) || (length == 768) || (length == 970)))
            //load raw LUT
            size = openImageJBinary(fileStream, true);
        if ((size == 0) && (length >= 768))
            size = openImageJText(fileStream);
        if (size == 0)
            LWARNING("unable to open transfer function: " << filename);

        // if size = 256, at least one of the procedures above was successful
        if (size == 256)
            LINFO("opened transfer function: " << filename);

        // same here; if one of the procedures was successful, true is returned
        return (size == 256);

    } else {
        // The file couldn't be opened and because of that, a default transfer function
        // is generated
        LWARNING("unable to open transfer function: " << filename);
        createStdFunc();
        return false;
    }

}

int TransFuncIntensity::openImageJBinary(std::ifstream& fileStream, bool raw) {
    // the default quantity of colors
    int numColors = 256;
    if (!raw) {
        // read the header information seperatly
        int id = readInt(fileStream);
        if (id != 1229147980) {
            // a leading 1229147980 (= 'ICOL') indicates an NIH Image LUT
            // if ICOL isn't the first entry in the table, let the other procedures handle it
            // but first, go back to the beginning
            fileStream.seekg(std::ios::beg);
            return 0;
        }
        readShort(fileStream); // Version
        numColors = readShort(fileStream); // Number of Colors
        readShort(fileStream);  // Start
        readShort(fileStream);  // End
        readDouble(fileStream); // Filler1
        readDouble(fileStream); // Filler2
        readInt(fileStream);    // Filler3
    }

    // The colors in a binary table are saved in succession so
    // first load the red's, then green's and at last blue's
    char* redColors = new char[256];
    fileStream.read(redColors, numColors);
    char* greenColors = new char[256];
    fileStream.read(greenColors, numColors);
    char* blueColors = new char[256];
    fileStream.read(blueColors, numColors);

    char* data = new char[256*4];

    for (int i = 0 ; i < 256 ; ++i) {
        data[i*4 + 0] = redColors[i];
        data[i*4 + 1] = greenColors[i];
        data[i*4 + 2] = blueColors[i];
        data[i*4 + 3] = (char)(255);
    }
    generateKeys(data);
    delete[] data;

    return 256;
}

int TransFuncIntensity::openImageJText(std::ifstream& fileStream){
    // in this array, the converted values are stored
    // The maximum are 256*4 entries
    char* data = new char[256*4];
    int tmp;
    int numValues = 0;
    char* entry = new char[50];
    int strToIntConversionStatus = 0;

    while (strToIntConversionStatus != -1) {
        fileStream >> entry;

        // After this line StrToIntConversionStatus is:
        // 1 if 'line' was an integer ; 0 otherwise ; -1 if the end of the sfile is reached
        strToIntConversionStatus = sscanf(entry, "%i", &tmp);
        
        // If an integer is encountered, add it to the array and increase the number of values
        if (strToIntConversionStatus == 1){
            data[numValues] = tmp;
            ++numValues;
        }
    }
    delete[] entry;

    /*  Now, two values of 'numValues' are possible. 256*3 = 768 or 256*4 = 1024.
        768, if there were 3 entries in a row
        1024, if there were 4 entires
    */
    if ((numValues != 768) && (numValues != 1024)){
        // A wrong file was loaded
        return 0;
    }

    if (numValues == 1024) {
        // The first entry is an index, therefore it can be overwritten
        // Additional to that, an alpha value of 255 is inserted at the 4th position
        for (int i = 0 ; i < 256; ++i){
            data[i*4 + 0] = data[i*4 + 1];
            data[i*4 + 1] = data[i*4 + 2];
            data[i*4 + 2] = data[i*4 + 3];
            data[i*4 + 3] = (char)(255);
        }
        generateKeys(data);
    }
    else{
        /* This one is a little bit tricky. We had 768 values in the file and added them
        sequentially to the data-array. Now we have to insert a '255' at every fourth position.
        In order to do that, we create a new array and move through both arrays at different
        paces.
        */
        char* newData = new char[1024];

        for (int i = 0 ; i < 256 ; ++i) {
            newData[i*4 + 0] = data[i*3 + 0];
            newData[i*4 + 1] = data[i*3 + 1];
            newData[i*4 + 2] = data[i*3 + 2];
            newData[i*4 + 3] = (char)(255);
        }

        generateKeys(newData);
        delete[] newData;
    }

    delete[] data;
    return 256;
}

void TransFuncIntensity::generateKeys(char* data, int width){
    /* A short overview about the idea behind this method. For the sake of simplicity this is
    demonstrated with only one color channel, say 'Red'. It is generalized in the code below:

    We want to detect the peaks(=extrema) in the graph containing all the 'red'-values.
    In order to do this, we look at one 1/width-th at a time and compare the difference between
    the (i-2)th and (i-1)th point (= oldDelta_x) to the difference between the (i-1)th and ith point
    (stored in newDelta_x). Several possible things can happen:
    i) The difference doesn't change at all. This means that the difference between the points is 
       linearly dependent and in this case, we don't have to add another key because we get linear
       interpolation from the the methods
    ii) The difference might be not-zero. This means the graph is discontinuous at this point and
        we have to insert a splitted point. On the left side we take the color from the
        (i-1)th point and on the right side from the ith point. The discontinuity is represented
        by the "jump" between the two parts of the splitted mapping key.
    iii) The difference could have changed by a factor of -1. In this case, we have to insert a
         mapping key right at the peak (i.e. at the ith point).

    If a key is placed, we don't want to place a key at the next location, because the difference
    will change not matter if there is a peak or not. There are a lot of cases, in this
    redundant keys will be generated and we don't want that  


    In the code below, we generate a key whenever any of the colorchannels meets a criterion above.
    */

    // Storage for the old values
    int oldDeltaRed;
    int oldDeltaGreen;
    int oldDeltaBlue;
    int oldDeltaAlpha;

    // Storage for the new values
    int newDeltaRed;
    int newDeltaGreen;
    int newDeltaBlue;
    int newDeltaAlpha;

    // We want at least 2 values in the data array
    if (width < 2)
        return;

    keys_.clear();

    addKey(new TransFuncMappingKey(0.f,
        tgt::col4(data[0], data[1], data[2], data[3])));

    // Calculate the starting point
    newDeltaRed   = data[4*1 + 0] - data[4*0 + 0];
    newDeltaGreen = data[4*1 + 1] - data[4*0 + 1];
    newDeltaBlue  = data[4*1 + 2] - data[4*0 + 2];
    newDeltaAlpha = data[4*1 + 3] - data[4*0 + 3];

    // The main loop. We start at 2 because the value for 1 already has been calculated.
    for (int iter = 2 ; iter < width ; ++iter){
        // Backup the old values and generate the new ones.
        oldDeltaRed = newDeltaRed;
        oldDeltaGreen = newDeltaGreen;
        oldDeltaBlue = newDeltaBlue;
        oldDeltaAlpha = newDeltaAlpha;

        newDeltaRed   = data[4*iter + 0] - data[4*(iter-1) + 0];
        newDeltaGreen = data[4*iter + 1] - data[4*(iter-1) + 1];
        newDeltaBlue  = data[4*iter + 2] - data[4*(iter-1) + 2];
        newDeltaAlpha = data[4*iter + 3] - data[4*(iter-1) + 3];

        // Has the difference quotient changed in any color channel?
        bool differenceQuotientChanged = (
            (oldDeltaRed   != newDeltaRed)   ||
            (oldDeltaGreen != newDeltaGreen) ||
            (oldDeltaBlue  != newDeltaBlue)  ||
            (oldDeltaAlpha != newDeltaAlpha));

        // Is the difference quotient different from zero in any channel?
        bool differenceQuotientNotZero = (
            (newDeltaRed   != 0) ||
            (newDeltaGreen != 0) ||
            (newDeltaBlue  != 0) ||
            (newDeltaAlpha != 0));

        // Has the difference quotient tilted in all channel's?
        // Mind the & instead of |
        bool differenceQuotientTilted = (
            (oldDeltaRed   == -newDeltaRed)   &&
            (oldDeltaGreen == -newDeltaGreen) &&
            (oldDeltaBlue  == -newDeltaBlue)  &&
            (oldDeltaAlpha == -newDeltaAlpha));

        if (differenceQuotientChanged) {
            if (differenceQuotientNotZero) {
                // We want to put a splitted key here (see ii above)
                TransFuncMappingKey* newkey = new TransFuncMappingKey( iter/static_cast<float>(width-1) ,
                    tgt::col4( data[4*(iter-1) + 0], data[4*(iter-1) + 1], data[4*(iter-1) + 2], data[4*(iter-1) + 3] )
                    );
                newkey->setSplit(true);
                newkey->setColorR(tgt::col4( data[4*iter + 0], data[4*iter + 1], data[4*iter + 2], data[4*iter + 3] ));
                addKey(newkey);
            }
            else if (differenceQuotientTilted) {
                // We want a single key at i-1 here (see iii above)
                addKey(
                    new TransFuncMappingKey((iter - 1)/static_cast<float>(width-1),
                    tgt::col4( data[4*(iter-1) + 0], data[4*(iter-1) + 1], data[4*(iter-1) + 2], data[4*(iter-1) + 3] )
                    ));
            }
            else {
                // Just add a key
                addKey(
                    new TransFuncMappingKey( iter/static_cast<float>(width-1),
                    tgt::col4( data[4*iter + 0], data[4*iter + 1], data[4*iter + 2], data[4*iter + 3] )));
            }
        }
    }
}

inline int TransFuncIntensity::readInt(std::ifstream& stream) {
    char* buffer = new char[4];

	// The bytes are inserted backwards because of a discrepancy of 
	// most significant bit <-> least significant bit between Java and C++
    for (int i = 3 ; i >= 0 ; --i)
        stream >> buffer[i];

    return *(reinterpret_cast<int*>(buffer));
    delete buffer;
}

inline short TransFuncIntensity::readShort(std::ifstream& stream) {
    char* buffer = new char[2];

    for (int i = 1 ; i >= 0 ; --i)
        stream >> buffer[i];
    
    return *(reinterpret_cast<short*>(buffer));
    delete buffer;
}

inline double TransFuncIntensity::readDouble(std::ifstream& stream) {
    char* buffer = new char[8];

    for (int i = 7 ; i >= 0 ; --i)
        stream >> buffer[i];

    return *(reinterpret_cast<double*>(buffer));
    delete buffer;
}

} //namespace voreen
