/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/io/analyzevolumereader.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/vector.h"

#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::hor;
using tgt::lessThanEqual;

namespace voreen {

const std::string AnalyzeVolumeReader::loggerCat_ = "voreen.io.VolumeReader.analyze";

void swap_long(unsigned char *pntr) {
    unsigned char b0, b1, b2, b3;
    b0 = *pntr;
    b1 = *(pntr+1);
    b2 = *(pntr+2);
    b3 = *(pntr+3);
    *pntr = b3;
    *(pntr+1) = b2;
    *(pntr+2) = b1;
    *(pntr+3) = b0;
}

void swap_short(unsigned char *pntr) {
    unsigned char b0, b1;
    b0 = *pntr;
    b1 = *(pntr+1);
    *pntr = b1;
    *(pntr+1) = b0;
}

struct header_key {
    int32_t sizeof_hdr;
    char data_type[10];
    char db_name[18];
    int32_t extents;
    int16_t session_error;
    char regular;
    char hkey_un0;

    void swapEndianess() {
        swap_long(reinterpret_cast<unsigned char*>(&sizeof_hdr));
        swap_long(reinterpret_cast<unsigned char*>(&extents));
        swap_short(reinterpret_cast<unsigned char*>(&session_error));
    }
};
/* 40 bytes */

struct image_dimension
{
    int16_t dim[8];
    int16_t unused8;
    int16_t unused9;
    int16_t unused10;
    int16_t unused11;
    int16_t unused12;
    int16_t unused13;
    int16_t unused14;
    int16_t datatype;
    int16_t bitpix;
    int16_t dim_un0;
    float pixdim[8];
    float vox_offset;
    float funused1;
    float funused2;
    float funused3;
    float cal_max;
    float cal_min;
    float compressed;
    float verified;
    int32_t glmax,glmin;

    void swapEndianess() {
        swap_short(reinterpret_cast<unsigned char*>(&dim[0]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[1]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[2]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[3]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[4]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[5]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[6]));
        swap_short(reinterpret_cast<unsigned char*>(&dim[7]));
        swap_short(reinterpret_cast<unsigned char*>(&unused8));
        swap_short(reinterpret_cast<unsigned char*>(&unused9));
        swap_short(reinterpret_cast<unsigned char*>(&unused10));
        swap_short(reinterpret_cast<unsigned char*>(&unused11));
        swap_short(reinterpret_cast<unsigned char*>(&unused12));
        swap_short(reinterpret_cast<unsigned char*>(&unused13));
        swap_short(reinterpret_cast<unsigned char*>(&unused14));
        swap_short(reinterpret_cast<unsigned char*>(&datatype));
        swap_short(reinterpret_cast<unsigned char*>(&bitpix));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[0]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[1]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[2]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[3]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[4]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[5]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[5]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[6]));
        swap_long(reinterpret_cast<unsigned char*>(&pixdim[7]));
        swap_long(reinterpret_cast<unsigned char*>(&vox_offset));
        swap_long(reinterpret_cast<unsigned char*>(&funused1));
        swap_long(reinterpret_cast<unsigned char*>(&funused2));
        swap_long(reinterpret_cast<unsigned char*>(&cal_max));
        swap_long(reinterpret_cast<unsigned char*>(&cal_min));
        swap_long(reinterpret_cast<unsigned char*>(&compressed));
        swap_long(reinterpret_cast<unsigned char*>(&verified));
        swap_short(reinterpret_cast<unsigned char*>(&dim_un0));
        swap_long(reinterpret_cast<unsigned char*>(&glmax));
        swap_long(reinterpret_cast<unsigned char*>(&glmin));
    }
};
/* 108 bytes */

struct data_history
{
    char descrip[80];
    char aux_file[24];
    char orient;
    char originator[10];
    char generated[10];
    char scannum[10];
    char patient_id[10];
    char exp_date[10];
    char exp_time[10];
    char hist_un0[3];
    int32_t views;
    int32_t vols_added;
    int32_t start_field;
    int32_t field_skip;
    int32_t omax, omin;
    int32_t smax, smin;

    void swapEndianess() {
        swap_long(reinterpret_cast<unsigned char*>(&views));
        swap_long(reinterpret_cast<unsigned char*>(&vols_added));
        swap_long(reinterpret_cast<unsigned char*>(&start_field));
        swap_long(reinterpret_cast<unsigned char*>(&field_skip));
        swap_long(reinterpret_cast<unsigned char*>(&omax));
        swap_long(reinterpret_cast<unsigned char*>(&omin));
        swap_long(reinterpret_cast<unsigned char*>(&smax));
        swap_long(reinterpret_cast<unsigned char*>(&smin));
    }
};
/* 200 bytes */

struct analyze_header
{
    header_key hk;
    image_dimension dime;
    data_history hist;
};
/* 348 bytes */

/* Acceptable values for datatype */
#define DT_NONE 0
#define DT_UNKNOWN 0
#define DT_BINARY 1
#define DT_UNSIGNED_CHAR 2
#define DT_SIGNED_SHORT 4
#define DT_SIGNED_INT 8
#define DT_FLOAT 16
#define DT_COMPLEX 32
#define DT_DOUBLE 64
#define DT_RGB 128
#define DT_ALL 255

                            /*------------------- new codes for NIFTI ---*/
#define DT_INT8                  256     /* signed char (8 bits)         */
#define DT_UINT16                512     /* unsigned short (16 bits)     */
#define DT_UINT32                768     /* unsigned int (32 bits)       */
#define DT_INT64                1024     /* long long (64 bits)          */
#define DT_UINT64               1280     /* unsigned long long (64 bits) */
#define DT_FLOAT128             1536     /* long double (128 bits)       */
#define DT_COMPLEX128           1792     /* double pair (128 bits)       */
#define DT_COMPLEX256           2048     /* long double pair (256 bits)  */
#define DT_RGBA32               2304     /* 4 byte RGBA (32 bits/voxel)  */

//struct complex
//{
    //float real;
    //float imag;
//};

struct nifti_1_header { /* NIFTI-1 usage         */  /* ANALYZE 7.5 field(s) */
                        /*************************/  /************************/
                                           /*--- was header_key substruct ---*/
 int32_t   sizeof_hdr;    /*!< MUST be 348           */  /* int sizeof_hdr;      */
 char  data_type[10]; /*!< ++UNUSED++            */  /* char data_type[10];  */
 char  db_name[18];   /*!< ++UNUSED++            */  /* char db_name[18];    */
 int32_t   extents;       /*!< ++UNUSED++            */  /* int extents;         */
 int16_t session_error; /*!< ++UNUSED++            */  /* short session_error; */
 char  regular;       /*!< ++UNUSED++            */  /* char regular;        */
 char  dim_info;      /*!< MRI slice ordering.   */  /* char hkey_un0;       */

                                      /*--- was image_dimension substruct ---*/
 int16_t dim[8];        /*!< Data array dimensions.*/  /* short dim[8];        */
 float intent_p1 ;    /*!< 1st intent parameter. */  /* int16_t unused8;       */
                                                     /* int16_t unused9;       */
 float intent_p2 ;    /*!< 2nd intent parameter. */  /* int16_t unused10;      */
                                                     /* int16_t unused11;      */
 float intent_p3 ;    /*!< 3rd intent parameter. */  /* int16_t unused12;      */
                                                     /* int16_t unused13;      */
 int16_t intent_code ;  /*!< NIFTI_INTENT_* code.  */  /* short unused14;      */
 int16_t datatype;      /*!< Defines data type!    */  /* short datatype;      */
 int16_t bitpix;        /*!< Number bits/voxel.    */  /* short bitpix;        */
 int16_t slice_start;   /*!< First slice index.    */  /* short dim_un0;       */
 float pixdim[8];     /*!< Grid spacings.        */  /* float pixdim[8];     */
 float vox_offset;    /*!< Offset into .nii file */  /* float vox_offset;    */
 float scl_slope ;    /*!< Data scaling: slope.  */  /* float funused1;      */
 float scl_inter ;    /*!< Data scaling: offset. */  /* float funused2;      */
 int16_t slice_end;     /*!< Last slice index.     */  /* float funused3;      */
 char  slice_code ;   /*!< Slice timing order.   */
 char  xyzt_units ;   /*!< Units of pixdim[1..4] */
 float cal_max;       /*!< Max display intensity */  /* float cal_max;       */
 float cal_min;       /*!< Min display intensity */  /* float cal_min;       */
 float slice_duration;/*!< Time for 1 slice.     */  /* float compressed;    */
 float toffset;       /*!< Time axis shift.      */  /* float verified;      */
 int32_t   glmax;         /*!< ++UNUSED++            */  /* int glmax;           */
 int32_t   glmin;         /*!< ++UNUSED++            */  /* int glmin;           */

                                         /*--- was data_history substruct ---*/
 char  descrip[80];   /*!< any text you like.    */  /* char descrip[80];    */
 char  aux_file[24];  /*!< auxiliary filename.   */  /* char aux_file[24];   */

 int16_t qform_code ;   /*!< NIFTI_XFORM_* code.   */  /*-- all ANALYZE 7.5 ---*/
 int16_t sform_code ;   /*!< NIFTI_XFORM_* code.   */  /*   fields below here  */
                                                     /*   are replaced       */
 float quatern_b ;    /*!< Quaternion b param.   */
 float quatern_c ;    /*!< Quaternion c param.   */
 float quatern_d ;    /*!< Quaternion d param.   */
 float qoffset_x ;    /*!< Quaternion x shift.   */
 float qoffset_y ;    /*!< Quaternion y shift.   */
 float qoffset_z ;    /*!< Quaternion z shift.   */

 float srow_x[4] ;    /*!< 1st row affine transform.   */
 float srow_y[4] ;    /*!< 2nd row affine transform.   */
 float srow_z[4] ;    /*!< 3rd row affine transform.   */

 char intent_name[16];/*!< 'name' or meaning of data.  */

 char magic[4] ;      /*!< MUST be "ni1\0" or "n+1\0". */

 void swapEndianess() {
     swap_long(reinterpret_cast<unsigned char*>(&sizeof_hdr));
     swap_long(reinterpret_cast<unsigned char*>(&extents));
     swap_short(reinterpret_cast<unsigned char*>(&session_error));
     swap_short(reinterpret_cast<unsigned char*>(&dim[0]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[1]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[2]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[3]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[4]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[5]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[6]));
     swap_short(reinterpret_cast<unsigned char*>(&dim[7]));
     swap_long(reinterpret_cast<unsigned char*>(&intent_p1));
     swap_long(reinterpret_cast<unsigned char*>(&intent_p2));
     swap_long(reinterpret_cast<unsigned char*>(&intent_p3));
     swap_short(reinterpret_cast<unsigned char*>(&intent_code));
     swap_short(reinterpret_cast<unsigned char*>(&datatype));
     swap_short(reinterpret_cast<unsigned char*>(&bitpix));
     swap_short(reinterpret_cast<unsigned char*>(&slice_start));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[0]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[1]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[2]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[3]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[4]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[5]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[6]));
     swap_long(reinterpret_cast<unsigned char*>(&pixdim[7]));
     swap_long(reinterpret_cast<unsigned char*>(&vox_offset));
     swap_long(reinterpret_cast<unsigned char*>(&scl_slope));
     swap_long(reinterpret_cast<unsigned char*>(&scl_inter));
     swap_short(reinterpret_cast<unsigned char*>(&slice_end));
     swap_long(reinterpret_cast<unsigned char*>(&cal_max));
     swap_long(reinterpret_cast<unsigned char*>(&cal_min));
     swap_long(reinterpret_cast<unsigned char*>(&slice_duration));
     swap_long(reinterpret_cast<unsigned char*>(&toffset));
     swap_long(reinterpret_cast<unsigned char*>(&glmax));
     swap_long(reinterpret_cast<unsigned char*>(&glmin));
     swap_short(reinterpret_cast<unsigned char*>(&qform_code));
     swap_short(reinterpret_cast<unsigned char*>(&sform_code));
     swap_long(reinterpret_cast<unsigned char*>(&quatern_b));
     swap_long(reinterpret_cast<unsigned char*>(&quatern_c));
     swap_long(reinterpret_cast<unsigned char*>(&quatern_d));
     swap_long(reinterpret_cast<unsigned char*>(&qoffset_x));
     swap_long(reinterpret_cast<unsigned char*>(&qoffset_y));
     swap_long(reinterpret_cast<unsigned char*>(&qoffset_z));

     swap_long(reinterpret_cast<unsigned char*>(&srow_x[0]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_x[1]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_x[2]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_x[3]));

     swap_long(reinterpret_cast<unsigned char*>(&srow_y[0]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_y[1]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_y[2]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_y[3]));

     swap_long(reinterpret_cast<unsigned char*>(&srow_z[0]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_z[1]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_z[2]));
     swap_long(reinterpret_cast<unsigned char*>(&srow_z[3]));
 }
};                   /**** 348 bytes total ****/

AnalyzeVolumeReader::AnalyzeVolumeReader(ProgressBar* progress)
    : VolumeReader(progress)
{
    extensions_.push_back("hdr");
    extensions_.push_back("nii");

    //make sure the header sizes are ok when compiled on all architectures:
    tgtAssert(sizeof(header_key) == 40, "Wrong header_key size!");
    tgtAssert(sizeof(image_dimension) == 108, "Wrong image_dimension size!");
    tgtAssert(sizeof(data_history) == 200, "Wrong data_history size!");
    tgtAssert(sizeof(analyze_header) == 348, "Wrong analyze_header size!");
    tgtAssert(sizeof(nifti_1_header ) == 348, "Wrong nifti_1_header size!");
}

std::string AnalyzeVolumeReader::getRelatedImgFileName(const std::string& fileName) {
    std::string imgFileName = fileName.substr(0, fileName.length()-3);
    imgFileName += "img";
    return imgFileName;
}

VolumeCollection* AnalyzeVolumeReader::read(const std::string &url)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    std::string fileName = origin.getPath();

    //check if we got analyze or nifti:

    //extension .nii => standalone nifti
    if(fileName.find(".nii") != std::string::npos)
        return readNifti(fileName, true);
    else {
        //check magic string:
        std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!file) {
            LERROR("Failed to open file: " << fileName);
            return 0;
        }

        file.seekg(0, std::ios::end);
        int fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if(fileSize < 348)
            throw tgt::CorruptedFileException("Analyze/Nifti file is smaller than 348 bytes!", fileName);

        nifti_1_header header;
        if (!file.read((char*)&header, sizeof(header))) {
            throw tgt::CorruptedFileException("Failed to read header!", fileName);
        }
        file.close();

        if( (header.magic[0] == 'n') && (header.magic[2] == '1') && (header.magic[3] == 0) ) {
            if(header.magic[1] == '+') {
                return readNifti(fileName, true);
            }
            else if(header.magic[1] == 'i') {
                return readNifti(fileName, false);
            }
            else
                return readAnalyze(fileName);
        }
        else
            return readAnalyze(fileName);
    }
    return 0;
}

VolumeCollection* AnalyzeVolumeReader::readNifti(const std::string &fileName, bool standalone)
    throw (tgt::FileException, std::bad_alloc)
{
    LINFO("Loading nifti file " << fileName);

    std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!file) {
        LERROR("Failed to open file: " << fileName);
        return 0;
    }

    //file.seekg(0, std::ios::end);
    //int fileSize = file.tellg();
    //file.seekg(0, std::ios::beg);

    nifti_1_header header;
    if (!file.read((char*)&header, sizeof(header))) {
        throw tgt::CorruptedFileException("Failed to read header!", fileName);
    }

    file.close();

    bool bigEndian = false;
    //check if swap is necessary:
    if((header.dim[0] < 0) || (header.dim[0] > 15)) {
        bigEndian = true;
        header.swapEndianess();
    }

    if(header.sizeof_hdr != 348) {
        throw tgt::CorruptedFileException("Invalid header.sizeof_hdr", fileName);
    }

    if(!( (header.magic[0] == 'n') && (header.magic[2] == '1') && (header.magic[3] == 0) ))
        throw tgt::CorruptedFileException("Not a Nifti header!", fileName);

    if(header.magic[1] == '+') {
        if(!standalone)
            LWARNING("Tried to read standalone Nifti as hdr+img!");
        standalone = true;
    }
    else if(header.magic[1] == 'i') {
        if(!standalone)
            LWARNING("Tried to hdr+img Nifti as standalone!");
        standalone = false;
    }
    else
        throw tgt::CorruptedFileException("Not a Nifti header!", fileName);

    RawVolumeReader::ReadHints h;

    h.dimensions_.x = header.dim[1];
    h.dimensions_.y = header.dim[2];
    h.dimensions_.z = header.dim[3];
    LINFO("Resolution: " << h.dimensions_);

    if (hor(lessThanEqual(h.dimensions_, ivec3(0)))) {
        LERROR("Invalid resolution or resolution not specified: " << h.dimensions_);
        throw tgt::CorruptedFileException("error while reading data", fileName);
    }

    h.spacing_.x = header.pixdim[1];
    h.spacing_.y = header.pixdim[2];
    h.spacing_.z = header.pixdim[3];
    LINFO("Spacing: " << h.spacing_);

    LINFO("Datatype: " << header.datatype);

    //TODO: support more datatypes
    if(header.datatype > 128) {
        header.datatype -= 128;
        h.objectModel_ == "RGB";
    }
    else
        h.objectModel_ == "I";


    switch(header.datatype) {
        case DT_UNSIGNED_CHAR:
            h.format_ = "UCHAR";
            h.objectModel_ == "I";
            break;
        case DT_SIGNED_SHORT:
            h.format_ = "SHORT";
            h.objectModel_ == "I";
            break;
        case DT_SIGNED_INT:
            h.format_ = "INT";
            h.objectModel_ == "I";
            break;
        case DT_FLOAT:
            h.format_ = "FLOAT";
            h.objectModel_ == "I";
            break;
        case DT_DOUBLE:
            h.format_ = "DOUBLE";
            h.objectModel_ == "I";
            break;
        case DT_RGB:
            h.format_ = "UCHAR";
            h.objectModel_ == "RGB";
            break;
        case DT_RGBA32:         /* 4 byte RGBA (32 bits/voxel)  */
            h.format_ = "UCHAR";
            h.objectModel_ == "RGBA";
            break;
        case DT_INT8:           /* signed char (8 bits)         */
            h.format_ = "CHAR";
            h.objectModel_ == "I";
            break;
        case DT_UINT16:         /* unsigned short (16 bits)     */
            h.format_ = "USHORT";
            h.objectModel_ == "I";
            break;
        case DT_UINT32:         /* unsigned int (32 bits)       */
            h.format_ = "UINT";
            h.objectModel_ == "I";
            break;
        case DT_INT64:          /* long long (64 bits)          */
        case DT_UINT64:         /* unsigned long long (64 bits) */
        case DT_FLOAT128:       /* long double (128 bits)       */
        case DT_COMPLEX128:     /* double pair (128 bits)       */
        case DT_COMPLEX256:     /* long double pair (256 bits)  */
        case DT_ALL:
        case DT_COMPLEX:
        case 0: //DT_NONE/DT_UNKNOWN
        case DT_BINARY:
        default:
            throw tgt::UnsupportedFormatException("Unsupported datatype!");
    }

    h.bigEndianByteOrder_ = bigEndian;

    //std::string objectType;
    //std::string gridType;
        //} else if (type == "ObjectType:") {
            //args >> objectType;
            //LDEBUG(type << " " << objectType);
        //} else if (type == "GridType:") {
            //args >> gridType;
            //LDEBUG(type << " " << gridType);
        //} else if (type == "BitsStored:") {
            //args >> h.bitsStored_;
            //LDEBUG(type << " " << h.bitsStored_);
        //} else if (type == "Unit:") {
            //args >> h.unit_;
            //LDEBUG(type << " " << h.unit_);

    if (standalone)
        h.headerskip_ = static_cast<uint16_t>(header.vox_offset);

    RawVolumeReader rawReader(getProgressBar());
    rawReader.setReadHints(h);

    VolumeCollection* volumeCollection = 0;
    if(standalone)
        volumeCollection = rawReader.read(fileName);
    else
        volumeCollection = rawReader.read(getRelatedImgFileName(fileName));

    if (!volumeCollection->empty())
        volumeCollection->first()->setOrigin(VolumeOrigin(fileName));

    return volumeCollection;
}

VolumeCollection* AnalyzeVolumeReader::readAnalyze(const std::string &fileName)
    throw (tgt::FileException, std::bad_alloc)
{
    LWARNING("Loading analyze file " << fileName);
    LWARNING("Related img file: " << getRelatedImgFileName(fileName));

    std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!file) {
        LERROR("Failed to open file: " << fileName);
        return 0;
    }

    file.seekg(0, std::ios::end);
    int fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if(fileSize != 348)
        LWARNING("Filesize != 348");

    header_key header;
    if (!file.read((char*)&header, sizeof(header))) {
        throw tgt::CorruptedFileException("Failed to read header!", fileName);
    }

    image_dimension dimension;
    if (!file.read((char*)&dimension, sizeof(dimension))) {
        throw tgt::CorruptedFileException("Failed to read dimensions!", fileName);
    }

    data_history history;
    if (!file.read((char*)&history, sizeof(history))) {
        throw tgt::CorruptedFileException("Failed to read history!", fileName);
    }

    bool bigEndian = false;
    //check if swap is necessary:
    if((dimension.dim[0] < 0) || (dimension.dim[0] > 15)) {
        bigEndian = true;
        header.swapEndianess();
        dimension.swapEndianess();
        history.swapEndianess();
    }

    RawVolumeReader::ReadHints h;

    h.dimensions_.x = dimension.dim[1];
    h.dimensions_.y = dimension.dim[2];
    h.dimensions_.z = dimension.dim[3];
    LINFO("Resolution: " << h.dimensions_);

    if (hor(lessThanEqual(h.dimensions_, ivec3(0)))) {
        LERROR("Invalid resolution or resolution not specified: " << h.dimensions_);
        throw tgt::CorruptedFileException("error while reading data", fileName);
    }

    h.spacing_.x = dimension.pixdim[1];
    h.spacing_.y = dimension.pixdim[2];
    h.spacing_.z = dimension.pixdim[3];
    LINFO("Spacing: " << h.spacing_);

    LINFO("Datatype: " << dimension.datatype);

    switch(dimension.datatype) {
        case DT_UNSIGNED_CHAR:
            h.format_ = "UCHAR";
            h.objectModel_ == "I";
            break;
        case DT_SIGNED_SHORT:
            h.format_ = "SHORT";
            h.objectModel_ == "I";
            break;
        case DT_SIGNED_INT:
            h.format_ = "INT";
            h.objectModel_ == "I";
            break;
        case DT_FLOAT:
            h.format_ = "FLOAT";
            h.objectModel_ == "I";
            break;
        case DT_DOUBLE:
            h.format_ = "DOUBLE";
            h.objectModel_ == "I";
            break;
        case DT_RGB:
            h.format_ = "UCHAR";
            h.objectModel_ == "RGB";
            break;
        case DT_ALL:
        case DT_COMPLEX:
        case 0: //DT_NONE/DT_UNKNOWN
        case DT_BINARY:
        default:
            throw tgt::UnsupportedFormatException("Unsupported datatype!");
    }

    h.bigEndianByteOrder_ = bigEndian;

    std::string objectType;
    std::string gridType;
        //} else if (type == "ObjectType:") {
            //args >> objectType;
            //LDEBUG(type << " " << objectType);
        //} else if (type == "GridType:") {
            //args >> gridType;
            //LDEBUG(type << " " << gridType);
        //} else if (type == "BitsStored:") {
            //args >> h.bitsStored_;
            //LDEBUG(type << " " << h.bitsStored_);
        //} else if (type == "Unit:") {
            //args >> h.unit_;
            //LDEBUG(type << " " << h.unit_);

    RawVolumeReader rawReader(getProgressBar());
    rawReader.setReadHints(h);

    VolumeCollection* volumeCollection = rawReader.read(getRelatedImgFileName(fileName));

    if (!volumeCollection->empty())
        volumeCollection->first()->setOrigin(VolumeOrigin(fileName));

    return volumeCollection;
}

VolumeReader* AnalyzeVolumeReader::create(ProgressBar* progress) const {
    return new AnalyzeVolumeReader(progress);
}

} // namespace voreen
