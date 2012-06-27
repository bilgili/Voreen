/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_ROIBASE_H
#define VRN_ROIBASE_H

#include "tgt/bounds.h"
#include "tgt/plane.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/geometry/facegeometry.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/matrixproperty.h"

namespace voreen {

class ROIBase;

class ControlPoint {
public:
    enum Type {
        CENTER = 0,
        X_PLUS = 1,
        X_MINUS = 2,
        Y_PLUS = 3,
        Y_MINUS = 4,
        Z_PLUS = 5,
        Z_MINUS = 6
    };

    ControlPoint(const ROIBase* roi, int id, tgt::vec3 pos) : roi_(roi), id_(id), pos_(pos) {
    }

    tgt::vec3 getPosition() const { return pos_; }
    const ROIBase* getROI() const { return roi_; }
    int getID() const { return id_; }
    virtual ControlPoint* clone() const { return new ControlPoint(roi_, id_, pos_); }
protected:
    const ROIBase* roi_;
    int id_;
    tgt::vec3 pos_;
};

class Grid : public Serializable {
public:
    Grid();
    Grid(tgt::vec3 origin, tgt::vec3 spacing, tgt::vec3 xVec = tgt::vec3(1.0f, 0.0f, 0.0f), tgt::vec3 yVec = tgt::vec3(0.0f, 1.0f, 0.0f), tgt::vec3 zVec = tgt::vec3(0.0f, 0.0f, 1.0f));

    tgt::mat4 getPhysicalToWorldMatrix() const;
    tgt::mat4 getWorldToPhysicalMatrix() const;

    tgt::vec3 getSpacing() const { return spacing_; }
    tgt::vec3 getOrigin() const { return origin_; }

    tgt::vec3 getXVector() const { return xVec_; }
    tgt::vec3 getYVector() const { return yVec_; }
    tgt::vec3 getZVector() const { return zVec_; }

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer& s);

    inline float difference(const Grid& b) const
    {
        float maxDiff = 0.0f;
        maxDiff = std::max(maxDiff, max(tgt::abs(origin_ - b.origin_)));
        maxDiff = std::max(maxDiff, max(tgt::abs(xVec_ - b.xVec_)));
        maxDiff = std::max(maxDiff, max(tgt::abs(yVec_ - b.yVec_)));
        maxDiff = std::max(maxDiff, max(tgt::abs(zVec_ - b.zVec_)));
        maxDiff = std::max(maxDiff, max(tgt::abs(spacing_ - b.spacing_)));
        return maxDiff;
    }

    inline bool operator==(const Grid& b) const
    {
        return (origin_ == b.origin_ &&
                xVec_ == b.xVec_ &&
                yVec_ == b.yVec_ &&
                zVec_ == b.zVec_ &&
                spacing_ == b.spacing_);
    }
    inline bool operator!=(const Grid& b) const
    {
        return !operator==(b);
    }
private:
    tgt::vec3 origin_;

    tgt::vec3 xVec_;
    tgt::vec3 yVec_;
    tgt::vec3 zVec_;

    tgt::vec3 spacing_;
};

Grid gridFromVolume(const VolumeBase* vol);

class MeshListGeometry;

class ROIMeshCache {
    struct CacheEntry {
        tgt::plane plane_;
        MeshListGeometry* mesh_;
        CacheEntry(tgt::plane pl, MeshListGeometry* m) : plane_(pl), mesh_(m) {}
    };
public:
    ROIMeshCache();
    ~ROIMeshCache();

    MeshListGeometry* getMesh() const;
    void setMesh(MeshListGeometry* m);

    MeshListGeometry* getRasterMesh() const;
    void setRasterMesh(MeshListGeometry* m);

    MeshListGeometry* getMesh(tgt::plane pl) const;
    void setMesh(MeshListGeometry* m, tgt::plane pl);

    MeshListGeometry* getRasterMesh(tgt::plane pl) const;
    void setRasterMesh(MeshListGeometry* m, tgt::plane pl);

    void clear();
private:
    MeshListGeometry* mesh3D_;
    MeshListGeometry* rasterMesh3D_;
    mutable std::list<CacheEntry> planeMeshes_;
    mutable std::list<CacheEntry> planeRasterMeshes_;
};

class VRN_CORE_API ROIBase : public PropertyOwner {
    public:
        enum InvalidationLevel {
            VALID = 0,
            METADATA_CHANGE = 1,        ///< Just a change of name, comment, etc...
            ROI_CHANGE = 10,            ///<  ROI has actually changed, remove statistices
        };
    ROIBase(Grid grid);
    ROIBase();
    virtual std::string getClassName() const = 0;
    virtual ROIBase* clone() const;
    virtual ROIBase* create() const = 0;

    virtual void invalidate(int inv = ROI_CHANGE);

    virtual std::vector<ROIBase*> getChildren() = 0;
    virtual std::vector<const ROIBase*> getChildren() const = 0;

    /// Test if p (in physical coordinates of the ROI) is inside this ROI.
    virtual bool inROI(tgt::vec3 p) const = 0;

    MeshListGeometry* getMesh() const;
    MeshListGeometry* getRasterMesh() const;
    virtual MeshListGeometry* generateMesh() const = 0;
    virtual MeshListGeometry* generateRasterMesh() const;
    virtual MeshListGeometry* generateRasterMesh(Grid g) const;

    MeshListGeometry* getMesh(tgt::plane pl) const;
    MeshListGeometry* getRasterMesh(tgt::plane pl) const;
    virtual MeshListGeometry* generateMesh(tgt::plane pl) const = 0;
    virtual MeshListGeometry* generateRasterMesh(const tgt::plane& pl) const;
    virtual MeshListGeometry* generateRasterMesh(const tgt::plane& pl, Grid g) const;

    /// Rasterize in a given grid.
    virtual VolumeRAM* rasterize(Grid g) const; // TODO: implement, returns 0 //TODO: cropped?
    /// Rasterize on own grid.
    virtual VolumeRAM* rasterize() const;

    /// Get bounding box (in physical coordinates)
    virtual tgt::Bounds getBoundingBox() const = 0;

    virtual std::vector<const ControlPoint*> getControlPoints() const = 0;
    virtual std::vector<const ControlPoint*> getControlPoints(tgt::plane pl) const = 0;

    virtual bool moveControlPoint(const ControlPoint* /*cp*/, tgt::vec3 /*offset*/) { return false; }

    virtual Grid getGrid() const;

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer& s);

    // Property getter/setter:
    std::string getName() const { return name_.get(); }
    void setName(const std::string& name) { return name_.set(name); }
    std::string getComment() const { return comment_.get(); }
    void setComment(const std::string& comment) { return comment_.set(comment); }

    bool isVisible() const { return isVisible_.get(); }
    void setVisible(bool visible) { isVisible_.set(visible); }
    bool isSelected() const { return isSelected_.get(); }
    void setSelected(bool selected) { isSelected_.set(selected); }
    bool snapsToGrid() const { return snapToGrid_.get(); }
    void setSnapToGrid(bool snapToGrid) { snapToGrid_.set(snapToGrid); }
    bool areChildrenVisible() const { return childrenVisible_.get(); }
    void setChildrenVisible(bool childrenVisible) { childrenVisible_.set(childrenVisible); }

    tgt::vec4 getColor() const { return color_.get(); }
    void setColor(tgt::vec4 color) { color_.set(color); }

    tgt::mat4 getTransformation() const { return transformation_.get(); }
    void setTransformation(tgt::mat4 transformation) { transformation_.set(transformation); }

    // Statistics:
    std::vector<std::string> getAvailableStatistics() const;
    const MetaDataBase* getStatistic(const std::string& id) const;
    void addStatistic(const std::string& id, MetaDataBase* stat) const;
    void removeStatistic(const std::string& id) const;
    void clearStatistics() const;

    private:
    StringProperty name_;
    StringProperty comment_;
    BoolProperty isVisible_;
    BoolProperty isSelected_;
    BoolProperty snapToGrid_;
    BoolProperty childrenVisible_;
    FloatVec4Property color_;
    FloatMat4Property transformation_;

    Grid grid_;

    mutable std::map<std::string, MetaDataBase*> statistics_;
    mutable ROIMeshCache cache_;

    static const std::string loggerCat_;
};

tgt::plane transform(const tgt::plane& pl, const tgt::mat4& m);
FaceGeometry createQuad(const tgt::plane& pl, const tgt::vec4& color);
/// Return the BB of roi in voxel coordinates (llf, urb).
std::pair<tgt::ivec3, tgt::ivec3> getVoxelBoundingBox(const VolumeBase* vh, const ROIBase* roi);

} //namespace

#endif
