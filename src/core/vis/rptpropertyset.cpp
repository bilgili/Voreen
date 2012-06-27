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

#include "voreen/core/vis/rptpropertyset.h"

namespace voreen{

RptPropertySet::RptPropertySet(){
    props_.clear();
	props_.push_back(0);
}

RptPropertySet::~RptPropertySet(){

}


void RptPropertySet::addRenderWrapper(RptRendererWrapper* rw){
	rendererWrapper_.push_back(rw); 

}

void RptPropertySet::removeRenderWrapper(RptRendererWrapper* rw){

}


void RptPropertySet::addProperty(Property* prop){
	props_.push_back(prop);  
}

void RptPropertySet::removeProperty(Property* prop){
  	//delete last ocurrence
    std::vector<Property*>::iterator index;
    

	//for (std::vector<Property*>::iterator iter=props_.begin();iter!=props_.end();iter++) { 
 //                
 //       if ((props_.at((int)iter))->getIdent() == prop->getIdent()) index = iter;   
	//}

   // props_.erase(index); 
}

Property* RptPropertySet::getProperty(Identifier ident){
    Property* result;   

    for (size_t i=0;i<props_.size();i++){
         if(props_.at(i)->getIdent() == ident) result = props_.at(i);  
    }
    return result;
}

}