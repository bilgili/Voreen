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
// NOTE: This file is currently not used.
//
#include <QtGui>

#include "voreen/qt/widgets/transfermarkerfield.h"

TransferMarkerField::TransferMarkerField(QWidget *parent, TransferGradient* gradient, MessageReceiver* msgReceiver/*= 0*/) : QWidget(parent) {
	markers_ = new QVector<TransferMarker*>();
	mouseDown_ = false;
    msgReceiver_ = msgReceiver;

    setFocus();
	setCursor(Qt::CrossCursor);
	markerSize_ = 15;
	gradient_ = gradient;
	gradient_->setMarkers(markers_);
}

void TransferMarkerField::resizeEvent(QResizeEvent*) {
	for (int i=0;i<markers_->size();i++) {
        markers_->at(i)->move((int)(width()*(markers_->at(i)->getIntensity()/MAX_NORM_VAL)-markerSize_/2),
                              (int)(height()-height()*markers_->at(i)->getTransparency()/MAX_NORM_VAL-markerSize_/2));
    }
}

void TransferMarkerField::addMarker(unsigned int intensity, unsigned int transparency, QColor color) {
	TransferMarker* newMarker = new TransferMarker(this);
	newMarker->setColor(color);
	newMarker->setGeometry(0, 0, markerSize_+2, markerSize_+2);
    newMarker->move((int)(width()*(intensity/MAX_NORM_VAL)-markerSize_/2),
                    (int)(height()-height()*transparency/MAX_NORM_VAL-markerSize_/2));
	newMarker->setIntensity(intensity);
	newMarker->setTransparency(transparency);
	newMarker->show();
	curMarker_ = 0;
	while (curMarker_ < markers_->size() && markers_->at(curMarker_)->getIntensity() < newMarker->getIntensity()) curMarker_++;
	markers_->insert(curMarker_, newMarker);
	update();
}

void TransferMarkerField::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

	float scalePosX = width()/MAX_NORM_VAL;
	float scalePosY = height()/MAX_NORM_VAL;
	// draw lines
	painter.setPen(Qt::black);
	for (int i=1;i<markers_->size();i++) {
		int curIntensity = markers_->at(i)->getIntensity();
		int curTransparency = markers_->at(i)->getTransparency();
		int preIntensity = markers_->at(i-1)->getIntensity();
		int preTransparency = markers_->at(i-1)->getTransparency();
	    painter.drawLine((int)(preIntensity*scalePosX), (int)(height()-preTransparency*scalePosY),
					     (int)(curIntensity*scalePosX), (int)(height()-curTransparency*scalePosY));
	}
	if (mouseDown_) {
		// draw text label at moving marker
		int curIntensity = markers_->at(curMarker_)->getIntensity();
		int curTransparency = markers_->at(curMarker_)->getTransparency();
		QString values = "(";
		values.append(QString::number(curIntensity));
		values.append(",");
		values.append(QString::number(curTransparency));
		values.append(")");
        int dx = 10;
        int dy = 0;
        //int dx = (MAX_NORM_VAL/2-curIntensity)/MAX_NORM_VAL/2*80;
        //int dy = (MAX_NORM_VAL/2-curTransparency)/MAX_NORM_VAL/2*30;
		painter.drawText((int)(curIntensity*scalePosX+dx-30),
                         (int)(height()-(curTransparency)*scalePosY-dy),
						 values);
	}
	gradient_->update();
}

void TransferMarkerField::mouseDoubleClickEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		TransferMarker* child = static_cast<TransferMarker*>(childAt(event->pos()));
		if (!child) return;
		child->setColor(QColorDialog::getColor());
        update();
    } else if (event->button() == Qt::MidButton) {
        saveToDisc("../data/transferfuncs/initial.tf");
    }
}

void TransferMarkerField::mousePressEvent(QMouseEvent* event) {
    if (msgReceiver_)
		postMessage(new BoolMsg(Identifier::switchCoarseness, true));
	QPoint pos = event->pos();
	if (event->button() == Qt::LeftButton) {
		TransferMarker* child = static_cast<TransferMarker*>(childAt(pos));
        if (!child) {
		    addMarker((int)(pos.x()/width()*MAX_NORM_VAL),
			          (int)((height()-pos.y())/height()*MAX_NORM_VAL),
                      QColor(Qt::lightGray));
				    //QColorDialog::getColor());
		    update();
        } else {
		    for (int i=0;i<markers_->size();i++) {
			    if (markers_->at(i)->getIntensity() == child->getIntensity() &&
				    markers_->at(i)->getTransparency() == child->getTransparency()) {
				    curMarker_ = i;
			    }
		    }
		    mouseDown_ = true;
        }
	} else if (event->button() == Qt::RightButton) {
		TransferMarker* child = static_cast<TransferMarker*>(childAt(pos));
        if (!child) return;
		for (int i=0;i<markers_->size();i++) {
			if (markers_->at(i)->getIntensity() == child->getIntensity() &&
				markers_->at(i)->getTransparency() == child->getTransparency()) {
				curMarker_ = i;
			}
		}
		if (curMarker_ != 0 && curMarker_ != markers_->size()-1) {
            TransferMarker* marker = markers_->at(curMarker_);
		    markers_->remove(curMarker_);
            marker->deleteLater();
			curMarker_--;
			update();
		}
	} else if (event->button() == Qt::MidButton) {
		TransferMarker* child = static_cast<TransferMarker*>(childAt(pos));
		if (!child) return;
		child->setColor(QColorDialog::getColor());
	}
}

void TransferMarkerField::mouseMoveEvent(QMouseEvent* event) {
	QPoint pos = event->pos();
    if (pos.y() < 0) pos.setY(0);
    if (pos.y() > height()) pos.setY(height());
	if (mouseDown_) {
		uint intensity = uint( (float)pos.x()/width()*MAX_NORM_VAL );
		uint transparency = uint( (float)(height()-pos.y())/height()*MAX_NORM_VAL );
		if ((curMarker_ != 0 && curMarker_ != markers_->size()-1) &&
		    (intensity > markers_->at(curMarker_-1)->getIntensity() &&
			intensity < markers_->at(curMarker_+1)->getIntensity())) {
			markers_->at(curMarker_)->setIntensity(intensity);
		}
		markers_->at(curMarker_)->setTransparency(transparency);
        markers_->at(curMarker_)->move((int)(width()*(markers_->at(curMarker_)->getIntensity()/MAX_NORM_VAL)-markerSize_/2),
                                       (int)(height()-height()*markers_->at(curMarker_)->getTransparency()/MAX_NORM_VAL-markerSize_/2));
		update();
	}
}

void TransferMarkerField::mouseReleaseEvent(QMouseEvent*) {
    if (msgReceiver_)
		postMessage(new BoolMsg(Identifier::switchCoarseness, false));
	mouseDown_ = false;
	update();
}

void TransferMarkerField::saveToDisc() {
    QString fileName = QFileDialog::getSaveFileName(
						this,
						tr("Choose a filename to save transfer function"),
						"../../data/transferfuncs",
						"Transfer Function (*.TF)");
    saveToDisc(fileName);
}

void TransferMarkerField::saveToDisc(QString fileName) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << (qint32)markers_->size();
    for (int i=0;i<markers_->size();i++) {
        out << (qint32)markers_->at(i)->getIntensity();
        out << (qint32)markers_->at(i)->getTransparency();
        out << markers_->at(i)->getColor();
    }
    file.close();
}

void TransferMarkerField::readFromDisc() {
	QString fileName = QFileDialog::getOpenFileName(
						this,
						tr("Choose a transfer function to open"),
						"../../data/transferfuncs",
						"Transfer Function (*.TF)");
    readFromDisc(fileName);
}

void TransferMarkerField::clean() {
    curMarker_ = 0;
    while (markers_->size() != 0) {
        TransferMarker* marker = markers_->at(0);
		markers_->remove(0);
        marker->deleteLater();
	}
    update();
}

void TransferMarkerField::readFromDisc(QString fileName) {
    clean();
    QFile file(fileName);
    bool res = file.open(QIODevice::ReadOnly);
    if (res) {
        QDataStream in(&file);
        qint32 size;
        in >> size;
        for (int i=0;i<size;i++) {
            qint32 intensity;
            qint32 transparency;
            QColor color;
            in >> intensity;
            in >> transparency;
            in >> color;
        addMarker(intensity,transparency,color);
        }
        file.close();
    } else {
        addMarker(0,0,Qt::black);
        addMarker(uint(MAX_NORM_VAL), uint(MAX_NORM_VAL), Qt::white);
    }
    update();
}
} // namespace voreen

