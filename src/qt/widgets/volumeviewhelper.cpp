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

#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/bricking/brickedvolume.h"

#include <QDir>

namespace voreen {

std::string VolumeViewHelper::getVolumeType(Volume* volume) {
    if (dynamic_cast<VolumeUInt8*>(volume)!=0) return "uint8";
    if (dynamic_cast<VolumeUInt16*>(volume)!=0) return "uint16";
    if (dynamic_cast<VolumeUInt32*>(volume)!=0) return "uint32";
    if (dynamic_cast<VolumeInt8*>(volume)!=0) return "int8";
    if (dynamic_cast<VolumeInt16*>(volume)!=0) return "int16";
    if (dynamic_cast<VolumeInt32*>(volume)!=0) return "int32";
    if (dynamic_cast<VolumeFloat*>(volume)!=0) return "float";
    if (dynamic_cast<VolumeDouble*>(volume)!=0) return "double";
    if (dynamic_cast<Volume4xUInt8*>(volume)!=0) return "4 x uint8";
    if (dynamic_cast<Volume4xUInt16*>(volume)!=0) return "4 x uint16";
    if (dynamic_cast<Volume4xInt8*>(volume)!=0) return "4 x int8";
    if (dynamic_cast<Volume4xInt16*>(volume)!=0) return "4 x int16";
    if (dynamic_cast<Volume3xUInt8*>(volume)!=0) return "3 x uint8";
    if (dynamic_cast<Volume3xUInt16*>(volume)!=0) return "3 x uint16";
    if (dynamic_cast<Volume3xInt8*>(volume)!=0) return "3 x int8";
    if (dynamic_cast<Volume3xInt16*>(volume)!=0) return "3 x int16";
    if (dynamic_cast<Volume3xFloat*>(volume)!=0) return "3 x float";
    if (dynamic_cast<Volume3xDouble*>(volume)!=0) return "3 x double";
    if (dynamic_cast<Volume4xFloat*>(volume)!=0) return "4 x float";
    if (dynamic_cast<Volume4xDouble*>(volume)!=0) return "4 x double";
    if (dynamic_cast<BrickedVolume*>(volume)!=0) {
        std::stringstream out;
        out << getVolumeType(dynamic_cast<BrickedVolume*>(volume)->getPackedVolume()) << " bricked";
        return  out.str();
    }
    return "";
}

QPixmap VolumeViewHelper::generatePreview(Volume* volume, int height) {
    return generateBorderedPreview(volume, height, 0);
}

QPixmap VolumeViewHelper::generateBorderedPreview(Volume* volume, int height, int border) {
    int xDimension = volume->getDimensions()[0];
    int yDimension = volume->getDimensions()[1];
    int zDimension = volume->getDimensions()[2]/2;
    float step;
    if (yDimension > xDimension) {
        step = (float)yDimension/height;
    }
    else {
        step = (float)xDimension/height;
    }
    QImage* preview = new QImage(height , height, QImage::Format_ARGB32);
    QRgb pixelValue;
    int greyInt;

    for(float y = .0f; y < height; y++){
        for(float x = .0f; x < height; x++){
            tgt::vec3 position;
            position.x = x*step;
            position.y = y*step;
            position.z = zDimension;
            if(position.x < xDimension && position.y < yDimension) {
                greyInt = static_cast<int>(255.f * volume->getVoxelFloatLinear(position));
                pixelValue = qRgb(greyInt, greyInt, greyInt);
            }
            else {
                pixelValue = qRgb(0, 0, 0);
            }
            preview->setPixel(static_cast<int>(x), static_cast<int>(y), pixelValue);
        }
    }
    QImage previewScaled = preview->scaledToHeight(height);

    //histogram equalization
    uint minGrey = previewScaled.pixel(0, 0);
    uint maxGrey = previewScaled.pixel(0, 0);
    for(int y = 0; y < previewScaled.height(); y++){
           for(int x = 0; x < previewScaled.width(); x++){
                   if(previewScaled.pixel(x, y) < minGrey) minGrey = previewScaled.pixel(x, y);
                   if(previewScaled.pixel(x, y) > maxGrey) maxGrey = previewScaled.pixel(x, y);
           }
    }
    for(int y = 0; y < previewScaled.height(); y++){
           for(int x = 0; x < previewScaled.width(); x++){
               if (maxGrey == minGrey)
                   greyInt = 0;
               else
                   greyInt = (previewScaled.pixel(x, y) - minGrey) * 255/(maxGrey - minGrey);
                   pixelValue = qRgb(greyInt, greyInt, greyInt);
                   previewScaled.setPixel(x, y, pixelValue);
           }
    }
    //draw border
    for(int y = 0; y < previewScaled.height(); y++){
        for(int x =0; x < border ; x++){
               previewScaled.setPixel(previewScaled.width()-x-1, y, qRgb(255, 255, 255));
        }
    }
    for(int x = 0; x < previewScaled.width(); x++){
        for(int y = 0; y < border ; y++){
               previewScaled.setPixel(x, previewScaled.height()-y-1, qRgb(255, 255, 255));
        }
    }

    delete preview;
    return QPixmap::fromImage(previewScaled);

}

std::string VolumeViewHelper::volumeInfoString(VolumeHandle* handle) {
    std::string outString;
    if (handle && handle->getVolume()) {

        Volume* volume = handle->getVolume();

        std::string filename = getStrippedVolumeName(handle);
        if(filename.length() > 13) {
            QString ext = QString::fromStdString(filename);
            filename.resize(13);
            filename+="...";
            filename+=ext.section('.',-1).toStdString();
        }
        std::string spacing = getVolumeSpacing(volume);
        std::string dimension = getVolumeDimension(volume);
        std::string type = getVolumeType(volume);
        outString+="<style type='text/css'> table {margin-left:0px; margin-top:1px; font-size: 9px;}</style><table><tr><td>"+filename+"</td><td></td></tr><tr><td>type </td><td>" \
            +type+"</td></tr><tr><td>dimension </td><td>" \
            +dimension+"</td></tr><tr><td>spacing </td><td>" \
            +spacing+"</td></tr><tr><td>memSize </td><td>" \
    +getVolumeMemorySize(volume)+" bytes</td></tr> </table>";

    }
    else {
        outString = "<strong>Volume:</strong> no Volume";
    }
    return outString;
}

std::string VolumeViewHelper::getVolumeName(VolumeHandle* handle){
    std::string volumeName;
    if (handle) {
        volumeName = handle->getOrigin().getPath();
    }
    else {
        volumeName = "no Volume";
    }
    return volumeName;
}

std::string VolumeViewHelper::getStrippedVolumeName(VolumeHandle* handle) {
    if (!handle)
        return "no volume";
    else {
        std::string volumePath = getVolumeName(handle);

        //  remove trailing slash (dicom)
        std::string::size_type separator = volumePath.find_last_of("/\\");
        if (separator == volumePath.size()-1)
            volumePath = volumePath.substr(0, volumePath.size()-1);

        return tgt::FileSystem::fileName(volumePath);
    }
}

std::string VolumeViewHelper::getVolumePath(VolumeHandle* handle) {
    std::string volumeName;
    if (handle) {
        QString fn = QString::fromStdString(handle->getOrigin().getPath());
        volumeName = fn.section('/',0, -2).toStdString();
    }
    else {
        volumeName = "no Volume";
    }
    return volumeName;

}

std::string VolumeViewHelper::getVolumeDimension(Volume* volume) {
    std::stringstream out;

    BrickedVolume* brick = dynamic_cast<BrickedVolume*>(volume);
    if (brick) {
        out << getVolumeDimension(brick->getEepVolume()) << " (using "
            << getVolumeDimension(brick->getPackedVolume()) << ")";
    } else {
        out << volume->getDimensions()[0] << " x " << volume->getDimensions()[1] << " x " << volume->getDimensions()[2];
    }

    return out.str();
}

std::string VolumeViewHelper::getVolumeSpacing(Volume* volume) {
    std::stringstream out;
    out << volume->getSpacing()[0] << " x " << volume->getSpacing()[1] << " x " << volume->getSpacing()[2];
    return out.str();
}

std::string VolumeViewHelper::getVolumeMemorySize(Volume* volume) {
    std::stringstream out;

    BrickedVolume* brick = dynamic_cast<BrickedVolume*>(volume);
    if (brick) {
        out << getVolumeMemorySize(brick->getEepVolume()) << " (using "
            << getVolumeMemorySize(brick->getPackedVolume()) << ")";
        return out.str();
    }

    long bytes = volume->getNumBytes();
    float mb = tgt::round(bytes/104857.6f) / 10.f;    //calculate mb with 0.1f precision
    float kb = tgt::round(bytes/102.4f) / 10.f;
    if (mb >= 0.5f) {
        out << mb << " MB";
    }
    else if (kb >= 0.5f) {
        out << kb << " kB";
    }
    else {
        out << bytes << " bytes";
    }
    return out.str();
}

} // namespace
