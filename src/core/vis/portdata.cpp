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

//
//#include "voreen/core/vis/portdata.h"
//
//PortData::PortData(Identifier type) 
//: type_(type)
//{}
//
//int PortData::getTarget() {
//	return -1;
//}
//
//void PortData::setTarget(int target) {
//}
//
////void PortData::insertTarget(int target) {
////}
//
//void PortData::call() {
//}
//
//PortDataTexture::PortDataTexture(int target) 
//: PortData("portdata.texture"),
//textureContainerTarget_(target)
//{
//}
//
//PortDataVolume::PortDataVolume(int target)
//: PortData("portdata.volume"),
//volumeContainerTarget_(target)
//{}
//
//PortDataCoProcessor::PortDataCoProcessor(Renderer* renderer, FunctionPointer functionPointer)
//: PortData("portdata.coprocessor"),
//renderer_(renderer),
//function_(functionPointer)
//{}
//
//void PortDataCoProcessor::call() {
//	int i=0;
//	renderer_->*function_;
//}