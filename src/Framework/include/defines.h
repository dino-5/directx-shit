#pragma once
#include<memory>

#define SHIT_ENGINE_NON_COPYABLE(NAME) NAME(const NAME&)=delete;\
NAME& operator=(const NAME&)=delete;


#define SHIT_ENGINE_SINGLETONE(NAME) SHIT_ENGINE_NON_COPYABLE(NAME)\
protected:\
static inline std::unique_ptr<NAME> singleton;\
public:\
static NAME* Get##NAME()\
{\
	if(singleton.get()==nullptr){\
		singleton = std::make_unique<NAME>();}\
	return singleton.get();\
}\
static void Set##NAME(NAME* obj)\
{\
	singleton.reset(obj);\
}\
NAME(){Set##NAME(this);}\

#define SHIT_ENGINE_GET_D3D12COMPONENT(TYPE, NAME, VAR_NAME) TYPE* Get##NAME(){ return VAR_NAME.Get(); } 

//#include "Framework/util/Logger.h"
//#include <format>
#define LogScope(name) engine::util::ScopeInfo _objectScope(std::format("{}::{}", name, __func__))
									
