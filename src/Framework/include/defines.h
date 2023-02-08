#pragma once
#include<memory>

#define SHIT_ENGINE_NON_COPYABLE(NAME) NAME(const NAME&)=delete;\
NAME& operator=(const NAME&)=delete;


#define SHIT_ENGINE_SINGLETONE(NAME) SHIT_ENGINE_NON_COPYABLE(NAME)\
NAME()=default;\
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
}

									
