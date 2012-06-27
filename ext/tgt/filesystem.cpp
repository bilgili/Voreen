/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/filesystem.h"

#include <cstring>
#include <iostream>
#include <fstream>

#ifdef TGT_HAS_ZLIB
#include <zlib.h>
#endif

// anonymous namespace
namespace {

enum {
    TEMP_BUFFERSIZE = 128,
    LINE_BUFFERSIZE = 512,

    // Standard Archive Format - Standard TAR - USTAR
    RECORDSIZE = 512,
    NAMSIZ = 100,
    TUNMLEN = 32,
    TGNMLEN = 32
};


union record {
    char charptr[RECORDSIZE];
    struct header {
        char    name[NAMSIZ];
        char    mode[8];
        char    uid[8];
        char    gid[8];
        char    size[12];
        char    mtime[12];
        char    chksum[8];
        char    linkflag;
        char    linkname[NAMSIZ];
        char    magic[8];
        char    uname[TUNMLEN];
        char    gname[TGNMLEN];
        char    devmajor[8];
        char    devminor[8];
    } header;
};


enum {
    ZIPHEADERSIZE = 30
};

// structs for reading zip files:
union zipheader {
    char charptr[ZIPHEADERSIZE];
    struct header {
        char signature[4];  //(0x04034b50)
        char versionNeeded[2];
        char generalPurposeFlag[2];
        char compressionMethod[2];
        char lastModTime[2];
        char lastModDate[2];
        char crc32[4];
        char compressedSize[4];
        char uncompressedSize[4];
        char filenameLength[2];
        char extraFieldLength[2];
    } header;
};

enum {
    ZIPEOCDRECORD = 22
};

union zipEOCheader {
    char charptr[ZIPEOCDRECORD];
    struct header {
        char signature[4];  //(0x06054b50)
        char numberOfDisc[2];
        char numberOfDiscWithStartOfCD[2];
        char numberOfEntriesInThisCD[2];
        char numberOfEntriesInCD[2];
        char sizeOfCD[4];
        char offsetStartCD[4];
        char commmentLength[2];
    } header;
};

enum {
    ZIPCENTRALHEADERSIZE = 46
};

union zipcentralheader {
    char charptr[ZIPCENTRALHEADERSIZE];
    struct header {
        char signature[4];  //(0x02014b50)

        char versionMadeBy[2];
        char versionNeeded[2];
        char generalPurposeFlag[2];
        char compressionMethod[2];
        char lastModTime[2];
        char lastModDate[2];

        char crc32[4];
        char compressedSize[4];
        char uncompressedSize[4];

        char filenameLength[2];
        char extraFieldLength[2];
        char fileCommentLength[2];
        char discNumberStart[2];
        char internalFileAttributs[2];

        char externalFileAttributs[4];
        char localHeaderOffset[4];
    } header;
};

} // anonymous namespace

namespace tgt {

File::File()
    : size_(0)
{}

File::File(const std::string& name)
    : name_(name),
      size_(0)
{}

File::~File()
{}

size_t File::findDelim(char* buf, size_t size, char delim) {
    size_t pos = 0;
    while (pos < (size - 1)) {
        if (buf[pos] == delim)
            return pos;
        ++pos;
    }
    return size+1;
}

size_t File::readLine(char *buf, size_t maxCount, char delim) {
    size_t bytesread = 0;
    size_t start = tell();
    char* p = buf;

    if (tell()+maxCount > size())
        maxCount = size()-tell();
    while (bytesread < maxCount) {
        size_t bufsize = TEMP_BUFFERSIZE;
        if ((bytesread + TEMP_BUFFERSIZE) > maxCount)
            bufsize = (maxCount - bytesread);
        read(p, bufsize);
        size_t d = findDelim(p, bufsize, delim);
        if (d < bufsize) {
            bytesread += d;
            p[d] = 0;
            seek(start + bytesread + 1);
            return bytesread;
        }
        p += bufsize;
        bytesread += bufsize;
    }
    return bytesread;
}

std::string File::getLine(char delim) {
    char* buf = new char[LINE_BUFFERSIZE];
    size_t br = readLine(buf, LINE_BUFFERSIZE, delim);
    std::string ret(buf, br);
    while (br == LINE_BUFFERSIZE) {
        size_t br = readLine(buf, LINE_BUFFERSIZE, delim);
        ret.append(buf, br);
    }
    delete[] buf;
    return ret;
}

std::string File::getAsString(void) {
    char* buf = new char[size()];
    read(buf, size());
    std::string ret(buf, size());
    delete[] buf;
    return ret;
}

size_t File::skipLine(char delim) {
    size_t bytesread = 0;
    size_t start = tell();
    char* p = new char[TEMP_BUFFERSIZE];
    while (true) {
        read(p, TEMP_BUFFERSIZE);
        size_t d = findDelim(p, TEMP_BUFFERSIZE, delim);
        if (d < TEMP_BUFFERSIZE) {
            bytesread += d-1;
            delete[] p;
            seek(start + bytesread + 2);
            return bytesread;
        }
        bytesread += TEMP_BUFFERSIZE;
    }
}

//-----------------------------------------------------------------------------

RegularFile::RegularFile(const std::string& filename)
  : File(filename)
{
    file_ = new std::ifstream(filename.c_str(), std::ios::binary);
    name_ = filename;

    // Check if file is open
    if (!file_) {
        std::cout << "Failed to open regular file: " << filename << std::endl;
        return;
    }

    if (!file_->good()) {
        std::cout << "Bad regular file: " << filename << std::endl;
        file_->close();
        delete file_;
        file_ = 0;
        return;
    }

    file_->seekg(0, std::ios::end);
    size_ = tell();
    file_->seekg(std::ios::beg);
}

RegularFile::~RegularFile() {
    close();
}

size_t RegularFile::read(void* buf, size_t count) {
    if (eof())
        return 0;
    if (!file_)
        return 0;

    file_->read(static_cast<char*>(buf), count);
    return file_->gcount();
}

void RegularFile::skip(long count) {
    if (file_)
        file_->seekg(count, std::ios::cur);
}

void RegularFile::seek(size_t pos) {
    if (file_)
        file_->seekg(pos);
}

void RegularFile::seek(size_t offset, File::SeekDir seekDir) {
    if (file_) {
        switch (seekDir)  {
        case File::begin:
            file_->seekg(offset, std::ios_base::beg);
            break;
        case File::current:
            file_->seekg(offset, std::ios_base::cur);
            break;
        case File::end:
            file_->seekg(offset, std::ios_base::end);
            break;
        }
    }
}

size_t RegularFile::tell() const {
    if (file_) {
        std::streampos p = file_->tellg();
        if (p >= 0)
            return static_cast<size_t>(p);
    }
    return 0;
}

bool RegularFile::eof() const {
    if (file_) {
        if (tell() == size())
            return true;
        else
            return file_->eof();
    } else {
        return true;
    }
}

void RegularFile::close() {
    if (file_) {
        file_->close();
        delete file_;
        file_ = 0;
    }
}

bool RegularFile::open() const {
    return (file_ != 0);
}

bool RegularFile::good() const {
    if (file_)
        return file_->good();
    else
        return false;
}

//-----------------------------------------------------------------------------

MemoryFile::MemoryFile(char* data, size_t size, const std::string& filename, bool deleteData)
  : File(filename)
{
    data_ = data;
    size_ = size;
    pos_ = 0;
    deleteData_ = deleteData;
}

MemoryFile::~MemoryFile(){
    if (data_ && deleteData_)
        delete[] data_;
}

size_t MemoryFile::read(void* buf, size_t count) {
    if ((pos_ + count) < size_) {
        memcpy(buf, (const void*) (data_ + pos_), count);
        pos_ += count;
        return count;
    }
    else {
        if (eof())
            return 0;
        size_t r = size_ - pos_;
        memcpy(buf, (const void*) (data_ + pos_), r);
        pos_ = size_ - 1;
        return r;
    }
}

void MemoryFile::skip(long count) {
    pos_ += count;
    if (pos_ >= size_)
        pos_ = size_ - 1;
}

void MemoryFile::seek(size_t pos) {
    pos_ = pos;
    if (pos_ >= size_)
        pos_ = size_ - 1;
}

void MemoryFile::seek(size_t offset, File::SeekDir seekDir) {
    switch (seekDir) {
    case File::begin :
        seek(offset);
        break;
    case File::current :
        seek(pos_+offset);
        break;
    case File::end :
        pos_ = size_ - 1;
        break;
    }
}

size_t MemoryFile::tell() const {
    return pos_;
}

bool MemoryFile::eof() const {
    return (pos_ == size_-1);
}

void MemoryFile::close() {
    pos_ = 0;
}

bool MemoryFile::open() const {
    return true;
}

bool MemoryFile::good() const {
    // only possibility not to be good is on EOF
    return !eof();
}

//-----------------------------------------------------------------------------

const std::string TarFile::loggerCat_("tgt.TarFile");

TarFile::TarFile(const std::string& filename, const std::string& tarfilename, size_t offset, size_t size)
    : File(filename)
{
    file_ = FileSys.open(tarfilename.c_str());
    name_ = filename;

    // Check if file is open:
    if (!file_) {
        std::cout << "Failed to open tar file: " << tarfilename << std::endl;
        return;
    }

    if (!file_->good()) {
        std::cout << "Bad tar file: " << tarfilename << std::endl;
        file_->close();
        delete file_;
        file_ = 0;
        return;
    }

#ifdef TGT_DEBUG
    file_->seek(0, File::end);
    size_ = file_->tell();
    file_->seek(File::begin);

    if ((offset + size) > size_) {
        std::cout << "Wrong tar file size/offset! file: " << filename << " in " << tarfilename << std::endl;
        std::cout << "size_ " << size_ << " offset " << offset << " size " << size << std::endl;
        file_->close();
        delete file_;
        file_ = 0;
        return;
    }
#endif // TGT_DEBUG

    size_ = size;
    offset_ = offset;
    file_->seek(offset_, File::begin);
}

TarFile::~TarFile() {
    close();
}

size_t TarFile::read(void* buf, size_t count) {
    if (tell() + count > size_)
        count = size_ - tell();
    if (!file_)
        return 0;
    file_->read(static_cast<char*>(buf), count);
    return count;
}

void TarFile::skip(long count) {
    if (tell() + count > size_)
        count = size_ - tell();

    if (file_)
        file_->seek(count, File::current);
}

void TarFile::seek(size_t pos) {
    if (file_)
        file_->seek(offset_ + pos);
}

void TarFile::seek(size_t offset, File::SeekDir seekDir) {
    if (file_)
        file_->seek(offset, seekDir);
}

size_t TarFile::tell() const {
    if (file_)
        return ((size_t)(file_->tell())) - offset_;
    else
        return 0;
}

bool TarFile::eof() const {
    if (file_)
       return (tell() == size_);
    else
        return true;
}

void TarFile::close() {
    if (file_) {
        file_->close();
        delete file_;
        file_ = 0;
    }
}

bool TarFile::open() const {
    return (file_ != 0);
}

bool TarFile::good() const {
    if (eof())
        return false;
        
    if (file_)
        return file_->good();
    else
        return false;
}


//-----------------------------------------------------------------------------

MemoryFileFactory::MemoryFileFactory(const std::string& filename, char* data, size_t size) {
    data_ = data;
    size_ = size;
    filename_ = filename;
}

MemoryFileFactory::MemoryFileFactory(const std::string& filename, const std::string& data) {
    size_ = data.length();
    data_ = new char[size_];
    memcpy(data_, data.c_str(), size_);
    filename_ = filename;
}

File* MemoryFileFactory::open(const std::string& filename) {
    if (filename == filename_)
        return new MemoryFile(data_, size_, filename_);
    else
        return 0;
}

std::vector<std::string> MemoryFileFactory::getFilenames() {
     std::vector<std::string> files;
     files.push_back(filename_);
     return files;
}

//-----------------------------------------------------------------------------

const std::string TarFileFactory::loggerCat_("tgt.TarFileFactory");

TarFileFactory::TarFileFactory(const std::string& filename, const std::string& rootpath) {
    filename_ = filename;
    File* file = FileSys.open(filename);

    // Check if file is open
    if (!file) {
        LERROR("Failed to open TAR archive " << filename);
        return;
    }

    if (!file->good()) {
        LERROR("Error opening TAR archive " << filename);
        return;
    }

    bool end = false;

    record* currecord = new record;

    while (!end) {
        file->read(currecord->charptr, RECORDSIZE);

        bool allzero = true;
        //check if record is all zero (=>eof)
        for (int i = 0; i < 512; i++) {
            if (currecord->charptr[i] != 0)
                allzero = false;
        }

        if (!allzero) {
            //convert from ascii-octal to normal int
            size_t filesize = 0;
            size_t mul = 1;
            for (int i = 10; i >= 0; i--) {
                filesize += ((currecord->header.size[i] - 48) * mul);
                mul *= 8;
            }

            if (filesize > 0) {
                size_t blocks = (filesize / RECORDSIZE);
                if ((filesize % RECORDSIZE) != 0)
                    ++blocks;
				LDEBUG("Found file " << currecord->header.name);
				LDEBUG("  Add it as " << rootpath+currecord->header.name
                       << "; filesize: " << filesize << "; offset " << file->tell());
                files_[rootpath+currecord->header.name].size_ = filesize;
                files_[rootpath+currecord->header.name].offset_ = file->tell();

                file->seek(RECORDSIZE*blocks, File::current);
            }

            if (file->eof())
                end = true;
        }
        else
            end = true;
    }
    file->close();
    delete file;
}

File* TarFileFactory::open(const std::string& filename) {
	LDEBUG("Loading file " << filename << " from " << filename_);
	LDEBUG("  Offset: " << files_[filename].offset_ << "; size: " << files_[filename].size_);
    TarFile* tf = new TarFile(filename, filename_, files_[filename].offset_,
                              files_[filename].size_);
    return tf;
}

std::vector<std::string> TarFileFactory::getFilenames() {
    std::vector<std::string> files;

    std::map<std::string, ArchivedFile>::iterator theIterator;
    for (theIterator = files_.begin(); theIterator != files_.end(); theIterator++)
        files.push_back((*theIterator).first);

    return files;
}

//-----------------------------------------------------------------------------

const std::string ZipFileFactory::loggerCat_("tgt.FileSystem.ZipFileFactory");

ZipFileFactory::ZipFileFactory(const std::string& filename, const std::string& rootpath) {
    filename_ = filename;
    file_ = FileSys.open(filename.c_str());
    // Check if file is open
    if (!file_) {
        LERROR("Failed to open Zip archive " << filename);
        return;
    }

    if (!file_->good()) {
        LERROR("Error opening Zip archive " << filename);
        return;
    }

    LDEBUG("Reading Zip archive...");

    LDEBUG("Reading central directory end header:");
    file_->seek(-ZIPEOCDRECORD, File::end);
    zipEOCheader* eocheader = new zipEOCheader;
    file_->read(eocheader->charptr, ZIPEOCDRECORD);

#ifndef tgtNOZIPCHECKS
    if (*(reinterpret_cast<unsigned int*>(eocheader->header.signature)) != 0x06054b50) {
        LERROR("Signature of central directoy end header does not match!");
        return;
    }
#endif

    unsigned int startOfCD = *(reinterpret_cast<unsigned int*>(eocheader->header.offsetStartCD));
    LDEBUG("Start offset of CD: " << startOfCD);

    file_->seek(startOfCD, File::begin);
    unsigned short int entries =
        *(unsigned short int*)eocheader->header.numberOfEntriesInThisCD;
    
    LDEBUG("Reading " << entries << "entries in central directory...");

    for (int i=0; i<entries; i++) {
        zipcentralheader* h = new zipcentralheader;
        file_->read(h->charptr, ZIPCENTRALHEADERSIZE);
#ifndef tgtNOZIPCHECKS
        if (*(reinterpret_cast<unsigned int*>(h->header.signature)) != 0x02014b50) {
            LERROR("Signature in central directoy header does not match!");
            return;
        }
#endif
        unsigned short int fnlength = *(unsigned short int*) h->header.filenameLength;
        unsigned short int extralength = *(unsigned short int*) h->header.extraFieldLength;
        unsigned short int commentlength = *(unsigned short int*) h->header.fileCommentLength;

        char* fname = new char[fnlength+1];
        file_->read(fname, fnlength);
        fname[fnlength] = 0;
        LDEBUG("Filename: " << fname);

        unsigned short int* compMeth = (unsigned short int*) h->header.compressionMethod;
        LDEBUG("Compression method: " << *compMeth);

        switch (*compMeth) {
        case 0:
            LDEBUG("The file is stored (no compression)");
            files_[rootpath+fname].offset_ = *(reinterpret_cast<unsigned int*>(h->header.localHeaderOffset));
            LDEBUG("Added file " << fname << "@" << files_[rootpath+fname].offset_);
            break;
        case 1:
            LWARNING("The file is Shrunk");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 2:
            LWARNING("The file is Reduced with compression factor 1");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 3:
            LWARNING("The file is Reduced with compression factor 2");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 4:
            LWARNING("The file is Reduced with compression factor 3");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 5:
            LWARNING("The file is Reduced with compression factor 4");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 6:
            LWARNING("The file is Imploded");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 7:
            LWARNING("Reserved for Tokenizing compression algorithm");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 8:
            LDEBUG("The file is Deflated");
#ifdef TGT_HAS_ZLIB
            files_[rootpath+fname].offset_ = *(reinterpret_cast<unsigned int*>(h->header.localHeaderOffset));
            LDEBUG("Added file " << fname << "@" << files_[rootpath+fname].offset_);
#else
            LWARNING("tgt was compiled without zlib support, file cannot be read and will not "
                     "be added to filesystem!");
#endif
            break;
        case 9:
            LWARNING("Enhanced Deflating using Deflate64(tm)");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        case 10:
            LWARNING("PKWARE Date Compression Library Imploding");
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
            break;
        default:
            LWARNING("Compression method unsupported, file will not be added to filesystem!");
        }

        file_->seek(extralength+commentlength, File::current);
    }
}

ZipFileFactory::~ZipFileFactory() {
    if (file_) {
        file_->close();
        delete file_;
    }
}

File* ZipFileFactory::open(const std::string& filename) {
    std::map<std::string, ArchivedFile>::iterator it = files_.find(filename);
    
    if (it == files_.end())
        return 0;

    file_->seek(it->second.offset_, File::begin);

    zipheader* currecord = new zipheader;
    file_->read(currecord->charptr, ZIPHEADERSIZE);

#ifndef tgtNOZIPCHECKS
    LDEBUG("FileHeader:");
    if (*(reinterpret_cast<unsigned int*>(currecord->header.signature)) != 0x04034b50) {
        LERROR("Local signature does not match!");
        delete currecord;
        return 0;
    }
    LDEBUG("Version needed: " << (static_cast<int>(currecord->header.versionNeeded[0])) / 10 << "."
           << (static_cast<int>(currecord->header.versionNeeded[0])) % 10);
#endif

    unsigned short int* genPurp = (unsigned short int*) currecord->header.generalPurposeFlag;

    unsigned short int genpf = *genPurp;
    char gpf[17];
    for (int i=0; i<16; i++) {
        gpf[i] = (genpf % 2) + 48;
        genpf /= 2;
    }
    gpf[16] = 0;
    LDEBUG("General purpose flag: " << gpf);
    LDEBUG("General purpose flag (as int): " << genpf);

    unsigned short int* compMeth = (unsigned short int*) currecord->header.compressionMethod;
    LDEBUG("Compression method: " << *compMeth);

    switch (*compMeth) {
    case 0:
        LDEBUG("The file is stored (no compression)");
        break;
    case 1:
        LERROR("The file is Shrunk");
        delete currecord;
        return 0;
    case 2:
        LERROR("The file is Reduced with compression factor 1");
        delete currecord;
        return 0;
    case 3:
        LERROR("The file is Reduced with compression factor 2");
        delete currecord;
        return 0;
    case 4:
        LERROR("The file is Reduced with compression factor 3");
        delete currecord;
        return 0;
    case 5:
        LERROR("The file is Reduced with compression factor 4");
        delete currecord;
        return 0;
    case 6:
        LERROR("The file is Imploded");
        delete currecord;
        return 0;
    case 7:
        LERROR("Reserved for Tokenizing compression algorithm");
        delete currecord;
        return 0;
    case 8:
        LDEBUG("The file is Deflated");
        break;
    case 9:
        LERROR("Enhanced Deflating using Deflate64(tm)");
        delete currecord;
        return 0;
    case 10:
        LERROR("PKWARE Date Compression Library Imploding");
        delete currecord;
        return 0;
    default:
        LERROR("Unknown compression method (>10)");
        delete currecord;
        return 0;
    }

    unsigned int csize = *reinterpret_cast<unsigned int*>(currecord->header.compressedSize);
    LDEBUG("Compressed size: " << csize);
    LDEBUG("Uncompressed size: " << *reinterpret_cast<unsigned int*>(currecord->header.uncompressedSize));
    unsigned short int* fnlength = (unsigned short int*) currecord->header.filenameLength;
    unsigned short int* extralength = (unsigned short int*) currecord->header.extraFieldLength;
    LDEBUG("Filename length: " << *fnlength);
    LDEBUG("Extra field length: " << *extralength);

    char* fname = new char[(*fnlength)+1];
    file_->read(fname, (*fnlength));
    fname[(*fnlength)] = 0;
    LDEBUG("Filename: " << fname);

    file_->seek((*extralength), File::current);

    size_t c = csize;

    File* f;

    if (*compMeth == 8) {
#ifdef TGT_HAS_ZLIB
        char* cdata = new char[c+2];
        file_->read(cdata+2, c);
        //Zlib's Uncompress method expects the 2 byte head that the Compress method adds
        //so we put that on first. Luckily it's always the same value.

        cdata[0] = 120;
        cdata[1] = 156;
        uLongf uc = *reinterpret_cast<unsigned int*>(currecord->header.uncompressedSize);
        uc += 12;
        char* ucdata = new char[uc];
        //deflate using zlib:
        switch (uncompress ((Bytef*) ucdata, &uc, (Bytef*) cdata, c + 2)) {
        case Z_OK:
            LDEBUG("success");
            break;
        case Z_MEM_ERROR:
            LDEBUG("there was not enough memory");
            break;
        case Z_BUF_ERROR:
            LDEBUG("there was not enough room in the output buffer");
            break;
        case Z_DATA_ERROR:
            LDEBUG("DATA ERROR");
            break;
        default:
            LDEBUG("DEFAULT");
        }
        f = new MemoryFile(ucdata, uc, filename, true);
        delete[] cdata;
#endif
    }
    else {
//         char* cdata = new char[c];
//         file_->read(cdata, c);
//         f = new MemoryFile(cdata, c, filename, true);

        f = new TarFile(filename, filename_, file_->tell(), c);
    }
    return f;
}

std::vector<std::string> ZipFileFactory::getFilenames() {
    std::vector<std::string> files;
//FIXME: this does nothing
    std::map<std::string, ArchivedFile>::iterator theIterator;
    for (theIterator = files_.begin(); theIterator != files_.end(); theIterator++)
        files.push_back((*theIterator).first);

    return files;
}

//-----------------------------------------------------------------------------

const std::string FileSystem::loggerCat_("tgt.FileSystem.FileSystem");

FileSystem::~FileSystem() {
    while (!factories_.empty()) {
        delete (factories_.back());
        factories_.pop_back();
    }
}

File* FileSystem::open(const std::string& filename) {
    if (virtualFS_.find(filename) != virtualFS_.end()) {
        LDEBUG("Opening file " << filename << " from virtualFS");
        return virtualFS_[filename]->open(filename);
    } else {
        LDEBUG("Opening file " << filename << " from real FS");

        RegularFile* f = new RegularFile(filename);
        if (f->open())
            return f;
        else
            return 0;
    }
}

bool FileSystem::exists(const std::string& filename) {
    if (virtualFS_.find(filename) != virtualFS_.end()) {
        LDEBUG("Checking if file " << filename << " exists in virtualFS");
		return true;
    } else {
		bool result;
        LDEBUG("Opening file " << filename << " from real FS");

		std::ifstream* file = new std::ifstream(filename.c_str(), std::ios::binary);

		//Check if file is open:
		if (!file) result=false;
		else result=file->good();

		if (result)
			file->close();

		return result;
	}
}

void FileSystem::addFactory(FileFactory* ff) {
    std::vector<std::string> files = ff->getFilenames();
    std::vector<std::string>::iterator theIterator;
    for (theIterator = files.begin(); theIterator != files.end(); theIterator++)
        virtualFS_[*theIterator] = ff;

    factories_.push_back(ff);
}

void FileSystem::addMemoryFile(const std::string& filename, char* data, size_t size) {
    FileFactory* ff = new MemoryFileFactory(filename, data, size);
    addFactory(ff);
}

void FileSystem::addMemoryFile(const std::string& filename, const std::string& data) {
    FileFactory* ff = new MemoryFileFactory(filename, data);
    addFactory(ff);
}

void FileSystem::addPackage(const std::string& filename, const std::string& rootpath) {
    LINFO("adding package " << filename);
	LDEBUG("Root path: " << rootpath);

    //TODO: do something like TextureReader in TexMgr...
    std::string::size_type loc = filename.find(".zip", 0);
	if (loc != std::string::npos) {
		LDEBUG("Recognized zip file");
        addFactory(new ZipFileFactory(filename, rootpath));
	}
    loc = filename.find(".3dp", 0);
	if (loc != std::string::npos) {
        LDEBUG("Recognized 3dp file");
		addFactory(new ZipFileFactory(filename, rootpath));
	}
    loc = filename.find(".tar", 0);
	if (loc != std::string::npos) {
		LDEBUG("Recognized tar file");
        addFactory(new TarFileFactory(filename, rootpath));
	}
}

} // namespace
