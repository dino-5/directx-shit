#pragma once
#include "EngineCommon/include/common.h"

class KeyboardHandler 
{
public:
	virtual void OnKeyDown(Key key) = 0;
};

class MouseHandler
{
public:
	virtual void OnMouseDown(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) = 0;
};
