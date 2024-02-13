#pragma once

#include "include/common.h"

fs::path homeDir;
fs::path demoDir;

D3D12_RESOURCE_FLAGS CastType(ResourceFlags flag)
{
	return static_cast<D3D12_RESOURCE_FLAGS>(flag);
}

