/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


#include <osgUI/ComboBox>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>
#include <osg/ValueObject>
#include <osg/io_utils>

using namespace osgUI;

ComboBox::ComboBox():
    _currentItem(0)
{
}

ComboBox::ComboBox(const osgUI::ComboBox& combobox, const osg::CopyOp& copyop):
    Widget(combobox, copyop),
    _items(combobox._items)
{
}

bool ComboBox::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    // OSG_NOTICE<<"ComboBox::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    if (!getHasEventFocus())
    {
        if (ea->getEventType()==osgGA::GUIEventAdapter::PUSH && _popup->getVisible())
        {
            _popup->setVisible(false);
        }

        return false;
    }


    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::SCROLL):
            if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
                if (getCurrentItem()<getNumItems()-1) setCurrentItem(getCurrentItem()+1);
                return true;
            }
            else if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP)
            {
                if (getCurrentItem()>0) setCurrentItem(getCurrentItem()-1);
                return true;
            }
            break;

        case(osgGA::GUIEventAdapter::KEYDOWN):
            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Down)
            {
                if (getCurrentItem()<getNumItems()-1) setCurrentItem(getCurrentItem()+1);
                return true;
            }
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Up)
            {
                if (getCurrentItem()>0) setCurrentItem(getCurrentItem()-1);
                return true;
            }

            break;

        case(osgGA::GUIEventAdapter::PUSH):
        {
            OSG_NOTICE<<"Button pressed "<<std::endl;
            // toggle visibility of popup.
            osgUI::Widget::Intersections intersections;
            osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
            osgGA::GUIEventAdapter* ea = event ? event->asGUIEventAdapter() : 0;
//           if ((aa && ea) && aa->computeIntersections(*ea, ev->getNodePath(), intersections))
            if ((aa && ea) && computeIntersections(ev, ea, intersections))
            {
                OSG_NOTICE<<"ComboBox intersections { "<<std::endl;
                for(osgUI::Widget::Intersections::const_iterator itr =intersections.begin();
                    itr!=intersections.end();
                    ++itr)
                {
                        const osgUtil::LineSegmentIntersector::Intersection& hit = *itr;
                        OSG_NOTICE<<"   hit:drawable "<<hit.drawable.get()<<", "<<hit.drawable->getName()<<std::endl;
                        OSG_NOTICE<<"   NodePath::size() "<<hit.nodePath.size()<<std::endl;
                }
                OSG_NOTICE<<"}"<<std::endl;

                const osgUtil::LineSegmentIntersector::Intersection& hit = *intersections.begin();
                osg::Vec3d localPosition = hit.getLocalIntersectPoint();
                if (_extents.contains(localPosition, 1e-6))
                {
                    OSG_NOTICE<<"ComboBox button"<<std::endl;
                    _popup->setVisible(!_popup->getVisible());
                }

                if (_popup->getVisible() && _popup->getExtents().contains(localPosition, 1e-6))
                {
                    OSG_NOTICE<<"In pop up"<<std::endl;
                    OSG_NOTICE<<"   hit:drawable "<<hit.drawable.get()<<std::endl;
                    OSG_NOTICE<<"   NodePath::size() "<<hit.nodePath.size()<<std::endl;

                    unsigned int index=_items.size();
                    for(osg::NodePath::const_reverse_iterator itr = hit.nodePath.rbegin();
                        itr != hit.nodePath.rend();
                        ++itr)
                    {
                        if ((*itr)==this) break;
                        if ((*itr)->getUserValue("index",index)) break;
                    }

                    if (index<_items.size())
                    {
                        OSG_NOTICE<<"   index selected "<<index<<std::endl;
                        setCurrentItem(index);
                    }
                    else
                    {
                        OSG_NOTICE<<"   No index selected "<<std::endl;
                    }

                    _popup->setVisible(false);
                }
            }
            else
            {
                _popup->setVisible(false);
            }
            break;
        }
        case(osgGA::GUIEventAdapter::RELEASE):
            OSG_NOTICE<<"Button release "<<std::endl;
            break;

        default:
            break;
    }

    return false;
}

void ComboBox::enterImplementation()
{
    OSG_NOTICE<<"ComboBox enter"<<std::endl;
    if (_backgroundSwitch.valid()) _backgroundSwitch->setSingleChildOn(1);
}


void ComboBox::leaveImplementation()
{
    OSG_NOTICE<<"ComboBox leave"<<std::endl;
    if (_backgroundSwitch.valid()) _backgroundSwitch->setSingleChildOn(0);
}

void ComboBox::setCurrentItem(unsigned int i)
{
    OSG_NOTICE << "ComboBox::setCurrentItem("<<i<<")"<<std::endl;
    _currentItem = i;
    if (_buttonSwitch.valid()) _buttonSwitch->setSingleChildOn(_currentItem);
}

void ComboBox::createGraphicsImplementation()
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    _buttonSwitch = new osg::Switch;
    _popup = new osgUI::Popup;
    _popup->setVisible(false);
    _popup->setFrameSettings(getFrameSettings());

    osg::BoundingBox extents(_extents);

    osg::ref_ptr<osg::Group> group = new osg::Group;
    bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);
    float frameWidth = 0.0;
    float unFocused = 0.92;
    float withFocus = 0.97;
    osg::Vec4 frameColor(unFocused,unFocused,unFocused,1.0f);

    if (requiresFrame)
    {
        frameWidth = getFrameSettings()->getLineWidth();

        group->addChild(style->createFrame(_extents, getFrameSettings(), frameColor));
        extents.xMin() += frameWidth;
        extents.xMax() -= frameWidth;
        extents.yMin() += frameWidth;
        extents.yMax() -= frameWidth;
    }


    bool itemsHaveColor = false;
    for(Items::iterator itr = _items.begin();
        itr != _items.end();
        ++itr)
    {
        Item* item = itr->get();
        if (item->getColor().a()!=0.0f)
        {
            itemsHaveColor = true; break;
        }
    }

    // work out position of carat.
    float h = extents.yMax()-extents.yMin();
    float w = h*0.7;
    float minItemWidth = (extents.xMax()-extents.xMin())*0.5f;
    if (w>minItemWidth) w = minItemWidth;
    float xDivision = extents.xMax()-w;

    osg::BoundingBox backgroundExtents = extents;
    osg::BoundingBox iconExtents = backgroundExtents;
    iconExtents.xMin() = xDivision;
    extents.xMax() = xDivision;

    if (itemsHaveColor)
    {
        backgroundExtents.xMin() = xDivision;
    }

    OSG_NOTICE<<"itemsHaveColor = "<<itemsHaveColor<<std::endl;

    // clear background of edit region
    _backgroundSwitch = new osg::Switch;
    _backgroundSwitch->addChild(style->createPanel(backgroundExtents, osg::Vec4(unFocused, unFocused, unFocused, 1.0)));
    _backgroundSwitch->addChild(style->createPanel(backgroundExtents, osg::Vec4(withFocus, withFocus, withFocus,1.0)));
    _backgroundSwitch->setSingleChildOn(0);


    // assign carat
    group->addChild(_backgroundSwitch.get());

    group->addChild(_buttonSwitch.get());

    //group->addChild(style->createIcon(iconExtents, "cow.osgt", osg::Vec4(withFocus, withFocus, withFocus,1.0)));
    group->addChild(style->createIcon(iconExtents, "Images/osg64.png", osg::Vec4(withFocus, withFocus, withFocus,1.0)));

    if (!_items.empty())
    {

        float margin = (extents.yMax()-extents.yMin())*0.1f;
        // float itemWidth = (_extents.xMax()-_extents.xMin()) - 2.0f*frameWidth;
        float itemHeight = (_extents.yMax()-_extents.yMin()) - 2.0f*frameWidth;
        float popupHeight = (itemHeight)* _items.size() + margin*static_cast<float>(_items.size()-1) + 2.0f*frameWidth;
        float popupTop = _extents.yMin()-frameWidth-margin*1.0f;
        float popupLeft = _extents.xMin();
        float popupRight = _extents.xMax();

        osg::BoundingBox popupExtents(popupLeft, popupTop-popupHeight, _extents.zMin(), popupRight, popupTop, _extents.zMax());
        _popup->setExtents(popupExtents);

        osg::BoundingBox popupItemExtents(popupExtents.xMin()+frameWidth, popupTop-frameWidth-itemHeight, popupExtents.zMin(), popupExtents.xMax()-frameWidth, popupTop-frameWidth, popupExtents.zMax());

        unsigned int index = 0;
        for(Items::iterator itr = _items.begin();
            itr != _items.end();
            ++itr, ++index)
        {
            Item* item = itr->get();
            OSG_NOTICE<<"Creating item "<<item->getText()<<", "<<item->getColor()<<std::endl;

            // setup graphics for button
            {
                osg::ref_ptr<osg::Group> group = new osg::Group;
                if (item->getColor().a()!=0.0f) group->addChild( style->createPanel(extents, item->getColor()) );
                if (!item->getText().empty()) group->addChild( style->createText(extents, getAlignmentSettings(), getTextSettings(), item->getText()) );
                _buttonSwitch->addChild(group.get());
            }

            // setup graphics for popup
            {
                osg::ref_ptr<osg::Group> group = new osg::Group;
                group->setUserValue("index",index);

                if (item->getColor().a()!=0.0f) group->addChild( style->createPanel(popupItemExtents, item->getColor()) );
                if (!item->getText().empty()) group->addChild( style->createText(popupItemExtents, getAlignmentSettings(), getTextSettings(), item->getText()) );
                _popup->addChild(group.get());

                popupItemExtents.yMin() -= (itemHeight+margin);
                popupItemExtents.yMax() -= (itemHeight+margin);
            }

        }

    }
    else
    {
        _buttonSwitch->addChild( style->createPanel(_extents, frameColor) );
    }

    _buttonSwitch->setSingleChildOn(_currentItem);

    style->setupClipStateSet(_extents, getOrCreateStateSet());

    setGraphicsSubgraph(0, group.get());
    addChild(_popup.get());
}
