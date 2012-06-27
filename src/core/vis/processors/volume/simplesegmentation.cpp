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

#include "voreen/core/vis/processors/volume/simplesegmentation.h"

#include "voreen/core/volume/modality.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volumesetcontainer.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "tgt/quadric.h"

#include <fstream>

using tgt::vec3;
using tgt::vec4;
using tgt::ivec3;

namespace voreen {

GeomSimpleSegmentation::GeomSimpleSegmentation()
    : GeometryRenderer(),
      showSkelPoints_("showSkelPoints", "show skel points", true),
      showCritPoints_("showCritPoints", "show critical points", true),
      showBoundingBox_("showBoundingBox", "show bounding box", true),
      spherePoints_("spherePoints", "show spheres", false),
      sphereSize_("sphereSize", "sphere size", 0.004f, 0.f, 0.9f),
      sphereModulo_("sphereModulo", "sphere modulo", 3, 1, 100),
      colorSegments_("colorSegments", "color individual segments", false)
{
    setName("Simple Segmentation Geometry");

    createCoProcessorOutport("coprocessor.simplesegmentationgeom", &Processor::call);
    createCoProcessorInport("coprocessor.proxygeometry");
    createCoProcessorInport("coprocessor.simplesegmentation");

    setIsCoprocessor(true);

    addProperty(&showSkelPoints_);
    addProperty(&showCritPoints_);
    addProperty(&showBoundingBox_);
    addProperty(&spherePoints_);
    addProperty(&sphereSize_);
    sphereSize_.setNumDecimals(3);
    addProperty(&sphereModulo_);
    addProperty(&colorSegments_);
}

const std::string GeomSimpleSegmentation::getProcessorInfo() const {
    return "Renders geometry hints related to simple segmentation.";
}

void GeomSimpleSegmentation::render(LocalPortMapping* portMapping) {

    PortDataCoProcessor* pgPort = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    if (!pgPort)
        return;

    Message* sizeMsg = pgPort->call(ProxyGeometry::getVolumeSize_);
    vec3 dim;
    if (sizeMsg)
        dim = sizeMsg->getValue<tgt::vec3>();
    else
        return;
    delete sizeMsg;

    PortDataCoProcessor* segPort = portMapping->getCoProcessorData("coprocessor.simplesegmentation");
    std::vector<vec3>* points = 0;
    std::vector<vec3>* critpoints = 0;
    std::vector<int>* skelsegcounts = 0;
    if (segPort) {
        Message* pointsMsg = segPort->call("simplesegmentation.getgeometry");
        if (pointsMsg)
            points = pointsMsg->getValue<std::vector<vec3>*>();
        delete pointsMsg;

        pointsMsg = segPort->call("simplesegmentation.getskelsegcounts");
        if (pointsMsg)
            skelsegcounts = pointsMsg->getValue<std::vector<int>*>();
        delete pointsMsg;

        pointsMsg = segPort->call("simplesegmentation.getcrit");
        if (pointsMsg)
            critpoints = pointsMsg->getValue<std::vector<vec3>*>();
        delete pointsMsg;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    glColor4f(0.8f, 0.8f, 0.8f, 1.f);

    vec3 geomLlf = -dim;
    vec3 geomUrb = dim;

    if (showBoundingBox_.get()) {
        glBegin(GL_LINE_LOOP);
        // back face
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        glEnd();
        glBegin(GL_LINE_LOOP);
        // front face
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);

        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);

        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);

        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glEnd();
    }

    if (points && showSkelPoints_.get()) {
        glColor4f(1.f, 0.f, 0.f, 1.f);
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_POINTS);
        for (size_t i=0; i < points->size(); i++) {
            vec3 p = (((*points)[i] * 2.0f) - 1.f) * dim;
            glVertex3fv(p.elem);
        }
        glEnd();
        glEnable(GL_DEPTH_TEST);
    }

    if (critpoints && showCritPoints_.get()) {
        glColor4f(0.f, 1.f, 0.f, 1.f);
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_POINTS);
        for (size_t i=0; i < critpoints->size(); i++) {
            vec3 p = (((*critpoints)[i] * 2.0f) - 1.f) * dim;
            glVertex3fv(p.elem);
        }
        glEnd();
        glEnable(GL_DEPTH_TEST);
    }

    if (points && spherePoints_.get()) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);



        tgt::Sphere sphere(sphereSize_.get(), 20, 20);
        tgt::Material mat(vec4(0.7f, 0.0f, 0.0f, 1.f),
                          vec4(0.3f, 0.0f, 0.0f, 1.f),
                          vec4(0.7f, 0.6f, 0.6f, 1.f),
                          25.f);

        vec3 colormap[] = { vec3(1.0, 0.0, 0.0),
                            vec3(0.0, 1.0, 0.0),
                            vec3(0.0, 0.0, 1.0),
                            vec3(1.0, 1.0, 0.0),
                            vec3(0.0, 1.0, 1.0),
                            vec3(1.0, 0.0, 1.0),
                            vec3(1.0, 0.5, 1.0),
                            vec3(1.0, 0.0, 0.5),
                            vec3(0.5, 1.0, 0.0),
                            vec3(0.0, 1.0, 0.5)
        };

        sphere.setMaterial(&mat);
        glEnable(GL_LIGHTING);

        GLfloat light_pos[4];      ///< position of the light source
        GLfloat light_ambient[4];  ///< ambient light parameter
        GLfloat light_diffuse[4];  ///< diffuse light parameter
        GLfloat light_specular[4]; ///< specular light parameter

        light_pos[0]      =  0.0f;
        light_pos[1]      =  1.0f;
        light_pos[2]      =  1.1f;
        light_pos[3]      =  1.0f;
        light_ambient[0]  =  0.7f;
        light_ambient[1]  =  0.7f;
        light_ambient[2]  =  0.7f;
        light_ambient[3]  =  1.0f;
        light_diffuse[0]  =  0.9f;
        light_diffuse[1]  =  0.9f;
        light_diffuse[2]  =  0.9f;
        light_diffuse[3]  =  1.0f;
        light_specular[0] =  1.0f;
        light_specular[1] =  1.0f;
        light_specular[2] =  1.0f;
        light_specular[3] =  1.0f;

        glDisable(GL_LIGHT0);
        glLightfv(GL_LIGHT3, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT3, GL_AMBIENT,  light_ambient);
        glLightfv(GL_LIGHT3, GL_DIFFUSE,  light_diffuse);
        glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);
        glEnable(GL_LIGHT3);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, light_diffuse);

        glClear(GL_DEPTH_BUFFER_BIT);
        int seg = 0;
        int inseg = 0;
        int color = 0;
        if (colorSegments_.get())
            mat.setValues(vec4(colormap[color] * 0.7f, 1.f),
                          vec4(colormap[color] * 0.3f, 1.f),
                          vec4(colormap[color] * 0.7f, 1.f),
                          25.f, vec4(0.f));
        for (size_t i=0; i < points->size(); i++) {
            if (i % sphereModulo_.get() != 0)
                continue;

            glPushMatrix();
            vec3 p = (((*points)[i] * 2.0f) - 1.f) * dim;
            glTranslatef(p.x, p.y, p.z);
            sphere.render();
            glPopMatrix();

            inseg++;
            if ((colorSegments_.get()) && inseg >= (*skelsegcounts)[seg]) {
                seg++;
                inseg = 0;
                mat.setValues(vec4(colormap[color] * 0.7f, 1.f),
                              vec4(colormap[color] * 0.3f, 1.f),
                              vec4(colormap[color] * 0.7f, 1.f),
                              25.f, vec4(0.f));
                color = (color + 1) % 10;
            }
        }
        glPopAttrib();
    }

    glPopAttrib();
}

//----------------------------------------------------------------------------------------------

const std::string SimpleSegmentationProcessor::loggerCat_("voreen.SimpleSegmentationProcessor");

SimpleSegmentationProcessor::SimpleSegmentationProcessor()
    : VolumeRenderer(),
      volume_(0),
      segmentation_(0),
      fhp_(-1),
      strictness_("strictness", "strictness", 0.8f, 0.f, 65535.f),
      thresholdFilling_("thresholdFilling", "apply threshold on flood fill", false),
      adaptive_("adaptive", "use adaptive growing criteria", false),
      maxSeedDistance_("maxSeedDistance", "maximum distance to seed point", 0, 0, 999),
      lastSegmentation_(0)
{
    setName("SimpleSegmentationProcessor");

    //
    // ports
    //
    createInport("volumehandle.volumehandle");
    createInport("volumehandle.segmentationvolumehandle");
    createInport("image.fhp");
    createOutport("image.output");

    createCoProcessorOutport("coprocessor.simplesegmentation", &Processor::call);

    //
    // properties
    //
    addProperty(&strictness_);
    addProperty(&thresholdFilling_);

    std::vector<std::string> costFunctions;
    costFunctions.push_back("intensity");
    costFunctions.push_back("gradient magnitude");
    costFunctions.push_back("weighted");
    fillCostFunction_ = new EnumProp("fillCostFunction", "Cost function", costFunctions);
    addProperty(fillCostFunction_);

    addProperty(&adaptive_);
    addProperty(&maxSeedDistance_);
}

SimpleSegmentationProcessor::~SimpleSegmentationProcessor() {
    delete lastSegmentation_;
    delete fillCostFunction_;
}

const std::string SimpleSegmentationProcessor::getProcessorInfo() const {
    return "Simple interactive volume segmentation.";
}

namespace {

// calculate mean, variance and standard deviation for some float values
class Stats {
public:
    Stats()
        : n_(0), mean_(0.f), m2_(0.f) {}

    void add(float x) {
        // on-line algorithm, see http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        n_++;
        float delta = x - mean_;
        mean_ += delta / n_;
        m2_ += delta * (x - mean_);
    }

    float mean() const { return mean_; }
    float variance() const { return m2_ / (n_ - 1); }
    float stddev() const { return std::sqrt(variance()); }

protected:
    int n_;
    float mean_;
    float m2_;
};


// Calculate standard deviation of the intensity values from the 26 neighbors of the voxel at
// position pos. Note: position must not be a border voxel.
template<class T>
float neighborStandardDeviation(const tgt::ivec3& pos, T* dataset, float max, Stats& stats) {
    for (int z=-1; z <= 1; z++)
        for (int y=-1; y <= 1; y++)
            for (int x=-1; x <= 1; x++) {
                ivec3 p(x, y, z);
                if (p != ivec3::zero)
                    stats.add(dataset->voxel(pos + p) / max);
            }

    return stats.stddev();
}

// Calculate gradient magnitude. Note: position must not be a border voxel.
template<class T>
float gradientMagnitude(const tgt::ivec3& pos, T* dataset, float max) {
    float v0 = dataset->voxel(pos + ivec3(1, 0, 0)) / max;
    float v1 = dataset->voxel(pos + ivec3(0, 1, 0)) / max;
    float v2 = dataset->voxel(pos + ivec3(0, 0, 1)) / max;
    float v3 = dataset->voxel(pos - ivec3(1, 0, 0)) / max;
    float v4 = dataset->voxel(pos - ivec3(0, 1, 0)) / max;
    float v5 = dataset->voxel(pos - ivec3(0, 0, 1)) / max;
    vec3 gradient = vec3(v3 - v0, v4 - v1, v5 - v2);
    return length(gradient);
}

// Calculate standard deviation from the gradient magnitudes of the 26 neighbors of the voxel at
// position pos. Note: position must be at least 2 voxels away from the border.
template<class T>
float neighborStandardDeviationGradients(const tgt::ivec3& pos, T* dataset, float max, Stats& stats) {
    for (int z=-1; z <= 1; z++)
        for (int y=-1; y <= 1; y++)
            for (int x=-1; x <= 1; x++) {
                ivec3 p(x, y, z);
                if (p != ivec3::zero)
                    stats.add(gradientMagnitude(pos + p, dataset, max));
            }

    return stats.stddev();
}

enum FloodFillMode { FLOODFILL_INTENSITY, FLOODFILL_GRADMAG, FLOODFILL_WEIGHTED };

template<class T>
int floodFill(const ivec3& seed_pos, int segment, float lowerThreshold, float upperThreshold,
              float strictness, T* dataset, VolumeUInt8* segvol, FloodFillMode mode,
              bool useThresholds, bool adaptive, float maxSeedDistance)
{
    ivec3 dims = dataset->getDimensions();
    float max = static_cast<float>(VolumeElement<typename T::VoxelType>::max());

    VolumeAtomic<bool> markedVoxels(dims);
    markedVoxels.clear();

    std::vector<ivec3> neighbors;
    for (int z=-1; z <= 1; z++)
        for (int y=-1; y <= 1; y++)
            for (int x=-1; x <= 1; x++) {
                ivec3 p(x, y, z);
                if (p != ivec3::zero)
                    neighbors.push_back(p);
            }


    std::stack<ivec3> voxelStack;
    voxelStack.push(seed_pos);
    for (size_t i=0; i < neighbors.size(); i++)
        voxelStack.push(seed_pos + neighbors[i]);

    Stats stats_value;
    Stats stats_gradmag;

    float seed_value = dataset->voxel(seed_pos) / max;
    float seed_stddev26 = neighborStandardDeviation(seed_pos, dataset, max, stats_value);

    float seed_gradmag = gradientMagnitude(seed_pos, dataset, max);
    float seed_gradmag_stddev26 = neighborStandardDeviationGradients(seed_pos, dataset, max, stats_gradmag);

    if (mode == FLOODFILL_INTENSITY || mode == FLOODFILL_WEIGHTED)
        std::cout << "seed value: " << seed_value << ", "
                  << "stddev 26: " << seed_stddev26 << std::endl;

    if (mode == FLOODFILL_GRADMAG || mode == FLOODFILL_WEIGHTED)
        std::cout << "seed_gradmag: " << seed_gradmag << ", "
                  << "stddev gradmag 26: " << seed_gradmag_stddev26 << std::endl;

    while (!voxelStack.empty()) {
        ivec3 pos = voxelStack.top();
        voxelStack.pop();

        if (pos.x < 2 || pos.x > dims.x - 3 ||
            pos.y < 2 || pos.y > dims.y - 3 ||
            pos.z < 2 || pos.z > dims.z - 3 ||
            markedVoxels.voxel(pos))
        {
            // on border or already visited
            continue;
        }

        float value = dataset->voxel(pos) / max;

        if (adaptive /* && not in initial neighbor */) {
            stats_value.add(value);
            seed_stddev26 = stats_value.stddev();

            if (mode == FLOODFILL_GRADMAG || mode == FLOODFILL_WEIGHTED) {
                stats_gradmag.add(gradientMagnitude(pos, dataset, max));
                seed_gradmag_stddev26 = stats_gradmag.stddev();
            }
        }


        if (useThresholds && (value < lowerThreshold || value > upperThreshold)) {
            // invalid value
            continue;
        }

        // Cost function: if less than 1 then voxel is within the region.
        //
        // Based on: Runzhen Huang, Kwan-Liu Ma. RGVis: Region growing based techniques for
        // volume visualization, 2003.
        float cost = 0.f;

        if (mode == FLOODFILL_INTENSITY)
            cost = fabs(value - seed_value) / (strictness * seed_stddev26);
        else if (mode == FLOODFILL_GRADMAG) {
            float gradmag = gradientMagnitude(pos, dataset, max);
            cost = fabs(gradmag - seed_gradmag) / (strictness * seed_gradmag_stddev26);
        }
        else if (mode == FLOODFILL_WEIGHTED) {
            float cost_a = fabs(value - seed_value) / (strictness * seed_stddev26);

            float gradmag = gradientMagnitude(pos, dataset, max);
            float cost_b = fabs(gradmag - seed_gradmag) / (strictness * seed_gradmag_stddev26);

            // weight p
            float p = (seed_gradmag_stddev26 / (seed_stddev26 + seed_gradmag_stddev26));
            cost = cost_a * p + cost_b * (1.f - p);
        }

        if (cost >= 1.f)
            continue;

        if (maxSeedDistance > 0 && tgt::distance(vec3(pos), vec3(seed_pos)) > maxSeedDistance)
            continue;

        // voxel is valid
        markedVoxels.voxel(pos) = true;

        // add neighbors to stack if not already visited
        for (size_t i=0; i < neighbors.size(); i++)
            if (!markedVoxels.voxel(pos + neighbors[i]))
                voxelStack.push(pos + neighbors[i]);
    }

    // now fill segmentation volume with all marked voxels
    float count = 0;
    for (int z=0; z < markedVoxels.getDimensions().z; z++) {
        for (int y=0; y < markedVoxels.getDimensions().y; y++) {
            for (int x=0; x < markedVoxels.getDimensions().x; x++) {
                if (markedVoxels.voxel(x, y, z)) {
                    uint8_t& v = segvol->voxel(x, y, z);
                    if (v == 0 || segment == 0) {
                        v = segment;
                        count++;
                    }
                }
            }
        }
    }

    return static_cast<int>(count);
}

} // namespace

void SimpleSegmentationProcessor::mark(const ivec3& seedpos, int segment) {
    if (!volume_ || !segmentation_)
        return;

    VolumeUInt8* segvol = dynamic_cast<VolumeUInt8*>(segmentation_->getVolume());

    if (!segvol) {
        LERROR("no volume or segmentation");
        return;
    }

    float seedval = volume_->getVolume()->getVoxelFloat(seedpos);
    LINFO("seed pos " << seedpos << " with value " << seedval);

    if (seedval <= 0.f || seedpos == ivec3(0)) {
        LERROR("ignoring this seed value");
        return;
    }

    // copy copy seg volume to last volume to enable undo
    if (!lastSegmentation_ || lastSegmentation_->getDimensions() != segvol->getDimensions()) {
        delete lastSegmentation_;
        lastSegmentation_ = segvol->clone();
    } else {
        memcpy(lastSegmentation_->getData(), segvol->getData(), segvol->getNumBytes());
    }

    // start the flood fill
    Volume* v = volume_->getVolume();
    VolumeUInt8* s = dynamic_cast<VolumeUInt8*>(segmentation_->getVolume());
    FloodFillMode mode = FLOODFILL_INTENSITY;
    if (fillCostFunction_->get() == 1)
        mode = FLOODFILL_GRADMAG;
    else if (fillCostFunction_->get() == 2)
        mode = FLOODFILL_WEIGHTED;

    int count = 0;
    if (dynamic_cast<VolumeUInt8*>(v))
        //FIXME: thresholds are applied to transfer function. What to do here? (cdoer)
        count = floodFill(seedpos, segment, 0.f/*getLowerThreshold()*/, 1.f/*getUpperThreshold()*/, strictness_.get(),
                          reinterpret_cast<VolumeUInt8*>(v), s, mode, thresholdFilling_.get(), adaptive_.get(),
                          static_cast<float>(maxSeedDistance_.get()));
    else if (dynamic_cast<VolumeUInt16*>(volume_->getVolume()))
        //FIXME: thresholds are applied to transfer function. What to do here? (cdoer)
        count = floodFill(seedpos, segment, 0.f/*getLowerThreshold()*/, 1.f/*getUpperThreshold()*/, strictness_.get(),
                          reinterpret_cast<VolumeUInt16*>(v), s, mode, thresholdFilling_.get(), adaptive_.get(),
                          static_cast<float>(maxSeedDistance_.get()));

    LINFO("filled voxels: " << count);
}

void SimpleSegmentationProcessor::processMessage(Message* msg, const Identifier& dest) {
    VolumeRenderer::processMessage(msg, dest);

    if (msg->id_ == "simplesegmentation.clear" && segmentation_) {
        VolumeUInt8* segvol = dynamic_cast<VolumeUInt8*>(segmentation_->getVolume());
        if (segvol) {
            int segment = msg->getValue<int>();
            LINFO("clearing segment " << segment);
            for (size_t i=0; i < segvol->getNumVoxels(); i++) {
                uint8_t& v = segvol->voxel(i);
                if (v == segment)
                    v = 0;
            }
            volumeModified(segmentation_);
        }
    }
    else if (msg->id_ == "simplesegmentation.undo" && segmentation_) {
        if (lastSegmentation_ && segmentation_) {
            LINFO("undo");
            VolumeUInt8* segvol = dynamic_cast<VolumeUInt8*>(segmentation_->getVolume());
            if (segvol && lastSegmentation_->getDimensions() == segvol->getDimensions()) {
                // copy last volume to seg volume
                memcpy(segvol->getData(), lastSegmentation_->getData(), lastSegmentation_->getNumBytes());
                volumeModified(segmentation_);
                LINFO("undo finished");
            }
        }
    }
    else if (msg->id_ == "simplesegmentation.mark") {
        tgt::ivec2 pos = msg->getValue<ivec3>().xy();
        int segment = msg->getValue<ivec3>().z;
        if (fhp_ > 0) {
            float* f = tc_->getTargetAsFloats(fhp_, pos.x, getSize().y - pos.y);
            vec4 c(f);
            delete f;

            if (segmentation_) {
                Volume* segvol = segmentation_->getVolume();
                ivec3 voxelpos = c.xyz() * vec3(segvol->getDimensions());
                mark(voxelpos, segment);
                volumeModified(segmentation_);
            }
        } else {
            LERROR("invalid fhp");
        }
    }
    else if (msg->id_ == "simplesegmentation.newsegmentation") {
        if (segmentation_ &&
            segmentation_->getParentSeries()->getModality() == Modality::MODALITY_SEGMENTATION)
        {
            LINFO("already have segmentation volume");
        }
        else if (volume_) {
            VolumeUInt8* segvol = new VolumeUInt8(volume_->getVolume()->getDimensions());
            segvol->clear();
            VolumeSeries* series = new VolumeSeries(Modality::MODALITY_SEGMENTATION.getName(),
                                                    Modality::MODALITY_SEGMENTATION);
            VolumeHandle* handle = new VolumeHandle(segvol, 0.f);
            series->addVolumeHandle(handle);
            segmentation_->getParentSeries()->getParentSet()->addSeries(series);
            LINFO("created new segmentation volume");
        }

    }
    else if (msg->id_ == "simplesegmentation.savesegmentation") {
        std::string filename = msg->getValue<std::string>();
        if (segmentation_) {
            LINFO("saving segmentation volume to " << filename);
            DatVolumeWriter writer;
            writer.write(filename, segmentation_->getVolume());
        }
    }
    else if (msg->id_ == "simplesegmentation.savesegment" && segmentation_) {
        std::string filename = msg->getValue<std::pair<std::string,int> >().first;
        int segment = msg->getValue<std::pair<std::string,int> >().second;
        VolumeUInt8* segvol = dynamic_cast<VolumeUInt8*>(segmentation_->getVolume());
        if (segvol) {
            LINFO("saving segment " << segment << " to " << filename);
            VolumeUInt8* tmpvol = segvol->clone();

            ivec3 dims = tmpvol->getDimensions();
            for (int z=0; z < dims.z; z++) {
                for (int y=0; y < dims.y; y++) {
                    for (int x=0; x < dims.x; x++) {
                        // pfskel doesn't like voxel on the border ("object touching bounding
                        // box"), so zero them out here.
                        const int s = 4;
                        bool border = (x < s || y < s || z < s ||
                                       x >= dims.x - s || y >= dims.y - s || z >= dims.z - s);
                        uint8_t& v = tmpvol->voxel(x, y, z);
                        if (v == segment && !border)
                            v = 1;
                        else
                            v = 0;
                    }
                }
            }
            DatVolumeWriter writer;
            writer.write(filename, tmpvol);
            delete tmpvol;
        }
    }
    else if (msg->id_ == "simplesegmentation.clearskel") {
        skelPoints_.clear();
        skelSegCounts_.clear();
        critPoints_.clear();
        invalidate();
    }
    else if (msg->id_ == "simplesegmentation.loadskel" && segmentation_) {
        std::string filename = msg->getValue<std::string>();
        LINFO("loading skel file " << filename);

        std::ifstream fin(filename.c_str());
        if (!fin.good()) {
            LERROR("loading skel file failed");
            return;
        }

        vec3 p;
        int segment;
        int lastsegment = -1;
        float dummy;
        int count = 0;
        vec3 dims = vec3(segmentation_->getVolume()->getDimensions());
        while (fin >> p.x >> p.y >> p.z >> segment >> dummy) {
            if (segment != lastsegment) {
                skelSegCounts_.push_back(0);
                lastsegment = segment;
            }
            skelPoints_.push_back(p / dims);
            count++;
            skelSegCounts_[skelSegCounts_.size() - 1]++;
        }
        if (fin.eof()) {
            LINFO("read " << count << " skel points");
        }
        else
            LERROR("invalid file format!");




        filename = filename.substr(0, filename.size() - 5) + ".crit";
        LINFO("loading crit file " << filename);

        std::ifstream fincrit(filename.c_str());
        if (!fincrit.good()) {
            LERROR("loading crit file failed");
            return;
        }

        count = 0;
        while (fincrit >> p.x >> p.y >> p.z) {
            critPoints_.push_back(p / dims);
            count++;
            for (int i=0; i < 13; i++)
                fincrit >> dummy;
        }
        if (fincrit.eof()) {
            LINFO("read " << count << " crit points");
        }
        else
            LERROR("invalid file format!");

        invalidate();
    }
}

void SimpleSegmentationProcessor::process(LocalPortMapping* portMapping) {
    fhp_ = portMapping->getTarget("image.fhp");

    int dest = portMapping->getTarget("image.output");
    tc_->setActiveTarget(dest,"SimpleSegmentationProcessor::image.output");

    glClearColor(0.f, 1.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    volume_ = portMapping->getVolumeHandle("volumehandle.volumehandle");
    segmentation_ = portMapping->getVolumeHandle("volumehandle.segmentationvolumehandle");
}

Message* SimpleSegmentationProcessor::call(Identifier ident, LocalPortMapping* /*portMapping*/) {
    if (ident == "simplesegmentation.getgeometry")
        return new TemplateMessage<std::vector<vec3>*>("", &skelPoints_);
    if (ident == "simplesegmentation.getskelsegcounts")
        return new TemplateMessage<std::vector<int>*>("", &skelSegCounts_);
    if (ident == "simplesegmentation.getcrit")
        return new TemplateMessage<std::vector<vec3>*>("", &critPoints_);
    else
        return 0;
}

void SimpleSegmentationProcessor::volumeModified(VolumeHandle* v) {
    // Free the hardware volume, will be re-generated on the next repaint.
    v->freeHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);

    // hack to notify everybody that the volume was changed
    MsgDistr.postMessage(new TemplateMessage<VolumeSetContainer*>("volumesetcontainer.clear", 0));
    invalidate();
}

} // namespace
