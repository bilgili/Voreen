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
//
//
//namespace voreen {
//
//	class Renderer;
//
//class PortData {
//public:
//	PortData(Identifier type);
//	Identifier getType() {return type_;}
//	void setType(Identifier type) {type_ = type;}
//	virtual int getTarget();
//	virtual void setTarget(int target);
//	//virtual void insertTarget(int target);
//	virtual void call();
//protected:
//	Identifier type_;
//};
//
//class PortDataTexture : public PortData {
//public:
//	PortDataTexture(int target);
//	int getTarget() {return textureContainerTarget_;}
//	void setTarget(int target) {textureContainerTarget_ = target;}
////	void insertTarget(int target) {textureContainerTarget_.push_back(target);}
//protected:
//	int textureContainerTarget_;
//};
//
//class PortDataVolume : public PortData {
//public:
//	PortDataVolume(int target);
//	int getTarget() {return volumeContainerTarget_;}
//	void setTarget(int target) {volumeContainerTarget_ = target;}
////	void insertTarget(int target) {volumeContainerTargets_.push_back(target);}
//protected:
//	int volumeContainerTarget_;
//};
//
//class PortDataCoProcessor : public PortData {
//public:
//	PortDataCoProcessor(Renderer* renderer, FunctionPointer functionPointer );
//	FunctionPointer getFunction() {return (function_);}
//	void setFunction(FunctionPointer fp) {function_ = fp;}
//	void setRenderer(Renderer* r) {renderer_=r;}
//	Renderer* getRenderer() {return renderer_;}
//	void call();
////	void insertFunctionPointer();
//protected:
//	FunctionPointer function_;
//	Renderer* renderer_;
//};
//
//} //namespace voreen