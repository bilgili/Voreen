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

#ifndef VRN_RPTPROPERTYLISTWIDGET_H
#define VRN_RPTPROPERTYLISTWIDGET_H

#include <QtGui>

#include "voreen/core/vis/property.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/voreenpainter.h"

#include "rptpropertybutton.h"

namespace voreen {

/**
 * this class was ment for displaying the values of properties that belong to one processor
 * - BoolProp - displayed as a combobox with true / false
 * - EnumProp - displayed as a combobox 
 * - FloatProp - displayed as a doublespinbox
 * - IntProp - displayed as a spinbox
 * - (Int|Float)Vec(2|3|4)Prop - displayed as (2|3|4) spinboxes
 * - ColorProp - displayed with a colored label
 */
class RptPropertyListWidget : public QTableWidget, MessageReceiver {
    Q_OBJECT
public:
    RptPropertyListWidget(QWidget* parent=0);
    ~RptPropertyListWidget();
    
    /**
     * this method can be called to display the properties of the given processor in a table
     */
	void setProcessor(Processor* processor);
    
    /**
     * checks if @param processor is equal to processor that was given by setProcessor, if so then clear table 
     */
    void deselectProcessor(Processor* processor);
    void resizeEvent(QResizeEvent* event);
    
    /**
     * write cell values back into the properties of a given processor
     */
    void exportCellValues();

    virtual void processMessage(Message* msg, const Identifier& dest =Message::all_);

	void setPainter(VoreenPainter* painter) { painter_ = painter; }

	VoreenPainter* getPainter() const { return painter_; }

    QSize sizeHint() const { return QSize(400, 400); }   
  
protected:
	VoreenPainter* painter_;

	/**
	 * When checking if float props have changed, direct comparisons are unreliable, so we check if their difference
	 * is greater than this epsilon value;
	 */
	float epsilon_;

    /**
     * expects a vector of pointer of properties, i.e. processor->getProperties()
     * to build the table
     */
    void insertProperties(std::vector<Property*> propertyList);
   
    /**
     *  returns a QCombobox displaying the value of a boolproperty
     */
    QComboBox* getBoolPropertyWidget(bool startValue);
   
    /**
     *  returns a QSpinbox displaying the value of an intproperty
     */
    QSpinBox*  getIntPropertyWidget(int min, int max, int startValue);

    /**
     * returns a QDoubleSpinbox displaying the value of a floatproperty
     */
    QDoubleSpinBox*  getFloatPropertyWidget(float min, float max, float startValue);
    
    
    QComboBox* getEnumPropertyWidget(std::vector<std::string> descr, int startvalue);
    
    
    std::vector<QWidget*> getIVec2PropertyWidget(tgt::ivec2 val,tgt::ivec2 min,tgt::ivec2 max);
    std::vector<QWidget*> getIVec3PropertyWidget(tgt::ivec3 val,tgt::ivec3 min,tgt::ivec3 max);
    std::vector<QWidget*> getIVec4PropertyWidget(tgt::ivec4 val,tgt::ivec4 min,tgt::ivec4 max);
    std::vector<QWidget*> getFVec2PropertyWidget(tgt::vec2 val,tgt::vec2 min,tgt::vec2 max);
    std::vector<QWidget*> getFVec3PropertyWidget(tgt::vec3 val,tgt::vec3 min,tgt::vec3 max);
    std::vector<QWidget*> getFVec4PropertyWidget(tgt::vec4 val,tgt::vec4 min,tgt::vec4 max);
	QDialogButtonBox*	getStringPropertyWidget(StringProp* prop);
	QDialogButtonBox*	getStringVectorPropertyWidget(StringVectorProp* prop);

    std::vector<Property*> propertyList_;

    std::vector<std::vector<QWidget*> > vectorWidgetList_; 
    QComboBox* proxyGeometryBox_;
	Processor* processor_;
	std::string networkPath_;

protected slots:
    void widgetChanged(int);
    void widgetChanged(double);
	void propertyButtonClicked(Property* prop);
	void stringPropButtonPushed(StringProp* prop);
	void stringVectorPropButtonPushed(StringVectorProp* prop);
};

//---------------------------------------------------------------------------

/**
 * this class displays the values of a color property, called by insertProperties() of RptPropertyListWidget
 */
class ColorPropertyWidget : public QWidget {
Q_OBJECT

public:
    ColorPropertyWidget(QWidget* parent=0, tgt::Color color = tgt::Color(1));
    tgt::vec4 getSelectedColor();

protected:
    QHBoxLayout* layout_;
    QLabel* colorLbl_;
    QPushButton* colorBt_;
    tgt::Color curColor_;
    RptPropertyListWidget* rptList_;

protected slots:
    void clickedColorBt();
};

//---------------------------------------------------------------------------

/**
 * FIXME: this class is for FileDialogProperties, because VoreenVE does not use voreen's widget generator yet.
 * It has to be removed when new property system is created (FW)
 */
class FileDialogPropertyWidget : public QWidget {
    Q_OBJECT
public:
    /**
     *Creates a new AGButtonWidget - the button send a (dummy) bool msg
     */
    FileDialogPropertyWidget(QWidget* parent = 0, FileDialogProp* prop = 0);

public slots:
    /**
     * SLOT-method of pushbutton - opens file dialog
     */
    void clicked();

private:
    FileDialogProp* myProp_;
    QPushButton* openFileDialogBtn_;

    std::string dialogCaption_;
    std::string directory_;
    std::string fileFilter_;
    QGroupBox* groupBox_; // equals zero if Plugin is not FrameControler
};

} //namespace voreen

#endif //VRN_RPTPROPERTYLISTWIDGET_H
