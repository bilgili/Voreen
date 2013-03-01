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

#include "voreen/core/utils/regressiontest/filecomparators.h"

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/vector.h"
#include "tgt/matrix.h"

#include "voreen/core/utils/stringutils.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumefactory.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "modules/core/io/vvdvolumereader.h"

#ifdef VRN_MODULE_DEVIL
#include <GL/glew.h>
#endif

#include <iostream>

namespace voreen {

using namespace tgt;

// some compare functions for primitive types
bool compareFloat(float a, float b, float epsilon) {
    return (std::abs(a-b) <= epsilon);
}
bool compareDouble(double a, double b, float epsilon) {
    return (std::abs(a-b) <= epsilon);
}

template<typename T>
bool compareVec(T a, T b, float epsilon) {
    tgtAssert(epsilon >= 0.f, "negative epsilon");
    return (tgt::max(tgt::abs(a-b)) <= epsilon);
}

bool compareMat4(mat4 a, mat4 b, float epsilon) {
    tgtAssert(epsilon >= 0.f, "negative epsilon");
    return (compareVec(a[0], b[0], epsilon) &&
        compareVec(a[1], b[1], epsilon) &&
        compareVec(a[2], b[2], epsilon) &&
        compareVec(a[3], b[3], epsilon)    );
}

bool compareMat3(mat3 a, mat3 b, float epsilon) {
    tgtAssert(epsilon >= 0.f, "negative epsilon");
    return (compareVec(a[0], b[0], epsilon) &&
            compareVec(a[1], b[1], epsilon) &&
            compareVec(a[2], b[2], epsilon)    );
}

bool compareMat2(mat2 a, mat2 b, float epsilon) {
    tgtAssert(epsilon >= 0.f, "negative epsilon");
    return (compareVec(a[0], b[0], epsilon) &&
            compareVec(a[1], b[1], epsilon)    );
}

bool compareMetaData(const MetaDataBase* a, const MetaDataBase* b, float epsilon) {
    tgtAssert(a && b, "null pointer passed");
    if (typeid(*a) != typeid(*b)) // check if types match
        return false;

    // compare float types with epsilon
    if (dynamic_cast<const FloatMetaData*>(a)) {
        return compareFloat(static_cast<const FloatMetaData*>(a)->getValue(),
            static_cast<const FloatMetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const DoubleMetaData*>(a)) {
        return compareDouble(static_cast<const DoubleMetaData*>(a)->getValue(),
            static_cast<const DoubleMetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Vec2MetaData*>(a)) {
        return compareVec(static_cast<const Vec2MetaData*>(a)->getValue(),
            static_cast<const Vec2MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Vec3MetaData*>(a)) {
        return compareVec(static_cast<const Vec3MetaData*>(a)->getValue(),
            static_cast<const Vec3MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Vec4MetaData*>(a)) {
        return compareVec(static_cast<const Vec4MetaData*>(a)->getValue(),
            static_cast<const Vec4MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const DVec2MetaData*>(a)) {
        return compareVec(static_cast<const DVec2MetaData*>(a)->getValue(),
            static_cast<const DVec2MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const DVec3MetaData*>(a)) {
        return compareVec(static_cast<const DVec3MetaData*>(a)->getValue(),
            static_cast<const DVec3MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const DVec4MetaData*>(a)) {
        return compareVec(static_cast<const DVec4MetaData*>(a)->getValue(),
            static_cast<const DVec4MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Mat2MetaData*>(a)) {
        return compareMat2(static_cast<const Mat2MetaData*>(a)->getValue(),
            static_cast<const Mat2MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Mat3MetaData*>(a)) {
        return compareMat3(static_cast<const Mat3MetaData*>(a)->getValue(),
            static_cast<const Mat3MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const Mat4MetaData*>(a)) {
        return compareMat4(static_cast<const Mat4MetaData*>(a)->getValue(),
            static_cast<const Mat4MetaData*>(b)->getValue(), epsilon);
    }
    else if (dynamic_cast<const RealWorldMappingMetaData*>(a)) {
        RealWorldMapping rwma = static_cast<const RealWorldMappingMetaData*>(a)->getValue();
        RealWorldMapping rwmb = static_cast<const RealWorldMappingMetaData*>(b)->getValue();
        if (!compareFloat(rwma.getOffset(), rwmb.getOffset(), epsilon))
            return false;
        if (!compareFloat(rwma.getScale(), rwmb.getScale(), epsilon))
            return false;
        if (rwma.getUnit() != rwmb.getUnit())
            return false;
        return true;
    }
    else {
        // compare remaining types as string
        return (a->toString() == b->toString());
    }
}

//----------------------------------------------------------------------------------------

void BinaryFileComparator::determineFileType(RegressionTestDataset& dataset) const {
    for (size_t i=0; i<dataset.files_.size(); i++)
        dataset.files_.at(i).fileType_ = BinaryFile;
}

bool BinaryFileComparator::compare(RegressionTestDataset& refDataset, RegressionTestDataset& outputDataset,
        std::string& report, const RegressionTestCase& testCase) const
{

    if (refDataset.files_.size() != outputDataset.files_.size()) {
        report = "BinaryFileComparator: file count does not match";
        return false;
    }

    for (size_t i=0; i<refDataset.files_.size(); i++) {

        std::string refFile = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refDataset.files_.at(i).filename_);
        refDataset.files_.at(i).fileType_ = BinaryFile;

        std::string outputFile = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputDataset.files_.at(i).filename_);
        outputDataset.files_.at(i).fileType_ = BinaryFile;

        try {
            RegularFile f1(refFile);
            if (!f1.isOpen())
                throw VoreenException("Unable to open reference file: " + refFile);
            RegularFile f2(outputFile);
            if (!f2.isOpen())
                throw VoreenException("Unable to open output file: " + outputFile);

            const int N = 10000;
            char buf1[N];
            char buf2[N];

            bool result = true;
            do {
                size_t r1 = f1.read(buf1, N);
                size_t r2 = f2.read(buf2, N);

                if (r1 != r2 || memcmp(buf1, buf2, r1))
                    result = false;
            }
            while (result && !f1.eof() && !f2.eof());
            result &= f1.eof() && f2.eof();

            if (!result) {
                report = "files are not binary equal";
                return false;
            }
        }
        catch (std::exception& e) {
            report = "BinaryFileComparator: exception during file comparison: " + std::string(e.what());
            return false;
        }
    }

    report = "files are binary equal";
    return true;
}

void BinaryFileComparator::generateDiffFile(const RegressionTestDataset& /*refDataset*/, const RegressionTestDataset& /*outputDataset*/,
        RegressionTestDataset& /*diffDataset*/, const RegressionTestCase& /*testCase*/) const throw (VoreenException)
{
    tgtAssert(false, "Generation of binary file diffs not supported");
    throw VoreenException("Generation of binary file diffs not supported");
}

//----------------------------------------------------------------------------------------

TextFileComparator::TextFileComparator() {
    supportedExtensions_.insert("txt");
    supportedExtensions_.insert("csv");
    supportedExtensions_.insert("vge"); //< Voreen Geometry files (XML)
    supportedExtensions_.insert("vvd"); //< Voreen Volume Data (XML)
}

bool TextFileComparator::supportsFormat(const RegressionTestDataset& dataset) const {
    // only single text files supported
    if (dataset.files_.size() != 1)
        return false;
    else {
        std::string filename = dataset.files_.at(0).filename_;
        return (supportedExtensions_.find(FileSystem::fileExtension(filename, true)) != supportedExtensions_.end());
    }
}

void TextFileComparator::determineFileType(RegressionTestDataset& dataset) const {
    if (supportsFormat(dataset))
        dataset.files_.at(0).fileType_ = TextFile;
    else
        for (size_t i=0; i<dataset.files_.size(); i++)
            dataset.files_.at(i).fileType_ = FileTypeUnknown;
}

bool TextFileComparator::compare(RegressionTestDataset& refDataset, RegressionTestDataset& outputDataset,
    std::string& report, const RegressionTestCase& testCase) const
{

    // check parameters
    if (refDataset.files_.size() != outputDataset.files_.size()) {
        tgtAssert(false, "file count of reference and output datasets differs (should have been checked by caller)");
        report = "file count of reference and output datasets differs";
        return false;
    }

    if (refDataset.files_.size() != 1) {
        tgtAssert(false, "dataset consists of multiple files (should have been rejected by supportsFormat()");
        report = "TextFileComparator: dataset consists of multiple files";
        return false;
    }

    std::string refFile = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refDataset.files_.at(0).filename_);
    refDataset.files_.at(0).fileType_ = TextFile;

    std::string outputFile = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputDataset.files_.at(0).filename_);
    outputDataset.files_.at(0).fileType_ = TextFile;

    // load text strings
    std::string refString, outputString;
    try {
        refString = loadTextFile(refFile);
        outputString = loadTextFile(outputFile);
    }
    catch (VoreenException& e) {
        report = "TextFileComparator: exception during file comparison: " + std::string(e.what());
        return false;
    }

    // harmonize line endings
    refString = strReplaceAll(refString, "\r\n", "\n");
    refString = strReplaceAll(refString, "\r", "\n");
    outputString = strReplaceAll(outputString, "\r\n", "\n");
    outputString = strReplaceAll(outputString, "\r", "\n");

    // compare harmonized strings
    if (refString == outputString) {
        report = "text files are equal";
        return true;
    }
    else {
        report = "text files are not equal";
        return false;
    }

}

void TextFileComparator::generateDiffFile(const RegressionTestDataset& /*refDataset*/, const RegressionTestDataset& /*outputDataset*/,
        RegressionTestDataset& /*diffDataset*/, const RegressionTestCase& /*testCase*/)
        const throw (VoreenException)
{
    tgtAssert(false, "Generation of text file diffs not supported");
    throw VoreenException("Generation of text file diffs not supported");
}

std::string TextFileComparator::loadTextFile(const std::string& filename) const throw (VoreenException) {
    RegularFile file(filename);
    if (!file.isOpen())
        throw VoreenException("Unable to open text file for reading: " + filename);
    return file.getAsString();
}

/*void TextFileComparator::saveTextToFile(const std::string& text, const std::string& filename) const
    throw (VoreenException)
{} */

//----------------------------------------------------------------------------------------

const std::string VgeFileComparator::loggerCat_("regressiontest.VgeFileComparator");

VgeFileComparator::VgeFileComparator(float geometryDiffTolerance)
    : geometryDiffTolerance_(geometryDiffTolerance)
{}

bool VgeFileComparator::supportsFormat(const RegressionTestDataset& dataset) const {
    // only single text files supported
    if (dataset.files_.size() != 1)
        return false;
    else
        return (FileSystem::fileExtension(dataset.files_.at(0).filename_) == "vge");
}

void VgeFileComparator::determineFileType(RegressionTestDataset& dataset) const {
    if (supportsFormat(dataset))
        dataset.files_.at(0).fileType_ = TextFile;
    else
        for (size_t i=0; i<dataset.files_.size(); i++)
            dataset.files_.at(i).fileType_ = FileTypeUnknown;
}

bool VgeFileComparator::compare(RegressionTestDataset& refDataset, RegressionTestDataset& outputDataset,
    std::string& report, const RegressionTestCase& testCase) const
{

    // check parameters
    if (refDataset.files_.size() != outputDataset.files_.size()) {
        tgtAssert(false, "file count of reference and output datasets differs (should have been checked by caller)");
        report = "file count of reference and output datasets differs";
        return false;
    }

    if (refDataset.files_.size() != 1) {
        tgtAssert(false, "dataset consists of multiple files (should have been rejected by supportsFormat()");
        report = "VgeFileComparator: dataset consists of multiple files";
        return false;
    }

    std::string refFile = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refDataset.files_.at(0).filename_);
    refDataset.files_.at(0).fileType_ = TextFile;

    std::string outputFile = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputDataset.files_.at(0).filename_);
    outputDataset.files_.at(0).fileType_ = TextFile;

    // load Voreen Geometry from strings
    Geometry* refGeometry = 0;
    Geometry* outputGeometry = 0;
    try {
        refGeometry = loadGeometry(refFile);
        outputGeometry = loadGeometry(outputFile);
    }
    catch (VoreenException& e) {
        delete refGeometry;
        report = "VgeFileComparator: exception during file comparison: " + std::string(e.what());
        return false;
    }
    tgtAssert(refGeometry, "no ref geometry");
    tgtAssert(outputGeometry, "no output geometry");

    float geometryDiffTolerance = (testCase.config_.geometryDiffTolerance_ >= 0.f ? testCase.config_.geometryDiffTolerance_ : geometryDiffTolerance_);

    // compare geometries
    bool result;
    if (refGeometry->equals(outputGeometry, 0.0)) {
        result = true;
        report = "geometries are equal";
    }
    else if (refGeometry->equals(outputGeometry, geometryDiffTolerance)) {
        result = true;
        report = "geometry difference is below threshold";
    }
    else {
        result = false;
        report = "geometries are not equal";
    }

    delete refGeometry;
    delete outputGeometry;
    return result;
}

void VgeFileComparator::generateDiffFile(const RegressionTestDataset& /*refDataset*/, const RegressionTestDataset& /*outputDataset*/,
        RegressionTestDataset& /*diffDataset*/, const RegressionTestCase& /*testCase*/)
        const throw (VoreenException)
{
    tgtAssert(false, "Generation of Geometry file diffs not supported");
    throw VoreenException("Generation of text file diffs not supported");
}

Geometry* VgeFileComparator::loadGeometry(const std::string& filename) const throw (VoreenException) {
    LDEBUG("Reading Geometry file: " << filename);
    Geometry* geometry = 0;
    std::ifstream geomFile(filename.c_str(), std::ios_base::in);
    if (geomFile.good()) {
        XmlDeserializer deserializer;
        try {
            deserializer.read(geomFile);
            deserializer.deserialize("Geometry", geometry);
        }
        catch (SerializationException& e) {
            throw VoreenException("Failed to deserialize Geometry file '" + filename + "': " + e.what());
        }
        geomFile.close();
    }
    else {
        throw VoreenException("Failed to open Geomtry file for reading: " + filename);
    }
    tgtAssert(geometry, "geometry is null");

    return geometry;
}

//----------------------------------------------------------------------------------------

#ifdef VRN_MODULE_DEVIL

ImageFileComparator::ImageFileComparator(float pixelDiffTolerance, int maxErrorPixels,
        int pixelSearchNeighborhood, float diffImageGamma, bool diffImageFullAlpha)
{
    pixelDiffTolerance_ = pixelDiffTolerance;
    maxErrorPixels_ = maxErrorPixels;
    pixelSearchNeighborhood_ = pixelSearchNeighborhood;
    diffImageGamma_ = diffImageGamma;
    diffImageFullAlpha_ = diffImageFullAlpha;

    supportedExtensions_.insert("bmp");
    supportedExtensions_.insert("jpg");
    supportedExtensions_.insert("jpeg");
    supportedExtensions_.insert("png");
    supportedExtensions_.insert("tif");
    supportedExtensions_.insert("tiff");
    supportedExtensions_.insert("gif");
    supportedExtensions_.insert("tga");
}

bool ImageFileComparator::supportsFormat(const RegressionTestDataset& dataset) const {
    // only single images supported
    if (dataset.files_.size() != 1)
        return false;
    else {
        std::string filename = dataset.files_.at(0).filename_;
        return (supportedExtensions_.find(FileSystem::fileExtension(filename, true)) != supportedExtensions_.end());
    }
}

void ImageFileComparator::determineFileType(RegressionTestDataset& dataset) const {
    if (supportsFormat(dataset))
        dataset.files_.at(0).fileType_ = ImageFile;
    else
        for (size_t i=0; i<dataset.files_.size(); i++)
            dataset.files_.at(i).fileType_ = FileTypeUnknown;
}

bool ImageFileComparator::compare(RegressionTestDataset& refDataset, RegressionTestDataset& outputDataset,
    std::string& report, const RegressionTestCase& testCase) const
{

    // check parameters
    if (refDataset.files_.size() != outputDataset.files_.size()) {
        tgtAssert(false, "file count of reference and output datasets differs (should have been checked by caller)");
        report = "file count of reference and output datasets differs";
        return false;
    }

    if (refDataset.files_.size() != 1) {
        tgtAssert(false, "dataset consists of multiple files (should have been rejected by supportsFormat()");
        report = "TextFileComparator: dataset consists of multiple files";
        return false;
    }

    std::string refFile = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refDataset.files_.at(0).filename_);
    refDataset.files_.at(0).fileType_ = ImageFile;

    std::string outputFile = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputDataset.files_.at(0).filename_);
    outputDataset.files_.at(0).fileType_ = ImageFile;

    // load ref image
    ILuint refImage;
    try {
        refImage = loadImage(refFile);
    }
    catch (VoreenException& e) {
        report = "failed to load reference image '" + refFile + "': " + e.what();
        return false;
    }

    // load output image
    ILuint outputImage;
    try {
        outputImage = loadImage(outputFile);
    }
    catch (VoreenException& e) {
        report = "failed to load output image '" + outputFile + "': " + e.what();
        ilDeleteImage(refImage);
        return false;
    }

    // compare image properties
    tgt::ivec2 refDim = getImageDimensions(refImage);
    tgt::ivec2 outputDim = getImageDimensions(outputImage);
    if (refDim != outputDim) {
        report = "images differ in size: reference=" + genericToString(refDim) + ", output=" + genericToString(outputDim);
        ilDeleteImage(refImage);
        ilDeleteImage(outputImage);
        return false;
    }
    size_t numPixels = tgt::hmul(refDim);

    GLint refFormat = getImageFormat(refImage);
    GLint outputFormat = getImageFormat(outputImage);
    if (refFormat != outputFormat) {
        report = "images format differs: reference=" + genericToString(refFormat) + ", output=" + genericToString(outputFormat);
        ilDeleteImage(refImage);
        ilDeleteImage(outputImage);
        return false;
    }

    /*GLint refDataType = getDataType(refImage);
    GLint outputDataType = getDataType(outputImage);
    if (refDataType != outputDataType) {
        report = "images data type differs: reference=" + genericToString(refDataType) + ", output=" + genericToString(outputDataType);
        ilDeleteImage(refImage);
        ilDeleteImage(outputImage);
        return false;
    } */

    // extract pixel data as RGBA float
    tgt::vec4* refBuffer = new tgt::vec4[numPixels];
    ilBindImage(refImage);
    ilCopyPixels(0, 0, 0, refDim.x, refDim.y, 1, IL_RGBA, IL_FLOAT, refBuffer);

    tgt::vec4* outputBuffer = new tgt::vec4[numPixels];
    ilBindImage(outputImage);
    ilCopyPixels(0, 0, 0, refDim.x, refDim.y, 1, IL_RGBA, IL_FLOAT, outputBuffer);

    // take thresholds from test case config if valid, member values otherwise
    float pixelDiffTolerance = (testCase.config_.pixelDiffTolerance_ >= 0.f ? testCase.config_.pixelDiffTolerance_ : pixelDiffTolerance_);
    int maxErrorPixels = (testCase.config_.maxErrorPixels_ >= 0 ? testCase.config_.maxErrorPixels_ : maxErrorPixels_);
    int pixelSearchNeighborhood = (testCase.config_.pixelSearchNeighborhood_ >= 0 ? testCase.config_.pixelSearchNeighborhood_ : pixelSearchNeighborhood_);

    // compare pixel buffers pixel by pixel
    size_t numDiff = 0;
    size_t numDiffAboveThreshold = 0;
    float maxDist = 0.f;
    for (int y=0; y<refDim.y; y++) {
        for (int x=0; x<refDim.x; x++) {
            int index = y*refDim.x + x;
            tgtAssert(index >= 0 && index < (int)numPixels, "invalid index");
            tgt::vec4 outputCol = outputBuffer[index];
            float dist = tgt::length(refBuffer[index] - outputCol);
            dist /= 2.f; //< normalize black-white distance to 1.0

            if (dist > 0.f)
                numDiff++;
            maxDist = std::max(maxDist, dist);

            if (dist > pixelDiffTolerance) {
                // search neighborhood for matching pixel
                bool match = false;
                for (int ny = y-pixelSearchNeighborhood; (ny <= y+pixelSearchNeighborhood) && !match; ny++) {
                    for (int nx = x-pixelSearchNeighborhood; (nx <= x+pixelSearchNeighborhood) && !match; nx++) {
                        if (nx < 0 || nx >= refDim.x || ny < 0 || ny >= refDim.y)
                            continue;
                        int nIndex = ny*refDim.x + nx;
                        tgtAssert(nIndex >= 0 && nIndex < (int)numPixels, "invalid index");
                        float nDist = tgt::length(refBuffer[nIndex] - outputCol);
                        nDist /= 2.f; //< normalize black-white distance to 1.0
                        if (nDist <= pixelDiffTolerance)
                            match = true;
                    }
                }

                if (!match)
                    numDiffAboveThreshold++;
            }
        }
    }

    // free data
    delete[] refBuffer;
    delete[] outputBuffer;
    ilDeleteImage(refImage);
    ilDeleteImage(outputImage);

    // analyze results
    if (numDiff == 0) {
        report = "images are pixel-wise equal";
        return true;
    }
    else if (static_cast<int>(numDiffAboveThreshold) <= maxErrorPixels) {
        report = "image difference below threshold: num diff=" + itos(numDiff) +
            ", num diff above tolerance=" + itos(numDiffAboveThreshold) +
            ", max color distance=" + ftos(maxDist);
        return true;
    }
    else {
        report = "image difference above threshold: num diff=" + itos(numDiff) +
            ", num diff above tolerance=" + itos(numDiffAboveThreshold) +
            ", max pixel diff=" + ftos(maxDist);
        return false;
    }

}

void ImageFileComparator::generateDiffFile(const RegressionTestDataset& refDataset, const RegressionTestDataset& outputDataset,
        RegressionTestDataset& diffDataset, const RegressionTestCase& testCase) const throw (VoreenException)
{

    // check parameters
    if (refDataset.files_.size() != outputDataset.files_.size())
        throw VoreenException("file count of reference and output datasets differs");
    if (refDataset.files_.size() != 1)
        throw VoreenException("dataset consists of multiple files");
    if (!diffDataset.files_.empty())
        throw VoreenException("diffDataset already contains files");

    std::string refFile = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refDataset.files_.at(0).filename_);
    std::string outputFile = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputDataset.files_.at(0).filename_);

    // load ref image
    ILuint refImage;
    try {
        refImage = loadImage(refFile);
    }
    catch (VoreenException& e) {
        throw VoreenException("Failed to load reference image '" + refFile + "': " + e.what());
    }

    // load output image
    ILuint outputImage;
    try {
        outputImage = loadImage(outputFile);
    }
    catch (VoreenException& e) {
        ilDeleteImage(refImage);
        throw VoreenException("Failed to load output image '" + outputFile + "': " + e.what());
    }

    // check image dimensions
    tgt::ivec2 refDim = getImageDimensions(refImage);
    tgt::ivec2 outputDim = getImageDimensions(outputImage);
    if (refDim != outputDim) {
        ilDeleteImage(refImage);
        ilDeleteImage(outputImage);
        throw VoreenException("Images differ in size: reference=" + genericToString(refDim) + ", output=" + genericToString(outputDim));
    }
    size_t numPixels = tgt::hmul(refDim);

    // extract pixel data as RGBA float
    tgt::vec4* refBuffer = new tgt::vec4[numPixels];
    ilBindImage(refImage);
    ilCopyPixels(0, 0, 0, refDim.x, refDim.y, 1, IL_RGBA, IL_FLOAT, refBuffer);

    tgt::vec4* outputBuffer = new tgt::vec4[numPixels];
    ilBindImage(outputImage);
    ilCopyPixels(0, 0, 0, refDim.x, refDim.y, 1, IL_RGBA, IL_FLOAT, outputBuffer);

    float pixelDiffTolerance = (testCase.config_.pixelDiffTolerance_ >= 0.f ? testCase.config_.pixelDiffTolerance_ : pixelDiffTolerance_);

    // create difference buffer (16 bit uint)
    tgt::Vector4<uint16_t>* differenceBuffer = new tgt::Vector4<uint16_t>[numPixels];
    for (size_t i=0; i<numPixels; i++) {
        tgt::vec4 diff = tgt::abs(refBuffer[i] - outputBuffer[i]);
        if (tgt::length(diff) / 2.f > pixelDiffTolerance) {
            diff.r = powf(diff.r, diffImageGamma_);
            diff.g = powf(diff.g, diffImageGamma_);
            diff.b = powf(diff.b, diffImageGamma_);
            if (diffImageFullAlpha_)
                diff.a = 1.f;
            else
                diff.a = powf(diff.a, diffImageGamma_);
        }
        else {
            diff = tgt::vec4(0.f);
        }
        tgt::Vector4<uint16_t> diffUInt16 = tgt::iround(diff * ((1 << 16) - 1.f));
        differenceBuffer[i] = diffUInt16;
    }

    // construct diff image path
    std::string extension = FileSystem::fileExtension(refFile);
    std::string diffFile = FileSystem::baseName(refFile) + ".diff";
    if (!extension.empty())
        diffFile += "." + extension;
    diffFile = FileSystem::cleanupPath(diffFile);
    std::string diffFileAbs = FileSystem::cleanupPath(testCase.outputDir_ + "/" + diffFile);

    // save difference buffer to file
    ILuint diffImage;
    ilGenImages(1, &diffImage);
    ilBindImage(diffImage);
    //ilSetPixels(0, 0, 0, refDim.x, refDim.y, 1, IL_RGBA, IL_FLOAT, differenceBuffer);
    ilTexImage(refDim.x, refDim.y, 1, 4, IL_RGBA, IL_UNSIGNED_SHORT, differenceBuffer);
    ilEnable(IL_FILE_OVERWRITE);
    //ilResetWrite();
    ILboolean success = ilSaveImage(const_cast<char*>(diffFileAbs.c_str()));
    std::string errorMsg;
    if (!success)
        errorMsg = DevILModule::getDevILError();

    // clear temp data
    delete[] refBuffer;
    delete[] outputBuffer;
    delete[] differenceBuffer;
    ilDeleteImage(refImage);
    ilDeleteImage(outputImage);
    ilDeleteImage(diffImage);

    if (success) {
        RegressionTestFile file;
        file.filename_ = diffFile;
        file.fileType_ = ImageFile;
        diffDataset.files_.push_back(file);
    }
    else
        throw VoreenException("Failed to save difference image '" + diffFileAbs + "':" + errorMsg);
}

tgt::ivec2 ImageFileComparator::getImageDimensions(unsigned int image) {
    tgt::ivec2 result;
    ilBindImage(static_cast<GLuint>(image));
    result.x = ilGetInteger(IL_IMAGE_WIDTH);
    result.y = ilGetInteger(IL_IMAGE_HEIGHT);
    return result;
}

tgt::ivec2 ImageFileComparator::getImageDimensions(const std::string& filename) {
    try {
        unsigned int image = loadImage(filename);
        tgt::ivec2 dims = getImageDimensions(image);
        ilDeleteImage(image);
        return dims;
    }
    catch (VoreenException& e) {
        LWARNINGC("regressiontest.ImageFileComparator", "Failed to determine image dimensions: " << e.what());
        return tgt::ivec2(0);
    }
}

unsigned int ImageFileComparator::loadImage(const std::string& filename) throw (VoreenException) {
    if (!FileSystem::fileExists(filename))
        throw VoreenException("Image file '" + filename + "' not found");

    ILuint image;
    ilGenImages(1, &image);
    ilBindImage(image);
    if (!ilLoadImage(filename.c_str())) {
        std::string errorMsg = DevILModule::getDevILError();
        ilDeleteImage(image);
        throw VoreenException(errorMsg);
    }

    return static_cast<unsigned int>(image);
}

ILint ImageFileComparator::getImageFormat(unsigned int image) {
    ilBindImage(static_cast<GLuint>(image));
    return ilGetInteger(IL_IMAGE_FORMAT);
}

ILint ImageFileComparator::getDataType(unsigned int image) {
    ilBindImage(static_cast<GLuint>(image));
    return ilGetInteger(IL_IMAGE_TYPE);
}

#endif // VRN_MODULE_DEVIL

//----------------------------------------------------------------------------------------

const std::string VvdFileComparator::loggerCat_("regressiontest.VvdFileComparator");

VvdFileComparator::VvdFileComparator(float voxelDiffTolerance, int maxErrorVoxels)
    : voxelDiffTolerance_(voxelDiffTolerance)
    , maxErrorVoxels_(maxErrorVoxels)
{}

bool VvdFileComparator::supportsFormat(const RegressionTestDataset& dataset) const {
    // expected: vvd+raw file
    if (dataset.files_.size() != 2)
        return false;
    else {
        std::string extensionA = FileSystem::fileExtension(dataset.files_.at(0).filename_);
        std::string extensionB = FileSystem::fileExtension(dataset.files_.at(1).filename_);
        return ((extensionA == "vvd" && extensionB == "raw") || (extensionB == "vvd" && extensionA == "raw"));
    }
}

void VvdFileComparator::determineFileType(RegressionTestDataset& dataset) const {
    if (supportsFormat(dataset)) {
        if (FileSystem::fileExtension(dataset.files_.at(0).filename_) == "vvd")
            dataset.files_.at(0).fileType_ = TextFile;
        else if (FileSystem::fileExtension(dataset.files_.at(0).filename_) == "raw")
            dataset.files_.at(0).fileType_ = BinaryFile;

        if (FileSystem::fileExtension(dataset.files_.at(1).filename_) == "vvd")
            dataset.files_.at(1).fileType_ = TextFile;
        else if (FileSystem::fileExtension(dataset.files_.at(1).filename_) == "raw")
            dataset.files_.at(1).fileType_ = BinaryFile;
    }
    else
        for (size_t i=0; i<dataset.files_.size(); i++)
            dataset.files_.at(i).fileType_ = FileTypeUnknown;
}

bool VvdFileComparator::compare(RegressionTestDataset& refDataset, RegressionTestDataset& outputDataset,
    std::string& report, const RegressionTestCase& testCase) const
{

    // check parameters
    if (refDataset.files_.size() != outputDataset.files_.size()) {
        tgtAssert(false, "file count of reference and output datasets differs (should have been checked by caller)");
        report = "file count of reference and output datasets differs";
        return false;
    }
    if (refDataset.files_.size() != 2) {
        tgtAssert(false, "dataset does not consist of two files (should have been rejected by supportsFormat()");
        report = "VvdComparator: dataset does not consist of two files";
        return false;
    }

    // run binary comparison first (faster)
    if (binaryComparator_.compare(refDataset, outputDataset, report, testCase))
        return true;

    //
    // if binary comparison failed, compare volumes voxel-wise
    //

    // absolute path to .vvd file of reference dataset
    std::string refVvdPath;
    if (FileSystem::fileExtension(refDataset.files_.at(0).filename_) == "vvd") {
        refVvdPath = refDataset.files_.at(0).filename_;
        refDataset.files_.at(0).fileType_ = TextFile;
        refDataset.files_.at(1).fileType_ = BinaryFile;
    }
    else if (FileSystem::fileExtension(refDataset.files_.at(1).filename_) == "vvd") {
        refVvdPath = refDataset.files_.at(1).filename_;
        refDataset.files_.at(0).fileType_ = BinaryFile;
        refDataset.files_.at(1).fileType_ = TextFile;
    }
    else {
        report = "reference dataset does not contain .vvd file";
        refDataset.files_.at(0).fileType_ = FileTypeUnknown;
        refDataset.files_.at(1).fileType_ = FileTypeUnknown;
        return false;
    }
    refVvdPath = FileSystem::cleanupPath(testCase.referenceDir_ + "/" + refVvdPath);

    // absolute path to .vvd file of output dataset
    std::string outputVvdPath;
    if (FileSystem::fileExtension(outputDataset.files_.at(0).filename_) == "vvd") {
        outputVvdPath = outputDataset.files_.at(0).filename_;
        outputDataset.files_.at(0).fileType_ = TextFile;
        outputDataset.files_.at(1).fileType_ = BinaryFile;
    }
    else if (FileSystem::fileExtension(outputDataset.files_.at(1).filename_) == "vvd") {
        outputVvdPath = outputDataset.files_.at(1).filename_;
        outputDataset.files_.at(0).fileType_ = BinaryFile;
        outputDataset.files_.at(1).fileType_ = TextFile;
    }
    else {
        report = "output dataset does not contain .vvd file";
        outputDataset.files_.at(0).fileType_ = FileTypeUnknown;
        outputDataset.files_.at(1).fileType_ = FileTypeUnknown;
        return false;
    }
    outputVvdPath = FileSystem::cleanupPath(testCase.outputDir_ + "/" + outputVvdPath);

    // read ref dataset
    VvdVolumeReader vvdReader;
    VolumeBase* refHandle = 0;
    try {
        LDEBUG("Reading reference volume: " << refVvdPath);
        VolumeList* refCollection = vvdReader.read(refVvdPath);
        tgtAssert(refCollection, "vvd reader returned null pointer");
        tgtAssert(!refCollection->empty(), "vvd reader returned empty collection");
        refHandle = refCollection->first();
        delete refCollection;
    }
    catch (std::exception& e) {
        report = "failed to read reference volume '" + refVvdPath + "': " + e.what();
        return false;
    }
    tgtAssert(refHandle, "refHandle is null");

    // read output dataset
    VolumeBase* outputHandle = 0;
    try {
        LDEBUG("Reading output volume: " << outputVvdPath);
        VolumeList* outputCollection = vvdReader.read(outputVvdPath);
        tgtAssert(outputCollection, "vvd reader returned null pointer");
        tgtAssert(!outputCollection->empty(), "vvd reader returned empty collection");
        outputHandle = outputCollection->first();
        delete outputCollection;
    }
    catch (std::exception& e) {
        delete refHandle;
        report = "failed to read output volume '" + outputVvdPath + "': " + e.what();
        return false;
    }
    tgtAssert(outputHandle, "outputHandle is null");

    // retrieve reference volume from handle
    const VolumeRAM* refVolume = 0;
    try {
        refVolume = refHandle->getRepresentation<VolumeRAM>();
    }
    catch (std::exception& e) {
        delete refHandle;
        delete outputHandle;
        report = "failed to read reference volume '" + refVvdPath + "': " + e.what();
        return false;
    }
    tgtAssert(refVolume, "ref volume is null");

    // retrieve output volume from handle
    const VolumeRAM* outputVolume = 0;
    try {
        outputVolume = outputHandle->getRepresentation<VolumeRAM>();
    }
    catch (std::exception& e) {
        delete refHandle;
        delete outputHandle;
        report = "failed to read output volume '" + outputVvdPath + "': " + e.what();
        return false;
    }
    tgtAssert(outputVolume, "output volume is null");

    //
    // check meta-data of volumes
    //

    // take thresholds from test case config if valid, member values otherwise
    const float voxelDiffTolerance = (testCase.config_.voxelDiffTolerance_ >= 0.f ? testCase.config_.voxelDiffTolerance_ : voxelDiffTolerance_);
    const int maxErrorVoxels = (testCase.config_.maxErrorVoxels_ >= 0 ? testCase.config_.maxErrorVoxels_ : maxErrorVoxels_);

    VolumeFactory volumeFactory;
    bool propertiesMatch = true;
    if (refHandle->getDimensions() != outputHandle->getDimensions()) {
        report = "volume dimensions do not match";
        propertiesMatch = false;
    }
    if (volumeFactory.getFormat(refVolume) != volumeFactory.getFormat(outputVolume)) {
        report = "type of output volume (" + volumeFactory.getFormat(outputVolume) + ") "
            "does not match type of reference volume (" + volumeFactory.getFormat(refVolume) + ")";
        propertiesMatch = false;
    }
    std::set<std::string> metaDataKeys;
    std::vector<std::string> refKeys = refHandle->getMetaDataKeys();
    std::vector<std::string> outputKeys = outputHandle->getMetaDataKeys();
    metaDataKeys.insert(refKeys.begin(), refKeys.end());
    metaDataKeys.insert(outputKeys.begin(), outputKeys.end());
    for (std::set<std::string>::iterator it = metaDataKeys.begin(); it != metaDataKeys.end() && propertiesMatch; ++it) {
        std::string key = *it;
        const MetaDataBase* refMetaDate = refHandle->getMetaData(key);
        const MetaDataBase* outputMetaDate = outputHandle->getMetaData(key);
        if (!refMetaDate) {
            report = "MetaData item '" + key + "' present in output volume, but not in reference volume";
            propertiesMatch = false;
        }
        else if (!outputMetaDate) {
            report = "MetaData item '" + key + "' present in reference volume, but not in output volume";
            propertiesMatch = false;
        }
        else {
            bool match = compareMetaData(refMetaDate, outputMetaDate, voxelDiffTolerance);
            if (!match) {
                report = "MetaData item '" + key + "' does not match";
                propertiesMatch = false;
            }
        }
    }

    if (!propertiesMatch) {
        delete refHandle;
        delete outputHandle;
        return false;
    }


    //
    // perform voxel-wise comparison
    //

    // at this point, volumes must of the same type
    tgtAssert(refHandle->getNumChannels() == outputHandle->getNumChannels(), "volume types differ");
    tgtAssert(refHandle->getBytesPerVoxel() == outputHandle->getBytesPerVoxel(), "volume types differ");

    size_t numDiffVoxels = 0;
    size_t numDiffVoxelsAboveThreshold = 0;
    float maxError = 0.f;

    if (dynamic_cast<const VolumeRAM_UInt8*>(refVolume)) {
        // optimized comparison for uint8 volumes
        LDEBUG("Running optimized comparison for uint8 volumes");
        const VolumeRAM_UInt8* refVolumeRAM_UInt8 = static_cast<const VolumeRAM_UInt8*>(refVolume);
        const VolumeRAM_UInt8* outputVolumeRAM_UInt8 = dynamic_cast<const VolumeRAM_UInt8*>(outputVolume);
        size_t numVoxels = refVolumeRAM_UInt8->getNumVoxels();
        tgtAssert(outputVolumeRAM_UInt8, "output vol type differs from ref vol type"); // must have been detected before
        tgtAssert(outputVolumeRAM_UInt8->getNumVoxels() == numVoxels, "output vol dim differes from ref vol dim");

        for (size_t i=0; i<numVoxels; i++) {
            uint8_t refVol = refVolumeRAM_UInt8->voxel(i);
            uint8_t outputVol = outputVolumeRAM_UInt8->voxel(i);
            if (refVol != outputVol) {
                numDiffVoxels++;
                float diff = std::abs(static_cast<int>(refVol) - static_cast<int>(outputVol)) / 255.f;
                maxError = std::max(diff, maxError);
                if (diff > voxelDiffTolerance)
                    numDiffVoxelsAboveThreshold++;
            }
        }
    }
    else if (dynamic_cast<const VolumeRAM_UInt16*>(refVolume)) {
        // optimized comparison for uint16 volumes
        LDEBUG("Running optimized comparison for uint16 volumes");
        const VolumeRAM_UInt16* refVolumeRAM_UInt16 = static_cast<const VolumeRAM_UInt16*>(refVolume);
        const VolumeRAM_UInt16* outputVolumeRAM_UInt16 = dynamic_cast<const VolumeRAM_UInt16*>(outputVolume);
        size_t numVoxels = refVolumeRAM_UInt16->getNumVoxels();
        tgtAssert(outputVolumeRAM_UInt16, "output vol type differs from ref vol type"); // must have been detected before
        tgtAssert(outputVolumeRAM_UInt16->getNumVoxels() == numVoxels, "output vol dim differs from ref vol dim");

        for (size_t i=0; i<numVoxels; i++) {
            uint16_t refVol = refVolumeRAM_UInt16->voxel(i);
            uint16_t outputVol = outputVolumeRAM_UInt16->voxel(i);
            if (refVol != outputVol) {
                numDiffVoxels++;
                float diff = std::abs(static_cast<int>(refVol) - static_cast<int>(outputVol)) / 65535.f;
                maxError = std::max(diff, maxError);
                if (diff > voxelDiffTolerance)
                    numDiffVoxelsAboveThreshold++;
            }
        }
    }
    else {
        // generic comparison for remaining volume types
        LDEBUG("Running generic volume comparison");
        size_t numVoxels = refVolume->getNumVoxels();
        size_t numChannels = refVolume->getNumChannels();
        tgtAssert(outputVolume->getNumChannels() == numChannels, "output vol channel count differs from ref channel count");

        if (numChannels == 1) {
            for (size_t i=0; i<numVoxels; i++) {
                float refVol = refVolume->getVoxelNormalized(i);
                float outputVol = outputVolume->getVoxelNormalized(i);
                if (refVol != outputVol) {
                    numDiffVoxels++;
                    float diff = std::abs(refVol - outputVol);
                    maxError = std::max(diff, maxError);
                    if (diff > voxelDiffTolerance)
                        numDiffVoxelsAboveThreshold++;
                }
            }
        }
        else if (numChannels == 2) {
            for (size_t i=0; i<numVoxels; i++) {
                tgt::vec2 refVol;
                refVol.x = refVolume->getVoxelNormalized(i, 0);
                refVol.y = refVolume->getVoxelNormalized(i, 1);
                tgt::vec2 outputVol;
                outputVol.x = outputVolume->getVoxelNormalized(i, 0);
                outputVol.y = outputVolume->getVoxelNormalized(i, 1);
                if (refVol != outputVol) {
                    numDiffVoxels++;
                    float diff = tgt::length(refVol - outputVol) / sqrt(2.f); //< normalize to [0.0;1.0]
                    maxError = std::max(diff, maxError);
                    if (diff > voxelDiffTolerance)
                        numDiffVoxelsAboveThreshold++;
                }
            }
        }
        else if (numChannels == 3) {
            for (size_t i=0; i<numVoxels; i++) {
                tgt::vec3 refVol;
                refVol.x = refVolume->getVoxelNormalized(i, 0);
                refVol.y = refVolume->getVoxelNormalized(i, 1);
                refVol.z = refVolume->getVoxelNormalized(i, 2);
                tgt::vec3 outputVol;
                outputVol.x = outputVolume->getVoxelNormalized(i, 0);
                outputVol.y = outputVolume->getVoxelNormalized(i, 1);
                outputVol.z = outputVolume->getVoxelNormalized(i, 2);
                if (refVol != outputVol) {
                    numDiffVoxels++;
                    float diff = tgt::length(refVol - outputVol) / sqrt(3.f); //< normalize to [0.0;1.0]
                    maxError = std::max(diff, maxError);
                    if (diff > voxelDiffTolerance)
                        numDiffVoxelsAboveThreshold++;
                }
            }
        }
        else if (numChannels == 4) {
            for (size_t i=0; i<numVoxels; i++) {
                tgt::vec4 refVol;
                refVol.x = refVolume->getVoxelNormalized(i, 0);
                refVol.y = refVolume->getVoxelNormalized(i, 1);
                refVol.z = refVolume->getVoxelNormalized(i, 2);
                refVol.w = refVolume->getVoxelNormalized(i, 3);
                tgt::vec4 outputVol;
                outputVol.x = outputVolume->getVoxelNormalized(i, 0);
                outputVol.y = outputVolume->getVoxelNormalized(i, 1);
                outputVol.z = outputVolume->getVoxelNormalized(i, 2);
                outputVol.w = outputVolume->getVoxelNormalized(i, 3);
                if (refVol != outputVol) {
                    numDiffVoxels++;
                    float diff = tgt::length(refVol - outputVol) / 2.f; //< normalize to [0.0;1.0]
                    maxError = std::max(diff, maxError);
                    if (diff > voxelDiffTolerance)
                        numDiffVoxelsAboveThreshold++;
                }
            }
        }
        else {
            LWARNING("unsupported channel count: " << numChannels);
            report = "unsupported channel count: " + itos(numChannels);
            delete refHandle;
            delete outputHandle;
            return false;
        }
    }

    // clean up
    delete refHandle;
    delete outputHandle;

    // analyze results
    if (numDiffVoxels == 0) {
        report = "volumes are voxel-wise equal";
        return true;
    }
    else if (static_cast<int>(numDiffVoxelsAboveThreshold) <= maxErrorVoxels) {
        report = "volume difference below threshold: num diff=" + itos(numDiffVoxels) +
            ", num diff above tolerance=" + itos(numDiffVoxelsAboveThreshold) +
            ", max voxel distance=" + ftos(maxError);
        return true;
    }
    else {
        report = "volume difference above threshold: num diff=" + itos(numDiffVoxels) +
            ", num diff above tolerance=" + itos(numDiffVoxelsAboveThreshold) +
            ", max voxel distance=" + ftos(maxError);
        return false;
    }

}

void VvdFileComparator::generateDiffFile(const RegressionTestDataset& /*refDataset*/, const RegressionTestDataset& /*outputDataset*/,
    RegressionTestDataset& /*diffDataset*/, const RegressionTestCase& /*testCase*/)
    const throw (VoreenException)
{
    tgtAssert(false, "Generation of volume diffs not supported");
    throw VoreenException("Generation of volume diffs not supported");
}

} // namespace
