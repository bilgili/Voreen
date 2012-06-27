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

#include "voreen/qt/widgets/widgetgeneratorplugins.h"

#include "voreen/core/vis/message.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"

#include <QFrame>
#include <QLabel>
#include <QPalette>
#include <QPainter>
#include <QListWidget>


namespace voreen {

ProcessorHeaderWidget::ProcessorHeaderWidget(QWidget *parent, WidgetGenerator::ProcessorPropsList* actRenderer,
                                   MessageReceiver* msgReceiver, WidgetGenerator* widgetGen, bool showCheckBox): QWidget(parent)
{
    widgetGen_ = widgetGen;
    msgReceiver_ = msgReceiver;
    actRenderer_ = actRenderer;
    setObjectName(QString::fromUtf8("ProcessorTitleWidget"));

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);

	QFrame* rendFrame = new QFrame();
	QHBoxLayout* frameLayout = new QHBoxLayout(rendFrame);
	frameLayout->setSpacing(0);
	frameLayout->setMargin(0);
	rendFrame->setFrameShape(QFrame::StyledPanel);

	expandButton_ = new QPushButton();
	expandButton_->setMinimumWidth(20);
	expandButton_->setMaximumWidth(20);
    if (showCheckBox) {
        expandButton_->setIcon(QIcon(":/icons/expand0.png"));
		connect(expandButton_, SIGNAL(clicked(void)), this, SLOT(setVisRender(void)));
	}
	expandButton_->setFlat(true);
	frameLayout->addWidget(expandButton_);

	QLabel* rendName = new QLabel();
	rendName->setText(tr(actRenderer->processor->getName().c_str()));
	frameLayout->addWidget(rendName);

	mainLayout->addWidget(rendFrame);
}

void ProcessorHeaderWidget::updateState() {
    if (actRenderer_->state)
        expandButton_->setIcon(QIcon(":/icons/expand1.png"));
    else
        expandButton_->setIcon(QIcon(":/icons/expand0.png"));
}

void ProcessorHeaderWidget::setVisRender(){
    actRenderer_->state = !actRenderer_->state;
	updateState();
    widgetGen_->stateChanged();
}

//---------------------------------------------------------------------


OverlayProcessorHeaderWidget::OverlayProcessorHeaderWidget(QWidget *parent, WidgetGenerator::OverlayPropsList* actOverlay, MessageReceiver* msgReceiver, WidgetGenerator* widgetGen, bool showCheckBox)
    : QWidget(parent)
    , widgetGen_(widgetGen)
    , msgReceiver_(msgReceiver)
    , actOverlay_(actOverlay)
{
    setObjectName(QString::fromUtf8("Activation-Plugin for Overlays"));

    QFont font;
    font.setBold(true);
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

    if (showCheckBox) {
        QPushButton* btnButton = new QPushButton();
        btnButton->setText(tr(actOverlay->overlay->getName().c_str()));
        btnButton->setFont(font);
        connect( btnButton , SIGNAL( clicked(void) ) , this , SLOT( setVisOverlay(void) ));
        hboxLayout->addWidget(btnButton);
    }
    else {
        QLabel* label = new QLabel(tr(actOverlay->overlay->getName().c_str()));
        label->setFont(font);
        hboxLayout->addWidget(label);
    }
}

void OverlayProcessorHeaderWidget::setVisOverlay() {
    actOverlay_->state = !actOverlay_->state;
    widgetGen_->stateChanged();
}

//---------------------------------------------------------------------

AGFloatWidget::AGFloatWidget(QWidget* parent, MessageReceiver* msgReceiver, FloatProp* prop) : QWidget(parent), TemplatePlugin<float>() {
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    sliderValue_ = 0;
    spinValue_ = 0;
    minimum_ = prop->getMinValue();
    maximum_ = prop->getMaxValue();
    setObjectName(QString::fromUtf8("Float-Slider"));

	QHBoxLayout* hboxLayout = new QHBoxLayout(this);
	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
    hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));

    slider_ = new QSlider(Qt::Horizontal);
    slider_->setTracking(prop->getInstantValueChangeState());
    hboxLayout->addWidget(slider_);

    spinBox_ = new QDoubleSpinBox();
    spinBox_->setDecimals(prop->getDecimals());
    hboxLayout->addWidget(spinBox_);

    setRange();
    changeValue(prop->get());

    connect( slider_ , SIGNAL( sliderMoved(int) ) , this , SLOT( sliderChanged(int) ));
    connect( slider_ , SIGNAL( valueChanged(int) ) , this , SLOT( sliderDragged(int) ));
    connect( slider_ , SIGNAL( sliderReleased() ) , this , SLOT( released() ));
    connect( spinBox_, SIGNAL( valueChanged(double) ), this, SLOT( spinboxChanged(double) ) );
    myProp_ = prop;
    groupBox_ = 0;
}

void AGFloatWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGFloatWidget::setVisibleState(bool vis) {
    setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}


void AGFloatWidget::sliderDragged(int value) {
    sliderChanged(value);
    if (!slider_->hasTracking())
        sendMessage();
}

void AGFloatWidget::initWidgetFromLoadedSettings(float min, float max, float value) {
    minimum_ = min;
    maximum_ = max;
    setRange();
    changeValue(value);
    //send message regardless of visibility
    if (myProp_->getAutoChange())
        myProp_->set(spinValue_);
    else
        msgReceiver_->postMessage(new FloatMsg(msgIdent_, spinValue_), myProp_->getMsgDestination());
}

void AGFloatWidget::setRange(){
    slider_->setRange(1, 100);
    spinBox_->setRange(minimum_, maximum_);
    //calculate stepvalue so that 100 steps are between minimum_ and maximum_
    spinBox_->setSingleStep((maximum_-minimum_)/100.0f);
    if (minimum_ >= 0 && minimum_ < 1 && maximum_ < 1) {
        int decimals = 1;
        //calculate number of decimals in min or in max if min is 0
        float tmp = (minimum_ == 0) ? maximum_ : minimum_;
        while (tmp < 1) {
            tmp = tmp*10.0f;
            decimals++;
        }
        spinBox_->setDecimals(decimals);
    }
}

void AGFloatWidget::sendMessage() {
    //don't send any message if widget is not visible
    //this avoids "got repaint while disallowed"-Message
    if (!isVisible())
        return;
    if (myProp_->getAutoChange())
        myProp_->set(spinValue_);
    else
        msgReceiver_->postMessage(new FloatMsg(msgIdent_, spinValue_), myProp_->getMsgDestination());
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void AGFloatWidget::spinboxChanged(double value) {
    if ((spinValue_ != value) && (value >= minimum_)) {
        int i;
        double m;
        m = 99.0f/(maximum_ - minimum_);
        i = static_cast<int>(value*m+100-m*maximum_);
        sliderValue_ = i;
        slider_->setSliderPosition(i);
        spinValue_ = value;
        sendMessage();
    }
    if (value < minimum_) {
        //avoid emit of spinboxchanged-slot - floatmessage is send multiple times or
        //circular call of slot can occur
        disconnect( spinBox_, SIGNAL( valueChanged(double) ), this, SLOT( spinboxChanged(double) ) );
        spinBox_->setValue(minimum_);
        connect( spinBox_, SIGNAL( valueChanged(double) ), this, SLOT( spinboxChanged(double) ) );
    }
}
void AGFloatWidget::released() {
    if (!slider_->hasTracking())
        sendMessage();
}

void AGFloatWidget::changeValue(float f) {
    int i;
    double m;
    m = 99.0f/(maximum_ - minimum_);
    i = static_cast<int>(f*m+100-m*maximum_);
    sliderValue_ = i;
    spinValue_ = f;
    slider_->setSliderPosition(i);
    //avoid emit of spinboxchanged-slot - floatmessage is send multiple times or
    //circular call of slot can occur
    spinBox_->blockSignals(true);
    spinBox_->setValue(f);
    spinBox_->blockSignals(false);
}

void AGFloatWidget::sliderChanged(int value) {
    if (value != sliderValue_) {
        float fl;
        double m;
        m = (maximum_ - minimum_)/99.0f;
        fl = static_cast<float>(value)*m+maximum_-m*100;
        sliderValue_ = value;
        spinValue_ = fl;
        //avoid emit of spinboxchanged-slot - floatmessage is send multiple times or
        //circular call of slot can occur
        disconnect( spinBox_, SIGNAL( valueChanged(double) ), this, SLOT( spinboxChanged(double) ) );
        spinBox_->setValue(fl);
        connect( spinBox_, SIGNAL( valueChanged(double) ), this, SLOT( spinboxChanged(double) ) );
        if (slider_->hasTracking())
            sendMessage();
    }
}

//---------------------------------------------------------------------

AGIntWidget::AGIntWidget(QWidget* parent, MessageReceiver* msgReceiver, IntProp* prop) : QWidget(parent), TemplatePlugin<int>() {
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    value_ = 0;
    setObjectName(QString::fromUtf8("Int-Slider"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));

    slider_ = new QSlider(Qt::Horizontal);
    slider_->setTracking(prop->getInstantValueChangeState());
    hboxLayout->addWidget(slider_);

    spinBox_ = new QSpinBox();
    hboxLayout->addWidget(spinBox_);

    setRange(prop->getMinValue(), prop->getMaxValue());
    changeValue(prop->get());

    connect( slider_ , SIGNAL( sliderMoved(int) ) , this , SLOT( valChanged(int) ));
    connect( slider_ , SIGNAL( valueChanged(int) ) , this , SLOT( sliderDragged(int) ));
    connect( slider_ , SIGNAL( sliderReleased() ) , this , SLOT( released() ));
    connect( spinBox_, SIGNAL( valueChanged(int) ), this, SLOT( valChanged(int) ) );
    myProp_ = prop;
    groupBox_ = 0;
}

void AGIntWidget::initWidgetFromLoadedSettings(int min, int max, int value) {
    setRange(min, max);
    changeValue(value);
    //send message regardless of visibility
    if (myProp_->getAutoChange())
        myProp_->set(value_);
    else
        msgReceiver_->postMessage(new IntMsg(msgIdent_, value_), myProp_->getMsgDestination());
}

void AGIntWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGIntWidget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGIntWidget::setRange(int min, int max) {
    slider_->setRange(min, max);
    slider_->setPageStep(static_cast<int>(max/10.f));
    spinBox_->setRange(min, max);
}

void AGIntWidget::changeValue(int i) {
    if (i != value_) {
        value_ = i;
        slider_->setSliderPosition(value_);
        spinBox_->blockSignals(true);
        spinBox_->setValue(value_);
        spinBox_->blockSignals(false);
    }
}

void AGIntWidget::sendMessage() {
    //don't send any message if widget is not visible
    //this avoids "got repaint while disallowed"-Message
    if (!isVisible())
        return;
    if (myProp_->getAutoChange())
        myProp_->set(value_);
    else
        msgReceiver_->postMessage(new IntMsg(msgIdent_, value_), myProp_->getMsgDestination());
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void AGIntWidget::sliderDragged(int value) {
    valChanged(value);
    if (!slider_->hasTracking())
        sendMessage();
}

void AGIntWidget::released() {
    if (!slider_->hasTracking())
        sendMessage();
}

void AGIntWidget::valChanged(int value) {
    if (value != value_) {
        value_ = value;
        bool spinboxchange = (spinBox_->value() == value);
        slider_->setSliderPosition(value);
        spinBox_->setValue(value);
        if (spinboxchange || slider_->hasTracking())
            sendMessage();
    }
}


//---------------------------------------------------------------------

AGBoolWidget::AGBoolWidget(QWidget* parent, MessageReceiver* msgReceiver, BoolProp* prop) : QWidget(parent), TemplatePlugin<bool>() {
    setObjectName(QString::fromUtf8("Bool-Checkbox"));
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);
	chBox_ = new QCheckBox(tr(prop->getGuiText().c_str()));
	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
    hboxLayout->addWidget(chBox_);
    chBox_->setChecked(prop->get());

    connect( chBox_ , SIGNAL( toggled(bool) ) , this , SLOT( valChanged(bool) ));
    myProp_ = prop;
    groupBox_ = 0;
}

void AGBoolWidget::initWidgetFromLoadedSettings(bool value) {
    chBox_->setChecked(value);
    if (myProp_->getAutoChange())
        myProp_->set(value);
    else
        msgReceiver_->postMessage(new BoolMsg(msgIdent_, value), myProp_->getMsgDestination());
}

void AGBoolWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGBoolWidget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGBoolWidget::valChanged(bool value) {
    if (myProp_->getAutoChange())
        myProp_->set(value);
    else
        msgReceiver_->postMessage(new BoolMsg(msgIdent_, value), myProp_->getMsgDestination());

    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}


void AGBoolWidget::changeValue(bool b) {
    if (!isVisible()) {
        chBox_->blockSignals(true);
        chBox_->setChecked(b);
        chBox_->blockSignals(false);
    }
    else
        chBox_->setChecked(b);
}

//---------------------------------------------------------------------

AGColorWidget::AGColorWidget(QWidget* parent, MessageReceiver* msgReceiver, ColorProp* prop) : QWidget(parent), TemplatePlugin<tgt::vec4>() {
    setObjectName(QString::fromUtf8("Color-Dialog"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);
	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();

	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));

    btnCol_ = new QPushButton();
    btnCol_->setText(tr("Change"));

    myLabel_ = new QLabel();

    myLabel_->setText(tr(""));

    myLabel_->setAutoFillBackground(true);

    QPalette newPalette = myLabel_->palette();
    newPalette.setColor(QPalette::Window, QColor(255,0,0,255) );
    myLabel_->setPalette(newPalette);
    myLabel_->setBackgroundRole(newPalette.Window);

	btnCol_->setPalette(newPalette);
    btnCol_->setBackgroundRole(newPalette.Window);
    changeValue(prop->get());

    hboxLayout->addWidget(myLabel_);
    hboxLayout->addWidget(btnCol_);

    connect( btnCol_ , SIGNAL( clicked(void) ) , this , SLOT( clicked(void) ));
    myProp_ = prop;
    groupBox_ = 0;
}

void AGColorWidget::initWidgetFromLoadedSettings(tgt::Color value) {
    changeValue(value);
    if (myProp_->getAutoChange())
        myProp_->set(curColor_);
    else
        msgReceiver_->postMessage(new ColorMsg(msgIdent_, curColor_), myProp_->getMsgDestination());
}

void AGColorWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGColorWidget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGColorWidget::changeValue(tgt::Color val) {
    QPalette newPalette = myLabel_->palette();
    newPalette.setColor(QPalette::Window, QColor(static_cast<int>(val.r*255), static_cast<int>(val.g*255), static_cast<int>(val.b*255)) );
    btnCol_->setPalette(newPalette);
    myLabel_->setPalette(newPalette);
    curColor_ = val;
}

void AGColorWidget::sendMessage() {
    if (myProp_->getAutoChange())
        myProp_->set(curColor_);
    else
        msgReceiver_->postMessage(new ColorMsg(msgIdent_, curColor_), myProp_->getMsgDestination());

    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}


void AGColorWidget::clicked() {
    QColor myColor;
    myColor.setRgba(QColorDialog::getRgba(QColor(static_cast<int>(curColor_.r*255), static_cast<int>(curColor_.g*255), static_cast<int>(curColor_.b*255),
                                                 static_cast<int>(curColor_.a*255)).rgba()));
    if (myColor != QColor(static_cast<int>(curColor_.r*255), static_cast<int>(curColor_.g*255), static_cast<int>(curColor_.b*255), static_cast<int>(curColor_.a*255)).rgba()) {
        QPalette newPalette = myLabel_->palette();
        newPalette.setColor(QPalette::Window, myColor );
        myLabel_->setPalette(newPalette);
        myLabel_->setBackgroundRole(newPalette.Window);
        curColor_ = tgt::Color(myColor.redF(),myColor.greenF(),myColor.blueF(), myColor.alphaF());
        sendMessage();
    }
}

//---------------------------------------------------------------------

AGEnumWidget::AGEnumWidget(QWidget* parent, MessageReceiver* msgReceiver, EnumProp* prop)
    : QWidget(parent)
    , TemplatePlugin<int>(),
    myProp_(prop)
{
    setObjectName(QString::fromUtf8("Combobox"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);
    hboxLayout->setSpacing(1);
    hboxLayout->setMargin(1);
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    sendStringMsg_ = prop->getSendStringMsg();
    //create new combobox and add it to the layout
    hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
    box_ = new QComboBox();
    setStrings(prop->getStrings());
    hboxLayout->addWidget(box_);
    //set valChanged as SLOT for currentIndexChanged-Event of combobox
    connect( box_ , SIGNAL( currentIndexChanged(int) ) , this , SLOT( valChanged(int) ));
    groupBox_ = 0;
}

void AGEnumWidget::initWidgetFromLoadedSettings(int index) {
    box_->blockSignals(true);
    box_->setCurrentIndex(index);
    box_->blockSignals(false);
    myProp_->set(index);
    if (sendStringMsg_)
        msgReceiver_->postMessage(new StringMsg(msgIdent_, strings_.at(index)), myProp_->getMsgDestination());
    else
        msgReceiver_->postMessage(new IntMsg(msgIdent_, index), myProp_->getMsgDestination());
}

void AGEnumWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGEnumWidget::setVisibleState(bool vis) {
    setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGEnumWidget::setStrings(const std::vector<std::string>& str) {
    box_->clear();
    strings_ = str;
    for (size_t i = 0; i < strings_.size(); i++)
        box_->addItem((QString) strings_[i].c_str());

    if (myProp_ != 0)
        box_->setCurrentIndex(myProp_->get());
}

void AGEnumWidget::valChanged(int value) {
    myProp_->set(value);
    if (sendStringMsg_)
		// fixme: replaced msgReceiver with MsgDistr (used in voreenreg)
		//msgReceiver_->...
        MsgDistr.postMessage(new StringMsg(msgIdent_, strings_.at(value)), myProp_->getMsgDestination());
    else
        msgReceiver_->postMessage(new IntMsg(msgIdent_, value), myProp_->getMsgDestination());

    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void AGEnumWidget::changeValue(int index) {
    box_->setCurrentIndex(index);
}

void AGEnumWidget::updateFromProperty() {
    box_->blockSignals(true);
    setStrings(myProp_->getStrings());
    box_->blockSignals(false);
//    repaint();
}

//---------------------------------------------------------------------

AGButtonWidget::AGButtonWidget(QWidget* parent, MessageReceiver* msgReceiver, ButtonProp* prop)
    : QWidget(parent)
    , TemplatePlugin< bool >()
{
    setObjectName(QString::fromUtf8("Button"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);
	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    //create new button and add it to the layout
	btnButton_ = new QPushButton();
    btnButton_->setText(tr(prop->getButtonText().c_str()));

    //set icon for the button
    std::string filename = prop->getIconFilename();
    if (filename != "")
        btnButton_->setIcon(QIcon(QString(filename.c_str())));

    hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
    hboxLayout->addWidget(btnButton_);
    //set valChanged as SLOT for button
    connect( btnButton_ , SIGNAL( clicked(void) ) , this , SLOT( clicked(void) ));
    myProp_ = prop;
    groupBox_ = 0;
}

void AGButtonWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGButtonWidget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGButtonWidget::clicked() {
    msgReceiver_->postMessage(new BoolMsg(msgIdent_, true), myProp_->getMsgDestination());
	msgReceiver_->postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void AGButtonWidget::changeValue() {
}

//---------------------------------------------------------------------

AGFileDialogWidget::AGFileDialogWidget(QWidget* parent, MessageReceiver* msgReceiver, FileDialogProp* prop)
    : QWidget(parent)
    , TemplatePlugin< std::string > ()
{
    setObjectName(QString::fromUtf8("file-Dialog"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);
	hboxLayout->setSpacing(1);
	hboxLayout->setMargin(1);
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
    parWidget_ = parent;

    openFileDialogBtn_ = new QPushButton();
    openFileDialogBtn_->setText(tr("open File-Dialog"));

	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
    hboxLayout->addWidget(openFileDialogBtn_);

    connect( openFileDialogBtn_ , SIGNAL( clicked(void) ) , this , SLOT( clicked(void) ));

    dialogCaption_ = prop->getDialogCaption();
    directory_ = prop->getDirectory();
    fileFilter_ = prop->getFileFilter();
    myProp_ = prop;
    groupBox_ = 0;
}

void AGFileDialogWidget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGFileDialogWidget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGFileDialogWidget::changeValue() {
}

void AGFileDialogWidget::clicked() {
    QString filename = QString(QFileDialog::getOpenFileName(parWidget_, dialogCaption_.c_str(), directory_.c_str() , fileFilter_.c_str()));
    if (myProp_->getAutoChange())
        myProp_->set(filename.toStdString());
    else
        msgReceiver_->postMessage(new StringMsg(msgIdent_, filename.toStdString()), myProp_->getMsgDestination());
}

//---------------------------------------------------------------------

AGIntVec2Widget::AGIntVec2Widget(QWidget* parent, MessageReceiver* msgReceiver, IntVec2Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::ivec2>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::ivec2(0);
    setObjectName(QString::fromUtf8("IntVec2-Spinboxes"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	hboxLayout->setSpacing(5);
	hboxLayout->setMargin(1);
	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout->addStretch();

	hboxLayout->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QSpinBox();
    hboxLayout->addWidget(spinBox_x);

	hboxLayout->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QSpinBox();
    hboxLayout->addWidget(spinBox_y);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
    myProp_ = prop;
    groupBox_ = 0;
}

void AGIntVec2Widget::initWidgetFromLoadedSettings(tgt::ivec2 min, tgt::ivec2 max, tgt::ivec2 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
    value_ = value;
    if (myProp_->getAutoChange())
        myProp_->set(value_);
    else
        msgReceiver_->postMessage(new IVec2Msg(msgIdent_, value_), myProp_->getMsgDestination());
}

void AGIntVec2Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGIntVec2Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGIntVec2Widget::setRange(tgt::ivec2 min, tgt::ivec2 max) {
    spinBox_x->setRange(min.x, max.x);
	spinBox_y->setRange(min.y, max.y);
}

void AGIntVec2Widget::changeValue(tgt::ivec2 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
        connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
    }
}

void AGIntVec2Widget::sendMessage() {
    if (isVisible()) {
        if (myProp_->getAutoChange())
            myProp_->set(value_);
        else
            msgReceiver_->postMessage(new IVec2Msg(msgIdent_, value_), myProp_->getMsgDestination());

        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
}

void AGIntVec2Widget::valChangedX(int x) {
    tgt::ivec2 tmp = tgt::ivec2(x, spinBox_y->value());
    if (tmp != value_) {
        value_ = tmp;
        spinBox_x->setValue(x);
        sendMessage();
    }
}

void AGIntVec2Widget::valChangedY(int y) {
	tgt::ivec2 tmp = tgt::ivec2(spinBox_x->value(), y);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}


//---------------------------------------------------------------------

AGIntVec3Widget::AGIntVec3Widget(QWidget* parent, MessageReceiver* msgReceiver, IntVec3Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::ivec3>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::ivec3(0);
	prop_ = prop;
    setObjectName(QString::fromUtf8("IntVec3-Spinboxes"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	hboxLayout->setSpacing(5);
	hboxLayout->setMargin(1);
	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout->addStretch();

	hboxLayout->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QSpinBox();
    hboxLayout->addWidget(spinBox_x);

	hboxLayout->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QSpinBox();
    hboxLayout->addWidget(spinBox_y);

	hboxLayout->addWidget(new QLabel(tr("z:")));
    spinBox_z = new QSpinBox();
    hboxLayout->addWidget(spinBox_z);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	connect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
    groupBox_ = 0;
}

void AGIntVec3Widget::initWidgetFromLoadedSettings(tgt::ivec3 min, tgt::ivec3 max, tgt::ivec3 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    spinBox_z->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    spinBox_z->setValue(value.z);
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
    spinBox_z->blockSignals(false);
    value_ = value;
    if (prop_->getAutoChange())
        prop_->set(value_);
    else
        msgReceiver_->postMessage(new IVec3Msg(msgIdent_, value_), prop_->getMsgDestination());
}

void AGIntVec3Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGIntVec3Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGIntVec3Widget::setRange(tgt::ivec3 min, tgt::ivec3 max) {
    spinBox_x->setRange(min.x, max.x);
	spinBox_y->setRange(min.y, max.y);
	spinBox_z->setRange(min.z, max.z);
}

void AGIntVec3Widget::changeValue(tgt::ivec3 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	    disconnect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
		spinBox_z->setValue(value_.z);
        connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	    connect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
	}
}

void AGIntVec3Widget::sendMessage() {
	if (isVisible()) {
		if (prop_->getAutoChange())
			prop_->set(value_);
		else
			msgReceiver_->postMessage(new IVec3Msg(msgIdent_, value_), prop_->getMsgDestination());

        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
	}
}

void AGIntVec3Widget::valChangedX(int x) {
	tgt::ivec3 tmp = tgt::ivec3(x, spinBox_y->value(), spinBox_z->value());
	if (tmp != value_) {
		value_ = tmp;
		spinBox_x->setValue(x);
		sendMessage();
	}
}

void AGIntVec3Widget::valChangedY(int y) {
	tgt::ivec3 tmp = tgt::ivec3(spinBox_x->value(), y, spinBox_z->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}

void AGIntVec3Widget::valChangedZ(int z) {
	tgt::ivec3 tmp = tgt::ivec3(spinBox_x->value(), spinBox_y->value(), z);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_z->setValue(z);
		sendMessage();
    }
}

//---------------------------------------------------------------------

AGIntVec4Widget::AGIntVec4Widget(QWidget* parent, MessageReceiver* msgReceiver, IntVec4Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::ivec4>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::ivec4(0);
	prop_ = prop;
    setObjectName(QString::fromUtf8("IntVec4-Spinboxes"));

	QWidget* top = new QWidget(this);
	QWidget* bottom = new QWidget(this);

    QHBoxLayout* hboxLayout_top = new QHBoxLayout(top);
	QHBoxLayout* hboxLayout_bottom = new QHBoxLayout(bottom);
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);

	vboxLayout->setSpacing(2);
	vboxLayout->setMargin(0);
	hboxLayout_top->setSpacing(5);
	hboxLayout_top->setMargin(1);
	hboxLayout_bottom->setSpacing(5);
	hboxLayout_bottom->setMargin(1);
	hboxLayout_top->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout_top->addStretch();

	hboxLayout_top->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QSpinBox();
    hboxLayout_top->addWidget(spinBox_x);

	hboxLayout_top->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QSpinBox();
    hboxLayout_top->addWidget(spinBox_y);

	hboxLayout_bottom->addStretch();
	hboxLayout_bottom->addWidget(new QLabel(tr("z:")));
    spinBox_z = new QSpinBox();
    hboxLayout_bottom->addWidget(spinBox_z);

	hboxLayout_bottom->addWidget(new QLabel(tr("w:")));
    spinBox_w = new QSpinBox();
    hboxLayout_bottom->addWidget(spinBox_w);
	vboxLayout->addWidget(top);
	vboxLayout->addWidget(bottom);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	connect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
	connect( spinBox_w, SIGNAL( valueChanged(int) ), this, SLOT( valChangedW(int) ) );
    groupBox_ = 0;
}

void AGIntVec4Widget::initWidgetFromLoadedSettings(tgt::ivec4 min, tgt::ivec4 max, tgt::ivec4 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    spinBox_z->blockSignals(true);
    spinBox_w->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    spinBox_z->setValue(value.z);
    spinBox_w->setValue(value.w);
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
    spinBox_z->blockSignals(false);
    spinBox_w->blockSignals(false);
    value_ = value;
    if (prop_->getAutoChange())
        prop_->set(value_);
    else
        msgReceiver_->postMessage(new IVec4Msg(msgIdent_, value_), prop_->getMsgDestination());
}

void AGIntVec4Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGIntVec4Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

void AGIntVec4Widget::setRange(tgt::ivec4 min, tgt::ivec4 max) {
    spinBox_x->setRange(min.x, max.x);
	spinBox_y->setRange(min.y, max.y);
	spinBox_z->setRange(min.z, max.z);
	spinBox_w->setRange(min.w, max.w);
}

void AGIntVec4Widget::changeValue(tgt::ivec4 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	    disconnect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
	    disconnect( spinBox_w, SIGNAL( valueChanged(int) ), this, SLOT( valChangedW(int) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
		spinBox_z->setValue(value_.z);
		spinBox_w->setValue(value_.w);
        connect( spinBox_x, SIGNAL( valueChanged(int) ), this, SLOT( valChangedX(int) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(int) ), this, SLOT( valChangedY(int) ) );
	    connect( spinBox_z, SIGNAL( valueChanged(int) ), this, SLOT( valChangedZ(int) ) );
	    connect( spinBox_w, SIGNAL( valueChanged(int) ), this, SLOT( valChangedW(int) ) );
	}
}

void AGIntVec4Widget::sendMessage() {
    if (isVisible()) {
        if (prop_->getAutoChange())
            prop_->set(value_);
        else
            msgReceiver_->postMessage(new IVec4Msg(msgIdent_, value_), prop_->getMsgDestination());

        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
}

void AGIntVec4Widget::valChangedX(int x) {
	tgt::ivec4 tmp = tgt::ivec4(x, spinBox_y->value(), spinBox_z->value(), spinBox_w->value());
	if (tmp != value_) {
		value_ = tmp;
		spinBox_x->setValue(x);
		sendMessage();
	}
}

void AGIntVec4Widget::valChangedY(int y) {
	tgt::ivec4 tmp = tgt::ivec4(spinBox_x->value(), y, spinBox_z->value(), spinBox_w->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}

void AGIntVec4Widget::valChangedZ(int z) {
	tgt::ivec4 tmp = tgt::ivec4(spinBox_x->value(), spinBox_y->value(), z, spinBox_w->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_z->setValue(z);
		sendMessage();
    }
}

void AGIntVec4Widget::valChangedW(int w) {
	tgt::ivec4 tmp = tgt::ivec4(spinBox_x->value(), spinBox_y->value(), spinBox_z->value(), w);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_w->setValue(w);
		sendMessage();
    }
}

//---------------------------------------------------------------------

AGFloatVec2Widget::AGFloatVec2Widget(QWidget* parent, MessageReceiver* msgReceiver, FloatVec2Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::vec2>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::vec2(0.0f);
	prop_ = prop;
    setObjectName(QString::fromUtf8("FloatVec2-Spinboxes"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	hboxLayout->setSpacing(5);
	hboxLayout->setMargin(1);
	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout->addStretch();

	hboxLayout->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QDoubleSpinBox();
    hboxLayout->addWidget(spinBox_x);

	hboxLayout->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QDoubleSpinBox();
    hboxLayout->addWidget(spinBox_y);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
    groupBox_ = 0;
}

void AGFloatVec2Widget::initWidgetFromLoadedSettings(tgt::vec2 min, tgt::vec2 max, tgt::vec2 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    value_ = value;
    if (prop_->getAutoChange())
        prop_->set(value_);
    else
        msgReceiver_->postMessage(new Vec2Msg(msgIdent_, value_), prop_->getMsgDestination());
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
}

void AGFloatVec2Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGFloatVec2Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

int AGFloatVec2Widget::calculateDecimals(float min, float max) {
	int decimals = 2;
    if (min >= 0 && min < 1 && max < 1) {
        //calculate number of decimals in min or in max if min is 0
        float tmp = (min == 0) ? max : min;
        while (tmp < 1) {
            tmp = tmp*10.0f;
            decimals++;
        }
	}
	return decimals;
}

void AGFloatVec2Widget::setRange(tgt::vec2 min, tgt::vec2 max) {
	spinBox_x->setRange(min.x, max.x);
	int decimals = calculateDecimals(min.x, max.x);
	if (decimals > 2)
		spinBox_x->setDecimals(decimals);
	
    spinBox_x->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_y->setRange(min.y, max.y);
	decimals = calculateDecimals(min.y, max.y);
	if (decimals > 2)
		spinBox_y->setDecimals(decimals);
	
    spinBox_y->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

}

void AGFloatVec2Widget::changeValue(tgt::vec2 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
        connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
    }
}

void AGFloatVec2Widget::sendMessage() {
	if (isVisible()) {
		if (prop_->getAutoChange())
			prop_->set(value_);
		else
			msgReceiver_->postMessage(new Vec2Msg(msgIdent_, value_), prop_->getMsgDestination());
		
		MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
	}
}

void AGFloatVec2Widget::valChangedX(double x) {
	tgt::vec2 tmp = tgt::vec2(x, spinBox_y->value());
	if (tmp != value_) {
		value_ = tmp;
		spinBox_x->setValue(x);
		sendMessage();
	}
}

void AGFloatVec2Widget::valChangedY(double y) {
	tgt::vec2 tmp = tgt::vec2(spinBox_x->value(), y);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}

//---------------------------------------------------------------------

AGFloatVec3Widget::AGFloatVec3Widget(QWidget* parent, MessageReceiver* msgReceiver, FloatVec3Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::vec3>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::vec3(0.0f);
	prop_ = prop;
    setObjectName(QString::fromUtf8("FloatVec3-Spinboxes"));
    QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	hboxLayout->setSpacing(5);
	hboxLayout->setMargin(1);
	hboxLayout->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout->addStretch();

	hboxLayout->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QDoubleSpinBox();
    hboxLayout->addWidget(spinBox_x);

	hboxLayout->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QDoubleSpinBox();
    hboxLayout->addWidget(spinBox_y);

	hboxLayout->addWidget(new QLabel(tr("z:")));
    spinBox_z = new QDoubleSpinBox();
    hboxLayout->addWidget(spinBox_z);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	connect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
    groupBox_ = 0;

}

void AGFloatVec3Widget::initWidgetFromLoadedSettings(tgt::vec3 min, tgt::vec3 max, tgt::vec3 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    spinBox_z->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    spinBox_z->setValue(value.z);
    value_ = value;
    if (prop_->getAutoChange())
        prop_->set(value);
    else
        msgReceiver_->postMessage(new Vec3Msg(msgIdent_, value), prop_->getMsgDestination());
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
    spinBox_z->blockSignals(false);
}

void AGFloatVec3Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGFloatVec3Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

int AGFloatVec3Widget::calculateDecimals(float min, float max) {
	int decimals = 2;
    if (min >= 0 && min < 1 && max < 1) {
        //calculate number of decimals in min or in max if min is 0
        float tmp = (min == 0) ? max : min;
        while (tmp < 1) {
            tmp = tmp*10.0f;
            decimals++;
        }
	}
	return decimals;
}

void AGFloatVec3Widget::setRange(tgt::vec3 min, tgt::vec3 max) {
    spinBox_x->setRange(min.x, max.x);
	int decimals = calculateDecimals(min.x, max.x);
	if (decimals > 2)
		spinBox_x->setDecimals(decimals);
	
    spinBox_x->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_y->setRange(min.y, max.y);
	decimals = calculateDecimals(min.y, max.y);
	if (decimals > 2)
		spinBox_y->setDecimals(decimals);
	
    spinBox_y->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_z->setRange(min.z, max.z);
	decimals = calculateDecimals(min.z, max.z);
	if (decimals > 2)
		spinBox_z->setDecimals(decimals);
	
    spinBox_z->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

}

void AGFloatVec3Widget::changeValue(tgt::vec3 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	    disconnect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
		spinBox_z->setValue(value_.z);
        connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	    connect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
    }
}

void AGFloatVec3Widget::sendMessage() {
	if (isVisible()) {
		if (prop_->getAutoChange())
			prop_->set(value_);
		else
			msgReceiver_->postMessage(new Vec3Msg(msgIdent_, value_), prop_->getMsgDestination());
		
		MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
	}
}

void AGFloatVec3Widget::valChangedX(double x) {
	tgt::vec3 tmp = tgt::vec3(x, spinBox_y->value(), spinBox_z->value());
	if (tmp != value_) {
		value_ = tmp;
		spinBox_x->setValue(x);
		sendMessage();
	}
}

void AGFloatVec3Widget::valChangedY(double y) {
	tgt::vec3 tmp = tgt::vec3(spinBox_x->value(), y, spinBox_z->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}

void AGFloatVec3Widget::valChangedZ(double z) {
	tgt::vec3 tmp = tgt::vec3(spinBox_x->value(), spinBox_y->value(), z);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_z->setValue(z);
		sendMessage();
    }
}

//---------------------------------------------------------------------

AGFloatVec4Widget::AGFloatVec4Widget(QWidget* parent, MessageReceiver* msgReceiver, FloatVec4Prop* prop)
    : QWidget(parent)
    , TemplatePlugin<tgt::vec4>()
{
    msgReceiver_ = msgReceiver;
    msgIdent_ = prop->getIdent();
	value_ = tgt::vec4(0.0f);
	prop_ = prop;
    setObjectName(QString::fromUtf8("FloatVec4-Spinboxes"));

 	QWidget* top = new QWidget(this);
	QWidget* bottom = new QWidget(this);

    QHBoxLayout* hboxLayout_top = new QHBoxLayout(top);
	QHBoxLayout* hboxLayout_bottom = new QHBoxLayout(bottom);
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);

	vboxLayout->setSpacing(2);
	vboxLayout->setMargin(0);
	hboxLayout_top->setSpacing(5);
	hboxLayout_top->setMargin(1);
	hboxLayout_bottom->setSpacing(5);
	hboxLayout_bottom->setMargin(1);
	hboxLayout_top->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
	hboxLayout_top->addStretch();

	hboxLayout_top->addWidget(new QLabel(tr("x:")));
    spinBox_x = new QDoubleSpinBox();
    hboxLayout_top->addWidget(spinBox_x);

	hboxLayout_top->addWidget(new QLabel(tr("y:")));
    spinBox_y = new QDoubleSpinBox();
    hboxLayout_top->addWidget(spinBox_y);

	hboxLayout_bottom->addStretch();
	hboxLayout_bottom->addWidget(new QLabel(tr("z:")));
    spinBox_z = new QDoubleSpinBox();
    hboxLayout_bottom->addWidget(spinBox_z);

	hboxLayout_bottom->addWidget(new QLabel(tr("w:")));
    spinBox_w = new QDoubleSpinBox();
    hboxLayout_bottom->addWidget(spinBox_w);
	vboxLayout->addWidget(top);
	vboxLayout->addWidget(bottom);

    setRange(prop->getMinimum(), prop->getMaximum());
    changeValue(prop->get());

    connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	connect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
	connect( spinBox_w, SIGNAL( valueChanged(double) ), this, SLOT( valChangedW(double) ) );
    groupBox_ = 0;
}

void AGFloatVec4Widget::initWidgetFromLoadedSettings(tgt::vec4 min, tgt::vec4 max, tgt::vec4 value) {
    spinBox_x->blockSignals(true);
    spinBox_y->blockSignals(true);
    spinBox_z->blockSignals(true);
    spinBox_w->blockSignals(true);
    setRange(min, max);
    spinBox_x->setValue(value.x);
    spinBox_y->setValue(value.y);
    spinBox_z->setValue(value.z);
    spinBox_w->setValue(value.w);
    value_ = value;
    if (prop_->getAutoChange())
        prop_->set(value);
    else
        msgReceiver_->postMessage(new Vec4Msg(msgIdent_, value), prop_->getMsgDestination());
    spinBox_x->blockSignals(false);
    spinBox_y->blockSignals(false);
    spinBox_z->blockSignals(false);
    spinBox_w->blockSignals(false);
}

void AGFloatVec4Widget::setFrameControler(QGroupBox* box) {
    groupBox_ = box;
}

void AGFloatVec4Widget::setVisibleState(bool vis) {
	setVisible(vis);
    if (groupBox_)
        groupBox_->setVisible(vis);
}

int AGFloatVec4Widget::calculateDecimals(float min, float max) {
	int decimals = 2;
    if (min >= 0 && min < 1 && max < 1) {
        //calculate number of decimals in min or in max if min is 0
        float tmp = (min == 0) ? max : min;
        while (tmp < 1) {
            tmp = tmp*10.0f;
            decimals++;
        }
	}
	return decimals;
}

void AGFloatVec4Widget::setRange(tgt::vec4 min, tgt::vec4 max) {
    spinBox_x->setRange(min.x, max.x);
	int decimals = calculateDecimals(min.x, max.x);
	if (decimals > 2) {
		spinBox_x->setDecimals(decimals);
	}
	spinBox_x->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_y->setRange(min.y, max.y);
	decimals = calculateDecimals(min.y, max.y);
	if (decimals > 2)
		spinBox_y->setDecimals(decimals);
	
    spinBox_y->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_z->setRange(min.z, max.z);
	decimals = calculateDecimals(min.z, max.z);
	if (decimals > 2)
		spinBox_z->setDecimals(decimals);
	
    spinBox_z->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

	spinBox_w->setRange(min.w, max.w);
	decimals = calculateDecimals(min.w, max.w);
	if (decimals > 2)
		spinBox_w->setDecimals(decimals);
	
    spinBox_w->setSingleStep(1.0f/pow(10.0f, static_cast<float>(decimals)));

}

void AGFloatVec4Widget::changeValue(tgt::vec4 vec) {
    if (vec != value_) {
        value_ = vec;
        disconnect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    disconnect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	    disconnect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
	    disconnect( spinBox_w, SIGNAL( valueChanged(double) ), this, SLOT( valChangedW(double) ) );
	    spinBox_x->setValue(value_.x);
        spinBox_y->setValue(value_.y);
		spinBox_z->setValue(value_.z);
		spinBox_w->setValue(value_.w);
        connect( spinBox_x, SIGNAL( valueChanged(double) ), this, SLOT( valChangedX(double) ) );
	    connect( spinBox_y, SIGNAL( valueChanged(double) ), this, SLOT( valChangedY(double) ) );
	    connect( spinBox_z, SIGNAL( valueChanged(double) ), this, SLOT( valChangedZ(double) ) );
	    connect( spinBox_w, SIGNAL( valueChanged(double) ), this, SLOT( valChangedW(double) ) );
    }
}

void AGFloatVec4Widget::sendMessage() {
	if (isVisible()) {
		if (prop_->getAutoChange())
			prop_->set(value_);
		else
			msgReceiver_->postMessage(new Vec4Msg(msgIdent_, value_), prop_->getMsgDestination());
		
		MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
	}
}

void AGFloatVec4Widget::valChangedX(double x) {
	tgt::vec4 tmp = tgt::vec4(x, spinBox_y->value(), spinBox_z->value(), spinBox_w->value());
	if (tmp != value_) {
		value_ = tmp;
		spinBox_x->setValue(x);
		sendMessage();
	}
}

void AGFloatVec4Widget::valChangedY(double y) {
	tgt::vec4 tmp = tgt::vec4(spinBox_x->value(), y, spinBox_z->value(), spinBox_w->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_y->setValue(y);
		sendMessage();
    }
}

void AGFloatVec4Widget::valChangedZ(double z) {
	tgt::vec4 tmp = tgt::vec4(spinBox_x->value(), spinBox_y->value(), z, spinBox_w->value());
    if (tmp != value_) {
		value_ = tmp;
		spinBox_z->setValue(z);
		sendMessage();
    }
}

void AGFloatVec4Widget::valChangedW(double w) {
	tgt::vec4 tmp = tgt::vec4(spinBox_x->value(), spinBox_y->value(), spinBox_z->value(), w);
    if (tmp != value_) {
		value_ = tmp;
		spinBox_w->setValue(w);
		sendMessage();
    }
}

} // namespace voreen
