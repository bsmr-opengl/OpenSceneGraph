// ***************************************************************************
//
//   Generated automatically by genwrapper.
//   Please DO NOT EDIT this file!
//
// ***************************************************************************

#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/StaticMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/CopyOp>
#include <osg/Object>
#include <osgAnimation/Animation>
#include <osgAnimation/AnimationManager>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgAnimation::AnimationManager)
	I_DeclaringFile("osgAnimation/AnimationManager");
	I_BaseType(osgAnimation::AnimationManagerBase);
	I_Method0(osg::Object *, cloneType,
	          Properties::VIRTUAL,
	          __osg_Object_P1__cloneType,
	          "Clone the type of an object, with Object* return type. ",
	          "Must be defined by derived classes. ");
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, x,
	          Properties::VIRTUAL,
	          __osg_Object_P1__clone__C5_osg_CopyOp_R1,
	          "Clone an object, with Object* return type. ",
	          "Must be defined by derived classes. ");
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj,
	          Properties::VIRTUAL,
	          __bool__isSameKindAs__C5_osg_Object_P1,
	          "",
	          "");
	I_Method0(const char *, libraryName,
	          Properties::VIRTUAL,
	          __C5_char_P1__libraryName,
	          "return the name of the object's library. ",
	          "Must be defined by derived classes. The OpenSceneGraph convention is that the namespace of a library is the same as the library name. ");
	I_Method0(const char *, className,
	          Properties::VIRTUAL,
	          __C5_char_P1__className,
	          "return the name of the object's class type. ",
	          "Must be defined by derived classes. ");
	I_Constructor0(____AnimationManager,
	               "",
	               "");
	I_ConstructorWithDefaults2(IN, const osgAnimation::AnimationManager &, b, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY,
	                           ____AnimationManager__C5_AnimationManager_R1__C5_osg_CopyOp_R1,
	                           "",
	                           "");
	I_Method1(void, update, IN, double, time,
	          Properties::VIRTUAL,
	          __void__update__double,
	          "",
	          "");
	I_MethodWithDefaults3(void, playAnimation, IN, osgAnimation::Animation *, pAnimation, , IN, int, priority, 0, IN, float, weight, 1.0,
	                      Properties::NON_VIRTUAL,
	                      __void__playAnimation__Animation_P1__int__float,
	                      "",
	                      "");
	I_Method1(bool, stopAnimation, IN, osgAnimation::Animation *, pAnimation,
	          Properties::NON_VIRTUAL,
	          __bool__stopAnimation__Animation_P1,
	          "",
	          "");
	I_Method1(bool, findAnimation, IN, osgAnimation::Animation *, pAnimation,
	          Properties::NON_VIRTUAL,
	          __bool__findAnimation__Animation_P1,
	          "",
	          "");
	I_Method1(bool, isPlaying, IN, osgAnimation::Animation *, pAnimation,
	          Properties::NON_VIRTUAL,
	          __bool__isPlaying__Animation_P1,
	          "",
	          "");
	I_Method1(bool, isPlaying, IN, const std::string &, animationName,
	          Properties::NON_VIRTUAL,
	          __bool__isPlaying__C5_std_string_R1,
	          "",
	          "");
	I_Method0(void, stopAll,
	          Properties::NON_VIRTUAL,
	          __void__stopAll,
	          "",
	          "");
END_REFLECTOR

