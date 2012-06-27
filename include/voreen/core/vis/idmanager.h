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

#ifndef VRN_IDMANAGER_H
#define VRN_IDMANAGER_H

#include "tgt/vector.h"
#include "voreen/core/vis/message.h"

namespace voreen {

class TextureContainer;

/**
 * Data Structure - enhanced Identifier
 */
class IDF {
public:
    IDF(std::string name);
    friend bool operator==(const IDF &x, const IDF &y);
    friend bool operator==(const std::string &x, const IDF &y);
    friend bool operator==(const IDF &x, const std::string &y);
    tgt::vec3 vec() const { return tgt::vec3(x_, y_, z_); }

    float x_;
    float y_;
    float z_;
    std::string name_;
};

/**
 * Construct an IDF with a std::string
 */
inline IDF::IDF(std::string name) : name_(name) {}

/**
 * Compares two IDF's
 */
inline bool operator==(const IDF &x, const IDF &y) {
    return x.name_.compare(y.name_) == 0;
}

/**
 * Compares std:string with IDF
 */
inline bool operator==(const std::string &x, const IDF &y) {
    return x.compare(y.name_) == 0;
}

/**
 * Compares IDF with std::string
 */
inline bool operator==(const IDF &x, const std::string &y) {
    return x.name_.compare(y) == 0;
}


/**
 * IDManager class for picking
 * usage:
 * Processor: addNewPickObj() - startBufferRenderering() - stopBufferRendering()
 * Validation: isClicked()
 */
class IDManager {

    class IDManagerContents {
    public:
        IDManagerContents();
        int currentID_R_;
        int currentID_G_;
        int currentID_B_;
        int textureTarget_;
        int oldRT_;
        bool isTC_;
        bool newRenderingPass_;
        TextureContainer* tc_;
        std::vector<IDF> picks_;
    };

public:
    IDManager();

    static IDManagerContents* getContent() {return content_;}

    /**
     * adds new picking object to the manager
     * @param ident identifies the picking object
     */
    void addNewPickObj(Identifier ident);

    /**
     * clears buffer (picking texture)
     */
    void clearTextureTarget();

    /**
     * Tells the IDManager that a new rendering pass has begun. The content of the
     * textureTarget will be deleted when startBufferRendering is called for the first time
     * in a rendering pass.
     */
    void signalizeNewRenderingPass();

    /**
     * Sets rendering target to picking buffer
     * Sets the correct color for picking object
     */
    void startBufferRendering(Identifier identIN);

    /**
     * Resets rendering target
     */
    void stopBufferRendering();

    /**
     * Validates picking objects position
     */
    bool isClicked(Identifier ident, int x, int y);

    /**
     * Resets saved picking objects
     */
    void clearIDs();

    /**
     * Sets texture container
     */
    void setTC(TextureContainer* tc);

    /**
     * Returns color at position (x,y) in the picking texture
     */
    tgt::vec3 getIDatPos(int x, int y) const;

    /** Returns the name of the picked object at position (x, y).
     *
     * NOTE: in contrast to methods <code>getIDadPos()</code> and
     * <code>isClicked()</code>, this methods flips the y-coordinate!
     * The caller has to pass the y-component in screen coordinates
     * like it is obtained for example by Qt mousePressEvent().
     *
     */
    std::string getNameAtPos(int x, int y) const;

private:
    static IDManagerContents* content_;
};


} //namespace voreen

#endif
