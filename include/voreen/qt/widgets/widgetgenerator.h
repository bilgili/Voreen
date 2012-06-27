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

#ifndef VRN_WIDGETGENERATOR_H
#define VRN_WIDGETGENERATOR_H

#include "widgetplugin.h"

#include "voreen/core/vis/property.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/voreenpainter.h"

#include "tinyxml/tinyxml.h"

#include <QtGui>

#include <vector>


namespace voreen {

class GroupFrame;
class ProcessorHeaderWidget;

class TransFuncPlugin;
class TransFuncAlphaPlugin;

class WidgetGenerator : public WidgetPlugin {
    Q_OBJECT
public:

    class PropsWidgetList{
    public:
        Property* prop;
        QWidget* widget;
    };

    class ProcessorPropsList{
    public:
        Processor* processor;
        bool state;
        std::vector<PropsWidgetList*> propWidg_list_;
    };

    class OverlayPropsList{
    public:
        VoreenPainterOverlay* overlay;
        bool state;
        std::vector<PropsWidgetList*> propWidg_list_;
    };

    WidgetGenerator(QWidget* parent = 0, MessageReceiver* msgreceiver = 0);
    ~WidgetGenerator();

	void createAllWidgets(std::vector<Processor*> processors, OverlayManager* mgr);

    /**
     *This method returns all TransFuncPlugins created by widgetgenerator
     *
     *@return vector with all created TransFuncPlugins
     */
    std::vector<TransFuncPlugin*> getTransFuncPlugins();


    /**
     *This method returns all TransFuncAlphaPlugins created by widgetgenerator
     *
     *@return vector with all created TransFuncAlphaPlugins
     */
    std::vector<TransFuncAlphaPlugin*> getTransFuncAlphaPlugins();

    /**
     * Validates the visual state of a processor (properties visible or not visible)
     */
    void stateChanged();

public slots:
    void loadWidgetSettings();
    void saveWidgetSettings();
protected:
    /**
     *Central function of Widgetgenerator - creates the widgets for the whole pipeline
     */
    virtual void createWidgets();

    /**
     *Inherited function - does nothing
     */
    virtual void createConnections() {};
private:
    PropsWidgetList* makeWidget(Property* prop, QHBoxLayout* hb, QGroupBox* frame = 0);
    void makeConditionedWidgets(Property* prop, QHBoxLayout* hb, ProcessorPropsList* propsWidgetLists);
    void makeGroupedWidgets(GroupProp* prop, QHBoxLayout* hb, ProcessorPropsList* propsWidgetLists);
    void makeConditionedWidgets(Property* prop, QHBoxLayout* hb, OverlayPropsList* propsWidgetLists);
    void makeGroupedWidgets(GroupProp* prop, QHBoxLayout* hb, OverlayPropsList* propsWidgetLists);

    void savePropsToXML(std::vector< PropsWidgetList* >, size_t, TiXmlElement*);
    void loadPropsFromXML(std::vector< PropsWidgetList* >, TiXmlElement*);

    QWidget* parWidget_;
    MessageReceiver* msgReceiver_;
    std::vector<Processor* > processor_list;
    std::vector< VoreenPainterOverlay* > overlays_;
    std::vector< ProcessorPropsList* > processorPropertyPlugin_list_;
    std::vector< OverlayPropsList* > overlayPropertyPlugin_list_;
    std::vector< GroupFrame* > frames_;
    std::vector< ProcessorHeaderWidget* > ProcessorHeaderWidgets_;
    QLineEdit* configName_;
};

//---------------------------------------------------------------------

class GroupFrame : public QGroupBox{//, public TemplatePlugin<bool> {
    Q_OBJECT
public:
    GroupFrame(Property* prop, WidgetGenerator::ProcessorPropsList* list, MessageReceiver* msgReceiver);
    GroupFrame(Property* prop, WidgetGenerator::OverlayPropsList* list, MessageReceiver* msgReceiver);
    void setVisibleState(bool vis);
    void changeValue(bool val);
    void initWidgetFromLoadedSettings(bool value);

    Property* prop_;
    WidgetGenerator::ProcessorPropsList* list_;
    WidgetGenerator::OverlayPropsList* listOverlay_;

public slots:
    void changeChecked(bool val);

private:
    MessageReceiver* msgReceiver_;
    Identifier msgIdent_;
};

} // namespace voreen

#endif //VRN_WIDGETGENERATOR_H
