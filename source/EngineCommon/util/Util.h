#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <source_location>
#include <format>
#include "EngineGfx/dx12/d3dx12.h"
#include "MathHelper.h"
#include <minwinbase.h>
#include "EngineCommon/include/types.h"

using namespace engine;

namespace engine::util
{
    inline std::wstring to_wstring(std::string str)
    {
        return std::wstring(str.begin(), str.end());
    }

    inline std::string to_string(std::wstring wstr)
    {
        return std::string(wstr.begin(), wstr.end());
    }

    inline std::string to_string(const wchar_t* wchar)
    {
        std::wstring wstr(wchar);
        return std::string(wstr.begin(), wstr.end());
    }

    inline std::string GetFormattedPath(const std::source_location& location, const std::string_view& name = "")
    {
        if(name.length() == 0)
            return std::format("{}::{}", location.function_name(), location.line());
        return std::format("{}::{}::{}", location.function_name(), location.line(), name);
    }

    inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }
    inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }
    inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
    {
        if (obj)
        {
            obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
        }
    }

    inline std::wstring AnsiToWString(const std::string& str)
    {
        WCHAR buffer[512];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
        return std::wstring(buffer);
    }

    bool IsKeyDown(int vkeyCode);

    std::string ToString(HRESULT hr);

    inline UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        return (byteSize + 255) & ~255;
    }

    template<typename T>
    inline T* FindElement(Table<T>& vector, std::wstring name)
    {
        auto findResult = std::find_if(vector.begin(), vector.end(),
            [name](TableEntry<T> el) { return el.first == name; });
        if (findResult != vector.end())
        {
            return &((findResult)->second);
        }
        return nullptr;
    }
    
#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

};
