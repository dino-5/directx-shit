#pragma once

#define SHIT_ENGINE_NON_COPYABLE(NAME) NAME(const NAME&)=delete;\
NAME& operator=(const NAME&)=delete;


#define SHIT_ENGINE_SINGLETONE(NAME) SHIT_ENGINE_NON_COPYABLE(NAME)\
NAME()=default;\
static NAME* Get##NAME()\
{\
	static std::unique_ptr<NAME> obj;\
	if(obj.get()==nullptr){\
		obj = std::make_unique<NAME>();}\
	return obj.get();\
}
									
