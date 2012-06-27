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

#ifndef VRN_VECTORQUANT_H
#define VRN_VECTORQUANT_H

#include "tgt/logmanager.h"

#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "tgt/config.h"

#define VRN_VQ_L1
//#define VRN_VQ_L2
//do not use this:
// #define VRN_VQ_LMAX

//#define VRN_VQ_ANGLE

//central configuration for DAO:
#define HISTOGRAM_BUCKETS 256
#define HISTOGRAM_TYPE float
// #define HISTOGRAM_CODEWORDS 1024 - obsolete
#define HISTOGRAM_BUCKETS_PACKED 16
#define HISTOGRAM_TYPE_PACKED float
#define NUM_REGIONS 2

template <typename T, int S>
struct vqVector;

template <typename T, int S>
inline float squareDist(const vqVector<T, S> &v0, const vqVector<T, S> &v1)
{
    float dist = 0.0f;
    for (int i=0; i<S; i++)
        dist += ((v0.data_[i]-v1.data_[i]) * (v0.data_[i]-v1.data_[i]));
    return dist;
}

template <typename T, int S>
inline float l1Dist(const vqVector<T, S> &v0, const vqVector<T, S> &v1)
{
    float dist = 0.0f;
    for (int i=0; i<S; i++)
        dist += fabs(v0.data_[i]-v1.data_[i]);
    return dist;
}

template <typename T, int S>
inline float maxDist(const vqVector<T, S> &v0, const vqVector<T, S> &v1)
{
    float maxDist = 0.0f;
    for (int i=0; i<S; i++)
    {
        float dist = fabs(v0.data_[i]-v1.data_[i]);
        if (maxDist<dist)
            maxDist = dist;
    }
    return maxDist;
}

// create distance from angle between two vectors
template <typename T, int S>
inline float angleDist(const vqVector<T, S> &v0, const vqVector<T, S> &v1)
{
    float angleDist = 0.0f;
    float v0length = 0.0f;
    float v1length = 0.0f;
    for (int i=0; i<S; ++i)
        v0length += v0.data_[i]*v0.data_[i];
    v0length = sqrt(v0length);
    v0length = 1.0f / v0length;
    for (int i=0; i<S; ++i)
        v1length += v1.data_[i]*v1.data_[i];
    v1length = sqrt(v1length);
    v1length = 1.0f / v1length;
    for (int i=0; i<S; i++)
    {
        angleDist += v0.data_[i]*v0length * v1.data_[i]*v1length;
    }
    angleDist = (1.0f-angleDist)/2.0f;
    return angleDist;
}

//Vector for VQ:
template <typename T, int S>
struct vqVector
{
    static float (*distFunction)(const vqVector<T,S>& v0, const vqVector<T,S>& v1);
    T data_[S];

    /// Wrapper to select one distance function
    float dist(const vqVector<T, S> &v)const
    {
        return (*distFunction)(*this, v);
    }
    static void setDistFunction(int i)
    {
        switch (i) {
        case 0:
            distFunction = l1Dist<T,S>;
            break;
        case 1:
            distFunction = squareDist<T,S>;
            break;
        case 2:
            distFunction = maxDist<T,S>;
            break;
        case 3:
            distFunction = angleDist<T,S>;
            break;
        default:
            distFunction = 0;
        }
    }
    ///Add another vqVector
    void operator+=(const vqVector<T, S>& v2)
    {
        for (int j=0; j<S; j++)
        {
            data_[j] += v2.data_[j];
        }
//         return *this;
    }

    ///Set all dimensions to value
    void initialize(T value = 0)
    {
        for (int i=0; i<S; ++i)
            data_[i] = value;
    }

    ///Divide every dimension by value
    void operator/=(T value)
    {
        for (int i=0; i<S; ++i)
            data_[i] /= value;
    }

    ///Writes data in textfile for gnuplot
    ///@param comment Comment that will be placed inside the file
    void writeForPlot(std::string filename, std::string comment = "") {
        std::fstream file(filename.c_str(), std::ios::out);
        file << "# S = " << S << std::endl;
        file << "#" << comment << std::endl;

        for (int i=0; i<S; ++i) {
            file << i << " " << data_[i] << std::endl;
        }

        file.close();
    }

    ///Returns the value found in all dimensions
    T getMaxValue() {
        T max = data_[0];
        for (int i=1; i<S; ++i) {
            if (data_[i] > max)
                max = data_[i];
        }
        return max;
    }
};

template <typename T, int S>
#ifdef VRN_VQ_L1
    float (*vqVector<T,S>::distFunction)(const vqVector<T,S>& v0, const vqVector<T,S>& v1) = squareDist<T,S>;
#elif defined(VRN_VQ_L2)
    float (*vqVector<T,S>::distFunction)(const vqVector<T,S>& v0, const vqVector<T,S>& v1) = l1Dist<T,S>;
#elif defined(VRN_VQ_LMAX)
    float (*vqVector<T,S>::distFunction)(const vqVector<T,S>& v0, const vqVector<T,S>& v1) = maxDist<T,S>;
#elif defined(VRN_VQ_ANGLE)
    float (*vqVector<T,S>::distFunction)(const vqVector<T,S>& v0, const vqVector<T,S>& v1) = angleDist<T,S>;
#endif

//-----------------------------------------------------------------------------

//VQ codebook: type of vector, size of vectors, size of codebook
template <typename T, int S>
class vqCodebook
{
    static const std::string loggerCat_;
    float disturbFactor_;
    std::set<int> deadCWs_;
    int activeCWs_;
    int numCodes_;

public:
    vqVector<T, S>* codewords_;
    int numRegions_;
    std::vector<std::string> comments_;
public:

    vqCodebook(int numCodes)
    {
        numCodes_ = numCodes;
        codewords_ = new vqVector<T, S>[numCodes_];

        numRegions_ = 1;
        disturbFactor_ = 0.0001f;
        activeCWs_ = 0;
    }

    ///Return the index of the codeword closest to v.
    ///@param maxCW Limit search to the first maxCW codewords.
    int getClosestCodeword(const vqVector<T, S> &v, int maxCW = -1)
    {
        if (maxCW == -1)
            maxCW = numCodes_;
        float closestDist = v.dist(codewords_[0]);
        int closest = 0;
        for (int i=1; i<maxCW; i++)
        {
            if (v.dist(codewords_[i]) < closestDist)
            {
                closest = i;
                closestDist = v.dist(codewords_[i]);
            }
        }
        return closest;
    }

    vqVector<T, S> getCodeword(int i)
    {
        return codewords_[i];
    }

    int getActiveCodewords() {
        return activeCWs_;
    }

protected:
    ///Calculate average error of the vectors in trainingVector
    ///@param codes Indices of the codewords assigned to the vectors in trainingVector
    float getError(const std::vector< vqVector<T, S> > &trainingVector, const std::vector<int> &codes)
    {
        float er = 0.0;
        int m = trainingVector.size();
//#ifdef _OPENMP
//#pragma omp parallel for shared(codewords_, codes, m) reduction(+:er)
//#endif
        for (int i=0; i<m; i++)
        {
            er += trainingVector[i].dist(codewords_[codes[i]]);
        }
        return er/trainingVector.size();
    }

    ///Optimize position of codewords by moving them to the center of mass of all assigned trainingvectors.
    ///@param codes Indices of the codewords assigned to the vectors in trainingVector
    ///@param maxCW Limit calculation to the first maxCW codewords.
    void optimize(const std::vector< vqVector<T, S> > &trainingVector, const std::vector<int> &codes, int maxCW);

    ///Optimize with the current number of codewords until progress gets very small.
    ///@param epsilon Stop if (((er-newEr)/newEr) <= epsilon). Errors are calculated with getError each pass.
    ///@param maxCW Limit calculation to the first maxCW codewords.
    void train(const std::vector< vqVector<T, S> > &trainingVector, int maxCW/* = numCodes*/, float epsilon = 0.001);

    ///Splits the CW split
    ///CW is copied and random distortion is added.
    void splitCodeword(int split, float disturbSize);

public:
    ///Generate optimized set of codewords for the vectors in trainingVector.
    ///@param epsilon See learn method.
    ///@param threshold Stop if the maximal error for one trainingsvector is smaller than this threshold
    ///@return The final error (See getError method)
    float learn(const std::vector< vqVector<T, S> > &trainingVector, float epsilon = 0.001, float threshold = 0.00001);

     //FIXME: copied from voreen::TextFileReader. How can we share this code fragment?
     bool getNextLine(std::ifstream *file_, std::string& type, std::istringstream& args,
                      bool toLowercase=true, std::string endTag="EndConfig");

     bool load(std::string filename);

     bool save(std::string filename, std::vector<std::string>& comments);


     template<typename T_i, int S_i>
     class CmpVqVectors {
     public:
        int operator()(const vqVector<T_i, S_i>& p1, const vqVector<T_i,S_i> &p2) {
            return p1.dist(p1)<p2.dist(p2);
        }
     };

     void sort()
     {
         std::sort(codewords_, codewords_+S, CmpVqVectors<T, S>());
     }
     inline static int getDimensions()
     {
        return S;
     }

     inline int getNumCodewords()
     {
        return numCodes_;
     }
};

template <typename T, int S>
const std::string vqCodebook<T,S>::loggerCat_("vrn.vq");

template <typename T, int S>
void vqCodebook<T, S>::optimize(const std::vector< vqVector<T, S> > &trainingVector, const std::vector<int> &codes, int maxCW)
{
    vqVector<T, S> *newwords = new vqVector<T, S>[numCodes_];  //holds the optimized codewords
    int* count = new int [numCodes_];    //counts number of trainingvectors assigned to the codewords
    memset( count, '\0', sizeof(int)*numCodes_);
    memset( newwords, '\0', sizeof(vqVector<T, S>) * numCodes_);

    //calculate center of mass:
    int m = trainingVector.size();
    for (int i=0; i<m; i++)
    {
        newwords[codes[i]] += trainingVector[i];
        count[codes[i]]++;
    }

//    #ifdef _OPENMP
//        #pragma omp parallel for shared(maxCW, codewords_, newwords, count)
//    #endif
    for (int i=0; i<maxCW; i++)
    {
        //no trainingvectors assigned to this CW...prevent division by zero and add to deadCWs set for recycling
        if (count[i] == 0)
        {
            std::cout << "-";
            deadCWs_.insert(i);
        }
        else
            for (int j=0; j<S; j++)
            {
                codewords_[i].data_[j] = newwords[i].data_[j] / count[i];
            }
    }
    delete[] count;
    delete[] newwords;
}

template <typename T, int S>
void vqCodebook<T, S>::train(const std::vector< vqVector<T, S> > &trainingVector, int maxCW, float epsilon)
{
    using std::vector;

    vector<int> cw;
    cw.resize(trainingVector.size());

    int M = trainingVector.size();
#ifdef _OPENMP
#pragma omp parallel for shared(cw, trainingVector, maxCW, M)
#endif
    for (int i=0; i<M; i++)
    {
        cw[i] = getClosestCodeword(trainingVector[i], maxCW);
    }

    float er;
    float newEr = getError(trainingVector, cw);
    int steps = 0;
    do
    {
        deadCWs_.clear();
        er = newEr;
        optimize(trainingVector, cw, maxCW);
        std::cout << "|";
        steps++;
        int m = trainingVector.size();
#ifdef _OPENMP
#pragma omp parallel for shared(cw, trainingVector, maxCW, m)
#endif
        for (int i=0; i<m; i++)
        {
            cw[i] = getClosestCodeword(trainingVector[i], maxCW);
        }
        newEr = getError(trainingVector, cw);
            //std::cout << "Error after " << steps << " training steps: " << newEr << std::endl;
            //std::cout << "((er-newEr)/newEr) " << ((er-newEr)/newEr) << " epsilon: " << epsilon << std::endl;
    }
    while (((er-newEr)/newEr) > epsilon);
}

template <typename T, int S>
void vqCodebook<T, S>::splitCodeword(int split, float disturbSize)
{
    float move;
    float adj = 1.0;

    //select a free or dead CW:
    int target = activeCWs_;
    if (!deadCWs_.empty())
    {
        std::set<int>::iterator theIterator;
        for ( theIterator = deadCWs_.begin(); theIterator != deadCWs_.end(); theIterator++ ) {
            std::cout << *theIterator << " ";
        }
        std::cout << std::endl;
        target = *(deadCWs_.begin());
//             adj = 0.1f;
        std::cout << "Re-using dead CW " << target << std::endl;
        deadCWs_.erase(target);
        for ( theIterator = deadCWs_.begin(); theIterator != deadCWs_.end(); theIterator++ ) {
            std::cout << *theIterator << " ";
        }
        std::cout << std::endl;
    }
    else
    {
        //use the next free codeword
        ++activeCWs_;
    }

    //copy and disturb old and new CW with a random offset
    for (int j=0; j<S; j++)
    {
        move = (static_cast<float>(rand()) * adj * disturbSize / static_cast<float>(RAND_MAX))/* + disturbSize*/;
        codewords_[target].data_[j] = codewords_[split].data_[j];
        codewords_[split].data_[j] = (T)(static_cast<float>(codewords_[split].data_[j]) + move);
        codewords_[target].data_[j] = (T)(static_cast<float>(codewords_[target].data_[j]) - move);
    }
}

template <typename T, int S>
float vqCodebook<T, S>::learn(const std::vector< vqVector<T, S> > &trainingVector, float epsilon, float threshold)
{
    using std::vector;
    activeCWs_ = 1;

#ifdef _OPENMP
        std::cout << "Using OpenMP for vqtrain. Number of CPUs: " << omp_get_num_procs() << std::endl;
#endif
        time_t startTime = time(0) - 1; // -1 prevents division by zero

        //srand( time(NULL) );
    srand(42);
    std::cout << "ATTENTION: set random seed to fixed value." << std::endl;

    codewords_[0].initialize();

    //calculate center of mass as first codeword:
    for (size_t i=0; i<trainingVector.size(); i++)
    {
        codewords_[0] += trainingVector[i];
    }
    codewords_[0] /= static_cast<T>(trainingVector.size());

    vector<int> cw; //holds the indices of the assigned codewords
    //the closest CW is 0 for all vectors...
    for (size_t i=0; i<trainingVector.size(); i++)
    {
        cw.push_back(0);
    }

    std::vector<float> errors(numCodes_); //the error of each CW, summed up
    std::vector<float> maxErrors(numCodes_); //the max error of each CW
    int *count = new int[numCodes_];    //the number of trainingvectors that are closest to the respective CW

    int loopRepeat = 0; //counts the number of times the loop has not added a new CW
    int lastActiveCWs = 0;

    //add codewords until max number is reached and no more CWs are dead (=no trainingvectors use this cw)
    while ((activeCWs_ < numCodes_) || (!deadCWs_.empty()))
    {
        if (lastActiveCWs == activeCWs_) {
            ++loopRepeat;
            if (loopRepeat > 10) {
                std::cout << "No new CWs added in the last 10 iterations...stopping." << std::endl;
                break;
            }
        }
        else
            loopRepeat = 0;

        lastActiveCWs = activeCWs_;

        std::cout << "Adding " << activeCWs_ << ". codeword (from " << numCodes_ << ")" << std::endl;
        //reset all errors and counts:
        for (int i=0; i < activeCWs_; i++)
        {
            errors[i] = 0.0;
            maxErrors[i] = 0.0;
            count[i] = 0;
        }

        //sum up errors for each active CW:
        int m = trainingVector.size();

        //problem: threads have to share var count.
        //writing of multiple threads to one variable has to be synchronized (to much overhead here)
        //OR has to be mapped to one var at the end of all threads by openmp:reduction (not applicable here
        //cause count is an array and reduction can ONLY be applied to scalars) (jk)
        //see http://www.llnl.gov/computing/tutorials/openMP/#Reduction for details
//            #ifdef _OPENMP
//            #pragma omp parallel for shared(errors, cw, trainingVector, codewords_)
//            #endif

        for (int i=0; i<m; i++)
        {
            if (maxErrors[cw[i]] < trainingVector[i].dist(codewords_[cw[i]]))
                maxErrors[cw[i]] = trainingVector[i].dist(codewords_[cw[i]]);
            errors[cw[i]] += trainingVector[i].dist(codewords_[cw[i]]);
            count[cw[i]]++;
        }

        //find CW with max and min error:
        int worst = 0;
        int best = 0;
        if (count[0] > 1)
            errors[0] /= count[0];
        float maxerr = errors[0];
        float minerr = errors[0];

        float maxSingleErr = maxErrors[0];

        for (int i=1; i < activeCWs_; i++)
        {
            if (count[i] > 1)
                errors[i] /= count[i];
            if (errors[i] > maxerr)
            {
                maxerr = errors[i];
                worst = i;
            }
            if (errors[i] < minerr)
            {
                minerr = errors[i];
                best = i;
            }

            if (maxErrors[i] > maxSingleErr)
                maxSingleErr = maxErrors[i];
        }
        if (maxSingleErr < threshold) {
            std::cout << "\nmaxSingleErr is below threshold(" << threshold<< "); stopping VQ\n\n";
            break;
        }
        else
            std::cout << "max single error: " << maxSingleErr << std::endl;

        std::cout << "CW with max error: " << worst << " (" << maxerr << ") Count: " << count[worst] << std::endl;
        std::cout << "CW with min error: " << best << " (" << minerr << ") Count: " << count[best] << std::endl;

//        #define ADD_SINGLE 1
        #ifdef ADD_SINGLE
            //add a single codeword by splitting the one worst one
            std::cout << "Distortion: " << sqrt(maxerr / (static_cast<float>(count[worst]) * S)) << std::endl;
            //add a new CW and move the worst and the new CW a bit apart:
            splitCodeword(worst, maxerr / static_cast<float>(count[worst]) * S);
        #else
            std::vector<float> errors_sort(errors);
//             std::sort(errors_sort.begin(), errors_sort.begin()+activeCWs_);

            //Add multiple CWs by splitting all CWs with errors above threshold
            float threshold;
            threshold = 0.5f * minerr + 0.5f * maxerr;
//             threshold = errors_sort[(int)(activeCWs_*0.7)];
//             threshold = errors_sort[(int)(activeCWs_*0.8)];
            for (int i=0; i < activeCWs_; i++)
            {
                if ((errors[i] >= threshold) && ((activeCWs_ < numCodes_) || (!deadCWs_.empty())))
                {
//                     std::cout << "Splitting codeword with error: " << errors[i] << " and count " << count[i];
                    splitCodeword(i, 0.1f *errors[i] / (static_cast<float>(count[i]) * S));
                }
            }
        #endif

        //Optimize VQ with current number of Cws...
        train(trainingVector, activeCWs_, epsilon);

        //update cw:
        m = trainingVector.size();
        // OPENMP cannot handle member variables of template classes(???) (jms)
        int activeCWs = activeCWs_;

#ifdef _OPENMP
#pragma omp parallel for shared(cw, trainingVector, activeCWs, m)
#endif
        for (int i=0; i<m; i++)
        {
            cw[i] = getClosestCodeword(trainingVector[i], activeCWs);
        }
    }
    std::cout << "vqtrain lasted " << (time(0) - startTime) / 60 << " min." << std::endl;
    std::cout << "Final error: " << getError(trainingVector, cw) << std::endl;
    delete[] count;
    return getError(trainingVector, cw);
}

template <typename T, int S>
bool vqCodebook<T, S>::getNextLine(std::ifstream *file_, std::string& type, std::istringstream& args,
                      bool toLowercase, std::string endTag)
{
    std::string line;
    if (file_->eof())
        return false;
    do {
        *file_ >> type;
        getline(*file_, line);
        if (type.find(endTag) == 0 && type.length() == endTag.length())
            return false; //
    }
    while (type.at(0) == '#' && !file_->eof());
    if (file_->eof())
        return false;
    if (toLowercase) {
        for (size_t i=0; i<type.length(); ++i) {
            type.at(i) = tolower(type.at(i));
        }
    }
    args.clear();
    args.rdbuf()->str(line);
    //cout << "rdstate: " << args.rdstate() << endl;
    //cout << "content: " << args.rdbuf() << endl;
    //args.seekg(0, std::ios_base::beg);
    //file.ignore(' ');
    return true;
}

template <typename T, int S>
bool vqCodebook<T, S>::load(std::string filename)
{
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (file.good())
    {
        uint8_t magicNumber = 0;
        file.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));
        file.seekg(0);
        if (magicNumber < 0x20 ) {
            uint16_t size;
            uint16_t dim;
            uint16_t numc;
            file.read(reinterpret_cast<char*>(&size), sizeof(uint16_t));
            file.read(reinterpret_cast<char*>(&dim), sizeof(uint16_t));
            file.read(reinterpret_cast<char*>(&numc), sizeof(uint16_t));
            numCodes_ = numc;
            delete[] codewords_;
            codewords_ = new vqVector<T, S>[numCodes_];
            if ((size != sizeof(T)) || (dim != S))
            {
                LERROR("Codebook could not be loaded because template parameters do not match!");
                LERROR("size "<< size << " sizeof(T) " << sizeof(T));
                LERROR("dim " << dim << " S " << S);
            }
            file.read(reinterpret_cast<char*>(codewords_), sizeof(vqVector<T, S>)*numCodes_);
        }
        else {
            int offset;
            std::string type;
            std::istringstream args;
            while (getNextLine(&file, type, args, false)) {
                LINFO("Type: " << type);
                if (type == "Version:") {
                    std::string version;
                    args >> version;
                    LINFO("Value: " << version);
                    if (version != "1.1")
                        LWARNING("Unknown version.");
                }
                else if (type == "Buckets:") {
                    int buckets;
                    args >> buckets;
                    LINFO("Value: " << buckets);
                    if (buckets != S) {
                        LERROR("Codebook could not be loaded because bucket size does not "
                            << "match (" << S << " expected).");
                        return false;
                    }
                }
                else if (type == "SizeOf:") {
                    int sizeOf;
                    args >> sizeOf;
                    LINFO("Value: " << sizeOf);
                    if (sizeOf != sizeof(T)) {
                        LERROR("Codebook could not be loaded because SizeOf does not "
                            << "match (" << sizeof(T) << " expected).");
                        return false;
                    }
                }
                else if (type == "Codewords:") {
                    int numC;
                    args >> numC;
                    LINFO("Value: " << numC);
                    numCodes_ = numC;
                    delete[] codewords_;
                    codewords_ = new vqVector<T, S>[numCodes_];
                }
                else if (type == "Comment:") {
                    comments_.push_back(args.str());
                }
                else if (type == "Offset:") {
                    args >> offset;
                    LINFO("Value: " << offset);
                }
                else if (type == "Regions:") {
                    args >> numRegions_;
                    LINFO("Value: " << numRegions_);
                }
                else {
                LERROR("Unknown type");
                }
                if ( args.fail() ) {
                LERROR("Format error");
                }
            }
            file.seekg(offset);
            file.read(reinterpret_cast<char*>(codewords_), sizeof(vqVector<T, S>)*numCodes_);
        }
        file.close();
        return true;
    }
    else
        return false;
}

template <typename T, int S>
bool vqCodebook<T, S>::save(std::string filename, std::vector<std::string>& comments)
{
    std::ofstream file(filename.c_str(), std::ios::binary);
    const int binOffset = 1024;
    file << "# cb.save" << std::endl;
    file << "Version:    1.1" << std::endl;
    file << "Buckets:    " << S << std::endl;
    file << "SizeOf:     " << sizeof(T) << std::endl;
    file << "Codewords:  " << numCodes_ << std::endl;
    file << "Offset:     " << binOffset << std::endl;
    file << "Regions:    " << numRegions_ << std::endl;
    for (size_t i=0; i<comments.size(); ++i)
        file << "Comment:    " << comments[i] << std::endl;
    file << "EndConfig" << std::endl;
    for (size_t i=0; i<comments_.size(); ++i)
        file << "Comment:    " << comments_[i] << std::endl;
    for (int i = file.tellp(); i<binOffset; ++i)
        file << " ";
    file.write(reinterpret_cast<char*>(codewords_), sizeof(vqVector<T, S>)*numCodes_);
    file.close();
    return true;
}

//-----------------------------------------------------------------------------

const int SOURCE = 2,
          DEST0 = 0,
          DEST1 = 1;

/**
  *copies unpacked src to packed dest using precalculated packScheme
  */
template<typename T, int S, typename U, int R>
vqVector<U, R> pack(const int* packScheme, vqVector<T, S> src) {
    vqVector<U, R> dest;
    for (int i=R+1; i<S; ++i) {
        src.data_[packScheme[3*i+DEST0]] += src.data_[packScheme[3*i+SOURCE]]/2;
        src.data_[packScheme[3*i+DEST1]] += src.data_[packScheme[3*i+SOURCE]]/2;
    }
    for (int i=0; i<R; ++i) {
        dest.data_[packScheme[3*i+DEST0]] = src.data_[packScheme[3*i+SOURCE]];
    }
    return dest;
}

/**
  *calculates packScheme for histogram of whole dataset
  *this packScheme is applied to envograms of every voxel in dataset by pack()
  */
template<int S, int R>
int* generatePackScheme(std::string filename) {
    std::string loggerCat_("vrn.vq");
    int* packScheme = new int[3*S];
    // load histogram
    std::ifstream file(filename.c_str(), std::ios::binary);
    int valuesCount;
    int bucketCount;
    file.read(reinterpret_cast<char*>(&valuesCount), sizeof(int));
    file.read(reinterpret_cast<char*>(&bucketCount), sizeof(int));
    if (bucketCount != S) {
        LERROR("Histogram size(" << bucketCount << ") doesn't match with bucket size (" << S << ")");
        exit(0);
        return packScheme;
    }
    int* histogram = new int[bucketCount];
    file.read(reinterpret_cast<char*>(histogram), bucketCount*sizeof(int));
    if ( !file.good() ) {
        LERROR("Problem while reading histogram.");
        exit(0);
    }
    // remain first R rows of packScheme empty for now
    for (int i=R; i<S; ++i) {
        // search minimal value in histogram
        int minValue = histogram[0];
        int minPos = 0;
        while (minValue == -1) {
            ++minPos;
            minValue = histogram[minPos];
        }
        for (int j=minPos+1; j<bucketCount; ++j) {
            if (histogram[j] != -1) {
                if (histogram[j] < minValue) {
                    minValue = histogram[j];
                    minPos = j;
                }
            }
        }
        // distribute to neighbours
        // find left neighbour
        int left = minPos-1;
        while (left >= 0 && histogram[left] == -1)
            --left;
        // find right neighbour
        int right = minPos+1;
        while (right < bucketCount && histogram[right] == -1)
            ++right;
        if (left < 0) { // special case: distribute all to right
            packScheme[3*i+DEST0] = right;
            packScheme[3*i+DEST1] = right;
            packScheme[3*i+SOURCE] = minPos;
        }
        else if (right >= bucketCount) { // special case: distribute all to left
            packScheme[3*i+DEST0] = left;
            packScheme[3*i+DEST1] = left;
            packScheme[3*i+SOURCE] = minPos;
        }
        else { // distribute to left and right
            packScheme[3*i+DEST0] = left;
            packScheme[3*i+DEST1] = right;
            packScheme[3*i+SOURCE] = minPos;
        }
        // mark column as deleted in histogram
        histogram[minPos] = -1;
        LINFO(i << ": Column " << minPos << " (minValue: " << minValue << "): source " << packScheme[3*i+SOURCE] <<
            ", dest 0 " << packScheme[3*i+DEST0] << ", dest 1 " << packScheme[3*i+DEST1]);
    }
    // remaining columns have to be copied
    // and the targets must be adapted
    int r = 0;
    for (int i=0; i<bucketCount; ++i) {
        if (histogram[i] != -1) {
            packScheme[3*r+SOURCE] = i;
            packScheme[3*r+DEST0] = r;
            packScheme[3*r+DEST1] = r;
            LINFO(r << ": Copy: source " << packScheme[3*r+SOURCE] <<
                  ", dest 0 " << packScheme[3*r+DEST0] << ", dest 1 " << packScheme[3*r+DEST1]);
            ++r;

        }
    }
    delete[] histogram;
    return packScheme;
}

#endif
