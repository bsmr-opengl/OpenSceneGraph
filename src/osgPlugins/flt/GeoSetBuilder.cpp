// GeoSetBuilder.cpp

#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( disable : 4786 )
#endif

#include "flt.h"
#include "FltFile.h"
#include "Pool.h"
#include "opcodes.h"
#include "GeoSetBuilder.h"

#include <osg/Object>
#include <osg/LOD>
#include <osg/Transparency>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/PolygonOffset>
#include <osg/Point>
#include <osg/Notify>

#include <osgUtil/Tesselator>

#include <map>
#include <algorithm>

using namespace flt;


////////////////////////////////////////////////////////////////////
//
//                       DynGeoSet
//
////////////////////////////////////////////////////////////////////


#define APPEND_DynGeoSet_List(list)                     \
    if (source->list.size() > 0)                        \
        list.insert(list.end(),                         \
            source->list.begin(), source->list.end());


void DynGeoSet::append(DynGeoSet* source)
{
    APPEND_DynGeoSet_List(_primLenList)
    APPEND_DynGeoSet_List(_coordList)
    if (_normal_binding==osg::Geometry::BIND_PER_VERTEX || _normal_binding==osg::Geometry::BIND_PER_PRIMITIVE) APPEND_DynGeoSet_List(_normalList)
    if (_color_binding==osg::Geometry::BIND_PER_VERTEX || _color_binding==osg::Geometry::BIND_PER_PRIMITIVE) APPEND_DynGeoSet_List(_colorList)
    if (_texture_binding==osg::Geometry::BIND_PER_VERTEX || _texture_binding==osg::Geometry::BIND_PER_PRIMITIVE) APPEND_DynGeoSet_List(_tcoordList)
}


#define VERIFY_DynGeoSet_Binding(binding,list)          \
        switch (binding)                                \
        {                                               \
        case osg::Geometry::BIND_PER_VERTEX:               \
            if (list.size() < _coordList.size()) {      \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::Geometry::BIND_PER_PRIMITIVE:                 \
            if (list.size() < _primLenList.size()) {    \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        case osg::Geometry::BIND_OVERALL:                 \
            if (list.size() < 1) {                      \
                binding = osg::Geometry::BIND_OFF;        \
                list.clear(); }                         \
            break;                                      \
        default:                                        \
            break;                                      \
        }

const osg::Primitive::Mode NO_PRIMITIVE_TYPE = (osg::Primitive::Mode)0xffff;

DynGeoSet::DynGeoSet()
{
    _primtype=NO_PRIMITIVE_TYPE;
}

void DynGeoSet::setBinding()
{
    VERIFY_DynGeoSet_Binding(_normal_binding, _normalList)
    VERIFY_DynGeoSet_Binding(_color_binding, _colorList)
    VERIFY_DynGeoSet_Binding(_texture_binding, _tcoordList)

    // Set bindings
    setNormalBinding(_normal_binding);
    setColorBinding(_color_binding);
    setTextureBinding(_texture_binding);

    osg::StateSet* stateset = getStateSet();
    if (stateset)
    {
        if (_normal_binding == osg::Geometry::BIND_OFF)
            stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    }
}



void DynGeoSet::addToGeometry(osg::Geometry* geom)
{
    int indexBase = 0;
    
    geom->setStateSet(getStateSet());

    osg::Vec3Array* vertices = geom->getVertexArray();
    if (vertices)
    {
        indexBase = vertices->size();
        vertices->insert(vertices->end(),_coordList.begin(),_coordList.end());
    }
    else
    {
        vertices = new osg::Vec3Array(_coordList.begin(),_coordList.end());
        geom->setVertexArray(vertices);
    }

    if (!_normalList.empty())
    {
        osg::Vec3Array* normals = geom->getNormalArray();
        if (normals)
        {
            if (_normal_binding==osg::Geometry::BIND_PER_VERTEX || _normal_binding==osg::Geometry::BIND_PER_PRIMITIVE)
                normals->insert(normals->end(),_normalList.begin(),_normalList.end());
        }
        else
        {
            normals = new osg::Vec3Array(_normalList.begin(),_normalList.end());
            geom->setNormalArray(normals);

            switch(_normal_binding)
            {
                case(osg::Geometry::BIND_OVERALL):geom->setNormalBinding(osg::Geometry::BIND_OVERALL);break;
                case(osg::Geometry::BIND_PER_VERTEX):geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);break;        
                case(osg::Geometry::BIND_PER_PRIMITIVE):geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);break;        
                default:geom->setNormalBinding(osg::Geometry::BIND_OFF); break;
            }
        }
    }

    if (!_tcoordList.empty())
    {
        osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(0));
        if (texcoords)
        {
            texcoords->insert(texcoords->end(),_tcoordList.begin(),_tcoordList.end());
        }
        else
        {
            texcoords = new osg::Vec2Array(_tcoordList.begin(),_tcoordList.end());
            geom->setTexCoordArray(0,texcoords);
        }
    }
    
    if (!_colorList.empty())
    {
        osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
        if (colors)
        {
            if (_color_binding==osg::Geometry::BIND_PER_VERTEX || _color_binding==osg::Geometry::BIND_PER_PRIMITIVE)
                colors->insert(colors->end(),_colorList.begin(),_colorList.end());
        }
        else
        {
            colors = new osg::Vec4Array(_colorList.begin(),_colorList.end());
            geom->setColorArray(colors);

            switch(_color_binding)
            {
                case(osg::Geometry::BIND_OVERALL):geom->setColorBinding(osg::Geometry::BIND_OVERALL);break;
                case(osg::Geometry::BIND_PER_VERTEX):geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);break;        
                case(osg::Geometry::BIND_PER_PRIMITIVE):geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);break;        
                default:geom->setColorBinding(osg::Geometry::BIND_OFF); break;
            }
        }
    }    
    
    if (_primtype!=osg::Primitive::POLYGON)
    {
        geom->addPrimitive(new osg::DrawArrays(_primtype,indexBase,_coordList.size()));
    }
    else
    {
        for(PrimLenList::iterator itr=_primLenList.begin();
            itr!=_primLenList.end();
            ++itr)
        {
            geom->addPrimitive(new osg::DrawArrays(_primtype,indexBase,*itr));
            indexBase += *itr;
        }
    }    

}


////////////////////////////////////////////////////////////////////
//
//                       GeoSetBuilder
//
////////////////////////////////////////////////////////////////////

// OpenFlight don't save data in GeoSets.  This class tries to find
// existing GeoSets with matching state before creating a new GeoSet.


GeoSetBuilder::GeoSetBuilder(osg::Geode* geode)
{
    _geode = geode;
    initPrimData();
}


void GeoSetBuilder::initPrimData()
{
    _dynGeoSet = new DynGeoSet;
    _dynGeoSet->setStateSet(new osg::StateSet);
}

struct SortDynGeoSet
{
    bool operator () (const osg::ref_ptr<DynGeoSet>& lhs,const osg::ref_ptr<DynGeoSet>& rhs)
    {
        return *lhs<*rhs;
    }
};

osg::Geode* GeoSetBuilder::createOsgGeoSets(osg::Geode* geode)
{
    if( geode == NULL) geode = _geode.get();
    if( geode == NULL) return geode;

    for(DynGeoSetList::iterator itr = _dynGeoSetList.begin();
        itr!=_dynGeoSetList.end();
        ++itr)
    {
        osg::Geometry* geom = new osg::Geometry;
        geode->addDrawable(geom);
        (*itr)->addToGeometry(geom);
    }

    osgUtil::Tesselator tesselator;
    for(unsigned int i=0;i<geode->getNumDrawables();++i)
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
        if (geom) tesselator.retesselatePolygons(*geom);
    }

    return geode;
}


bool GeoSetBuilder::addPrimitive(bool dontMerge)
{
    DynGeoSet* dgset = getDynGeoSet();  // This is the new geoset we want to add

    if (dgset->getPrimType()==NO_PRIMITIVE_TYPE)
    {
        dgset->setPrimType(findPrimType(dgset->coordListSize()));
    }

    // Still no primitive type?
    if (dgset->getPrimType()==NO_PRIMITIVE_TYPE)
        return false;

    dgset->setBinding();

    if( dontMerge == true)
    {
        _dynGeoSetList.push_back(dgset);
    }
    else
    {
        DynGeoSet* match = findMatchingGeoSet();
        if (match)
            match->append(dgset);
        else
            _dynGeoSetList.push_back(dgset);
    }
    
    initPrimData();     // initialize _dynGeoSet
    return true;
}


DynGeoSet* GeoSetBuilder::findMatchingGeoSet()
{
    DynGeoSet* new_dgset = getDynGeoSet();
    for(DynGeoSetList::iterator itr=_dynGeoSetList.begin();
        itr!=_dynGeoSetList.end();
        ++itr)
    {
        DynGeoSet* dgset = itr->get();
        if (*new_dgset == *dgset)
            return dgset;
    }

    return NULL;
}


osg::Primitive::Mode GeoSetBuilder::findPrimType(const int nVertices)
{
    switch (nVertices)
    {
        case 1: return osg::Primitive::POINTS;
        case 2: return osg::Primitive::LINES;
        case 3: return osg::Primitive::TRIANGLES;
        case 4: return osg::Primitive::QUADS;
    }

    if (nVertices>=5) return osg::Primitive::POLYGON;
    
    return NO_PRIMITIVE_TYPE;
}


