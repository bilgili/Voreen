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

#include "voreen/modules/base/processors/plotting/plotdataexport.h"

#include "voreen/core/plotting/plotdata.h"
#include "voreen/core/plotting/plotcell.h"
#include "voreen/core/plotting/plotrow.h"
#include "voreen/core/plotting/plotfunction.h"

#include "voreen/core/voreenapplication.h"

#include <math.h>
#include <iostream>
#include <fstream>

namespace voreen {

const std::string PlotDataExport::loggerCat_("voreen.PlotDataExport");

PlotDataExport::PlotDataExport():
    Processor()
    , inPort_(Port::INPORT,"PlotData.InPort")
    , outputFile_("outputFile", "Save File", "Write CSV Output file", VoreenApplication::app()->getDataPath(),
        "Comma-Separated Files (*.csv);;Textfile (*.txt);;XML-File (*.xml);;HTML-File (*.htm *.html);;All TextFiles (*.csv *.xml *.txt *.htm *.html)",
        FileDialogProperty::SAVE_FILE,Processor::VALID)
    , separator_("Separator","CSV Separator",";",Processor::VALID)
    , valueBracket_("use inverted comma","Use Inverted Comma (txt and csv)",false,Processor::VALID)
    , pData_(0)
    , pPlotFunction_(0)
    , rewrite_("rewrite","rewrite Export File",Processor::VALID)
{
    outputFile_.onChange(CallMemberAction<PlotDataExport>(this, &PlotDataExport::exportFile));
    rewrite_.onChange(CallMemberAction<PlotDataExport>(this, &PlotDataExport::exportFile));
    addProperty(&separator_);
    addProperty(&valueBracket_);
    addProperty(&outputFile_);
    addProperty(&rewrite_);
    addPort(&inPort_);

    setPropertyGroupGuiName("FileSelection", "File-Selection");
    setPropertyGroupGuiName("FileParams", "File-Parameter");

    outputFile_.setGroupID("FileSelection");
    rewrite_.setGroupID("FileSelection");

    separator_.setGroupID("Fileparams");
    valueBracket_.setGroupID("Fileparams");
}

PlotDataExport::~PlotDataExport() {
    // nothing to do here
}

Processor* PlotDataExport::create() const {
    return new PlotDataExport();
}

std::string PlotDataExport::getProcessorInfo() const {
    return std::string("This Processor export a PlotDataObject to CSV-, txt-, Xml- or html-File.");
}

bool PlotDataExport::isEndProcessor() const {
    return true;
}

bool PlotDataExport::isReady() const {
    return inPort_.isConnected();
}

void PlotDataExport::process() {
    pData_ = 0;
    pPlotFunction_ = 0;
    if (inPort_.hasData()) {
        if (dynamic_cast<PlotData*>(inPort_.getData())) {
            pData_ = dynamic_cast<PlotData*>(inPort_.getData());
            pPlotFunction_ = 0;
        }
        else if (dynamic_cast<PlotFunction*>(inPort_.getData())) {
            pPlotFunction_ = dynamic_cast<PlotFunction*>(inPort_.getData());
            pData_ = 0;
        }
        else {
            LWARNING("PlotDataExport can only handle with PlotData- and PlotFunction-Objects");
        }
    }
}

void PlotDataExport::initialize() throw (VoreenException) {
    Processor::initialize();

}

void PlotDataExport::deinitialize() throw (VoreenException) {
    pData_ = 0;
    pPlotFunction_ = 0;
    Processor::deinitialize();
}

void PlotDataExport::exportFile() {
    if (pData_ || pPlotFunction_) {
        std::string filename = outputFile_.get();
        size_t pos = filename.find_last_of(".");
        std::string extension = filename.substr(pos);
        std::string separator = separator_.get();
        std::ofstream exportfile (filename.c_str());
        bool invertedComma = valueBracket_.get();
        if (extension == ".csv" || extension == ".txt") {
            if (pData_) {
                for (int i = 0; i < pData_->getColumnCount(); ++i) {
                    if (invertedComma)
                        exportfile << "\"" << pData_->getColumnLabel(i) << "\"";
                    else exportfile << pData_->getColumnLabel(i);
                    if (i < pData_->getColumnCount()-1)
                        exportfile << separator;
                    else
                        exportfile << "\n";
                }
                std::vector<PlotCellValue> row;
                for (int i = 0; i < pData_->getRowsCount(); ++i) {
                    row = pData_->getRow(i).getCells();
                    for (size_t j = 0; j < row.size(); ++j) {
                        if (invertedComma)
                            exportfile << "\"";
                        if (row[j].isValue())
                            exportfile << row.at(j).getValue();
                        else
                            exportfile << row.at(j).getTag();
                        if (invertedComma)
                            exportfile << "\"";
                        if (j < row.size()-1)
                            exportfile << separator;
                        else if (i < pData_->getRowsCount()-1)
                            exportfile << "\n";
                    }
                }
            }
            else {
                exportfile << pPlotFunction_->getPlotExpression().getExpressionString();
            }
        }
        else if (extension == ".xml") {
            exportfile << "<?xml version=\"1.0\" ?>\n";
            if (pData_) {
                exportfile << "<plotdata>\n<plotrowvalue>\n";
                for (int i = 0; i < pData_->getColumnCount(); ++i) {
                    exportfile << "<plotcellvalue>" << pData_->getColumnLabel(i);
                    exportfile << "</plotcellvalue>";
                }
                exportfile << "\n</plotrowvalue>\n";
                std::vector<PlotCellValue> row;
                for (int i = 0; i < pData_->getRowsCount(); ++i) {
                    row = pData_->getRow(i).getCells();
                    exportfile << "<plotrowvalue>";
                    for (size_t j = 0; j < row.size(); ++j) {
                        exportfile << "<plotcellvalue>";
                        if (row[j].isValue())
                            exportfile << row.at(j).getValue();
                        else
                            exportfile << row.at(j).getTag();
                        exportfile << "</plotcellvalue>";
                    }
                    exportfile << "</plotrowvalue>\n";
                }
                exportfile << "</plotdata>";
                std::vector<PlotCellImplicit> rowIm;
                if (pData_->getImplicitRowsCount() > 0) {
                    exportfile << "<plotrowimplicit>";
                    for (int i = 0; i < pData_->getColumnCount(); ++i) {
                        exportfile << "<plotcellimplicit>&nbsp;</plotcellimplicit>";
                    }
                    exportfile << "</plotrowimplicit>\n";
                    for (int i = 0; i < pData_->getImplicitRowsCount(); ++i) {
                        rowIm = pData_->getImplicitRow(i).getCells();
                        exportfile << "<plotrowimplicit>\n";
                        for (size_t j = 0; j < 2*rowIm.size(); ++j) {
                            if (j == rowIm.size())
                                exportfile << "</plotrowimplicit>\n<plotrowimplicit>\n";
                            if (rowIm.at(j%rowIm.size()).getAggregationFunction()) {
                                if (j/rowIm.size() == 0)
                                    exportfile << "<plotcellimplicit>" << rowIm.at(j%rowIm.size()).getAggregationFunction()->toString() << "</plotcellimplicit>";
                                else
                                    exportfile << "<plotcellimplicit>" << rowIm.at(j%rowIm.size()).getValue() << "</plotcellimplicit>";
                            }
                            else
                                exportfile << "<plotcellimplicit></plotcellimplicit>";

                        }
                        exportfile << "\n</plotrowimplicit>\n";
                    }
                }
            }
            else {
                exportfile << "<plotfunction>\n<expression>\n" << pPlotFunction_->getPlotExpression().getExpressionString()
                    << "\n</expression>\n<numberofvariables>\n" << pPlotFunction_->getPlotExpression().numberOfVariables()
                    << "\n</numberofvariables>\n</plotfunction>";
            }
        }
        else if (extension == ".htm" || extension == ".html") {
            exportfile << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org"
                << "/TR/html4/strict.dtd\"><html>\n<head><title>";
            if (pData_)
                exportfile << "PlotData";
            else
                exportfile << "PlotFunction";
            exportfile << "</title></head><body>\n<table border=\"2px\">";
            exportfile << "<thead><tr>";
            if (pData_) {
                for (int i = 0; i < pData_->getColumnCount(); ++i) {
                    exportfile << "<td>" << pData_->getColumnLabel(i);
                    exportfile << "</td>";
                }
                exportfile << "</tr></thead>\n";
                std::vector<PlotCellValue> row;
                for (int i = 0; i < pData_->getRowsCount(); ++i) {
                    row = pData_->getRow(i).getCells();
                    exportfile << "<tr>";
                    for (size_t j = 0; j < row.size(); ++j) {
                        exportfile << "<td>";
                        if (row[j].isValue())
                            exportfile << row.at(j).getValue();
                        else
                            exportfile << row.at(j).getTag();
                        exportfile << "</td>";
                    }
                    exportfile << "</tr>\n";
                }
                std::vector<PlotCellImplicit> rowIm;
                if (pData_->getImplicitRowsCount() > 0) {
                    exportfile << "<tr>";
                    for (int i = 0; i < pData_->getColumnCount(); ++i) {
                        exportfile << "<td>&nbsp;</td>";
                    }
                    exportfile << "</tr>\n";
                    for (int i = 0; i < pData_->getImplicitRowsCount(); ++i) {
                        rowIm = pData_->getImplicitRow(i).getCells();
                        exportfile << "<tr>";
                        for (size_t j = 0; j < 2*rowIm.size(); ++j) {
                            if (j == rowIm.size())
                                exportfile << "</tr><tr>";
                            if (rowIm.at(j%rowIm.size()).getAggregationFunction()) {
                                if (j/rowIm.size() == 0)
                                    exportfile << "<td>" << rowIm.at(j%rowIm.size()).getAggregationFunction()->toString() << "</td>";
                                else
                                    exportfile << "<td>" << rowIm.at(j%rowIm.size()).getValue() << "</td>";
                            }
                            else
                                exportfile << "<td></td>";

                        }
                        exportfile << "</tr>\n";
                    }
                }
            }
            else {
                exportfile << "<td>Expression</td><td>Number of Variables</td></tr></head>\n<tr>\n<td>"
                    << pPlotFunction_->getPlotExpression().getExpressionString() << "</td><td>"
                    << pPlotFunction_->getPlotExpression().numberOfVariables() << "</td></tr>\n";
            }
            exportfile << "</table>\n</body>\n</html>";
        }
        exportfile.close();
    }
}


}
