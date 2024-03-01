#pragma once

#include "include/common.h"


D3D12_RESOURCE_FLAGS CastType(ResourceFlags flag)
{
	return static_cast<D3D12_RESOURCE_FLAGS>(flag);
}

