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

#ifdef VRN_WITH_DEVIL

#include "commands_convert.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumeatomic.h"
#include "tgt/vector.h"

#include <IL/il.h>
#include <IL/ilu.h>
 
namespace voreen {

CommandStackImg::CommandStackImg() {
    help_ = "All image formats supported by DevIL can be used.\n";
    
    info_ = "Stack images given by sourceFilename(s) to create one volumedataset.";
    name_ = "stackimg";
    syntax_ = name_ + " *IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandStackImg::execute(const std::vector<std::string>& parameters) {
    Volume* targetDataset_;
    tgt::ivec3 dimensions;
    dimensions.z = parameters.size()-1;

    ilInit();
    iluInit();
    ILuint ImageName;

    //load first image to check dimensions...
    ilGenImages(1, &ImageName);
    ilBindImage(ImageName);
    ilLoadImage((char*)parameters[0].c_str());
    ILuint Width, Height, Format, Type;
    Width = ilGetInteger(IL_IMAGE_WIDTH);
    Height = ilGetInteger(IL_IMAGE_HEIGHT);
    Format = ilGetInteger(IL_IMAGE_FORMAT);
    Type = ilGetInteger(IL_IMAGE_TYPE);

    switch(Format) {
        case IL_RGB : LDEBUG("Format: IL_RGB");
            break;
        case IL_RGBA : LDEBUG("Format: IL_RGBA");
            break;
        case IL_BGR : LDEBUG("Format: IL_BGR");
            break;
        case IL_BGRA : LDEBUG("Format: IL_BGRA");
            break;
        case IL_LUMINANCE: LDEBUG("Format: IL_LUMINANCE");
            break;
    }

    switch(Type) {
        case IL_UNSIGNED_BYTE : LDEBUG("Type: IL_UNSIGNED_BYTE");
            break;
        case IL_BYTE : LDEBUG("Type: IL_BYTE");
            break;
        case IL_UNSIGNED_SHORT : LDEBUG("Type: IL_UNSIGNED_SHORT");
            break;
        case IL_SHORT : LDEBUG("Type: IL_SHORT");
            break;
        case IL_INT : LDEBUG("Type: IL_INT");
            break;
        case IL_UNSIGNED_INT : LDEBUG("Type: IL_UNSIGNED_INT");
            break;
    }

    ILenum Error;
    while ((Error = ilGetError()) != IL_NO_ERROR) {
        LERROR(Error << iluErrorString(Error))
//         printf("%d: %s/n", Error, );
    }
    dimensions.x = Width;
    dimensions.y = Height;
    LINFO("stacking " << dimensions.z << " images with dimensions: " << dimensions);
    ilDeleteImages(1, &ImageName);

    std::string fname;  //current filename
    targetDataset_ = new VolumeUInt8(dimensions);
    uint8_t* p = (uint8_t*)targetDataset_->getData();

    for(int i=0; i < dimensions.z; i++) {
        fname = parameters[i];
        ilGenImages(1, &ImageName);
        ilBindImage(ImageName);
        ilLoadImage((char*)fname.c_str());
        ilCopyPixels(0, 0, 0, dimensions.x, dimensions.y, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, p);
        p += dimensions.x * dimensions.y;
        ilDeleteImages(1, &ImageName);
    }
    
    if(targetDataset_) {
        VolumeSerializerPopulator volLoadPop;
        VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
        serializer->save(parameters.back(), targetDataset_);
        delete serializer;
        delete targetDataset_;
    }
    return true;
}

//-----------------------------------------------------------------------------

CommandStackRaw::CommandStackRaw() {
    help_ =  "Input images with size DX*DY\n";
    help_ += "HEADERSIZE bytes at the beginning of each file are skipped.\n";
    help_ += "Formats:\n";
    help_ += "gs16: 16 bit grayscale\n";
    help_ += "rgbni: 24 bit rgb, non interleaved\n";

    info_ = "Stack raw-images given by sourceFilename(s)";
    name_ = "stackraw";
    syntax_ = name_ + " DX DY HEADERSIZE [gs16|rgbni] *IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandStackRaw::execute(const std::vector<std::string>& parameters) {
    Volume* targetDataset_;
    
    tgt::ivec3 dimensions;
    dimensions.x = asInt(parameters[0]);
    dimensions.y = asInt(parameters[1]);

    dimensions.z = parameters.size()-5;
    
    int headersize;
    headersize = asInt(parameters[2]);
    
    std::string format = parameters[3];
    LINFO("stacking " << dimensions.z << " images with dimensions: " << dimensions);
    LINFO("skipping header with size " << headersize);
    LINFO("format: " << format);

    if(format == "gs16")
    {
        targetDataset_ = new VolumeUInt16(dimensions);
        uint8_t* p = (uint8_t*)targetDataset_->getData();
        std::ifstream fin;
        int readsize = dimensions.x * dimensions.y * 2;
        std::string fname;  //current filename

        for(int i=0; i < dimensions.z; i++) {
            fname = parameters[i+4];
            fin.open(fname.c_str(), std::ios::in | std::ios::binary);
            if (!fin.good()) {
                LERROR("failed to open " << fname);
                break;
            }
            else
                LINFO("opened: " << fname);
            //skip header:
            fin.seekg(headersize, std::ios::beg );
            fin.read(reinterpret_cast<char*>(p), readsize);
            LDEBUG("read " << fin.gcount() << " of " << readsize <<  " bytes");
            p += readsize;
            if( fin.bad() ) {
                LERROR("Error reading data");
                exit( 0 );
            }
            fin.close();
        }
    }
    else if(format == "rgbni")
    {
        targetDataset_ = new Volume4xUInt8(dimensions);
        uint8_t* p = (uint8_t*)targetDataset_->getData();
        std::ifstream fin;
        int readsize = dimensions.x * dimensions.y;
        uint8_t* buffer = new uint8_t[readsize];
        std::string fname;  //current filename

        for(int i=0; i < dimensions.z; i++) {
            fname = parameters[i+4];
            fin.open(fname.c_str(), std::ios::in | std::ios::binary);
            if (!fin.good()) {
                LERROR("failed to open " << fname);
                break;
            }
            else
                LDEBUG("opened: " << fname);
            //skip header:
            fin.seekg(headersize, std::ios::beg);
            //loop over rgb:
            for(int c=0; c<3; c++) {
                fin.read(reinterpret_cast<char*>(buffer), readsize);
                LDEBUG("read " << fin.gcount() << " of " << readsize <<  " bytes");
                if( fin.bad() ) {
                    LERROR("Error reading data");
                    return false;
                }
                uint8_t* colorp = p;
                for(int j=0; j<readsize; j++) {
                    colorp[c] = buffer[j];
                    colorp += 4;
                }
            }
            //set alpha to opaque:
            uint8_t* colorp = p;
            for(int j=0; j<readsize; j++) {
                //calculate greyscale value for alpha channel:
                //colorp[3] = 0.3 * (float) colorp[0] + 0.59 * (float) colorp[1] + 0.11 * (float) colorp[2];
                colorp[3] = 255;
                colorp += 4;
            }
            fin.close();
            p += readsize*4;
        }
        delete[] buffer;
    }
    else
        throw SyntaxException("Unknown format!");
        
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    serializer->save(parameters.back(), targetDataset_);
    
    delete serializer;
    delete targetDataset_;
    return true;
}


//-----------------------------------------------------------------------------

CommandConvert::CommandConvert() {
    help_ =  "TARGET:\n";
    help_ += "8: 8 bit DS\n";
    help_ += "8s: 8 bit DS, with smart convert\n";
    help_ += "12: 12 bit DS\n";
    help_ += "12s: 12 bit DS, with smart convert\n";
    help_ += "16: 16 bit DS\n";
    help_ += "16s: 16 bit DS, with smart convert\n";

    info_ = "Convert Volume Datasets";
    name_ = "convert";
    syntax_ = name_ + " TARGET IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandConvert::execute(const std::vector<std::string>& parameters) {
    Volume* targetDataset_;
    
    checkParameters(parameters.size() == 3);
    
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[1], false);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();
    
    if(parameters[0] == "8") {
        targetDataset_ = new VolumeUInt8(sourceDataset_->getDimensions());
        targetDataset_->convert(sourceDataset_, false);
    }
    else if(parameters[0] == "12") {
        targetDataset_ = new VolumeUInt16(sourceDataset_->getDimensions(), sourceDataset_->getSpacing(), 12);
        targetDataset_->convert(sourceDataset_, false);
    }
    else if(parameters[0] == "16") {
        targetDataset_ = new VolumeUInt16(sourceDataset_->getDimensions());
        targetDataset_->convert(sourceDataset_, false);
    }
    //smart conversion:
    else if(parameters[0] == "8s") {
        targetDataset_ = new VolumeUInt8(sourceDataset_->getDimensions());
        targetDataset_->convert(sourceDataset_, true);
    }
    else if(parameters[0] == "12s") {
        targetDataset_ = new VolumeUInt16(sourceDataset_->getDimensions(), sourceDataset_->getSpacing(), 12);
        targetDataset_->convert(sourceDataset_, true);
    }
    else if(parameters[0] == "16s") {
        targetDataset_ = new VolumeUInt16(sourceDataset_->getDimensions());
        targetDataset_->convert(sourceDataset_, true);
    }
    else {
        delete sourceDataset_;
        throw SyntaxException("Unknown target!");
    }
    
    //VolumeSerializerPopulator volLoadPop;
    //VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    serializer->save(parameters.back(), targetDataset_);
    
    delete serializer;
    delete targetDataset_;
    return true;
}

}   //namespace voreen

#endif // VRN_WITH_DEVIL
