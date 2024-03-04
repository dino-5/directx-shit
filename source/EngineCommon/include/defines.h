#pragma once
#include<memory>

#define SHIT_ENGINE_NON_COPYABLE(NAME) NAME(const NAME&)=delete;\
NAME& operator=(const NAME&)=delete;


#define SHIT_ENGINE_SINGLETONE(NAME, DEFAULT) SHIT_ENGINE_NON_COPYABLE(NAME)\
protected:\
public:\
static NAME& Get##NAME()\
{\
	static NAME instance;\
	return instance;\
}\
private:\
NAME()DEFAULT;\
public:

#define SHIT_ENGINE_GET_D3D12COMPONENT(TYPE, NAME, VAR_NAME) TYPE* Get##NAME(){ return VAR_NAME; } 

//#include "EngineCommon/util/Logger.h"
//#include <format>
#define LogScope(name) engine::util::ScopeInfo _objectScope(std::format("{}::{}", name, __func__))
									
