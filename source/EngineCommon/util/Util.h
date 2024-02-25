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
#include "EngineGfx/dx12/d3dx12.h"
#include "MathHelper.h"
#include <minwinbase.h>
#include "EngineCommon/include/types.h"


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

    class d3dUtil
    {
    public:

        static bool IsKeyDown(int vkeyCode);

        static std::string ToString(HRESULT hr);

        static UINT CalcConstantBufferByteSize(UINT byteSize)
        {
            // Constant buffers must be a multiple of the minimum hardware
            // allocation size (usually 256 bytes).  So round up to nearest
            // multiple of 256.  We do this by adding 255 and then masking off
            // the lower 2 bytes which store all bits < 256.
            // Example: Suppose byteSize = 300.
            // (300 + 255) & ~255
            // 555 & ~255
            // 0x022B & ~0x00ff
            // 0x022B & 0xff00
            // 0x0200
            // 512
            return (byteSize + 255) & ~255;
        }

        static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

        static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
            ID3D12Device* device,
            ID3D12GraphicsCommandList* cmdList,
            const void* initData,
            UINT64 byteSize,
            Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

        static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
            const std::wstring& filename,
            const D3D_SHADER_MACRO* defines,
            const std::string& entrypoint,
            const std::string& target);
    };

    class DxException
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber);

        std::string ToString()const;

        HRESULT ErrorCode = S_OK;
        std::string FunctionName;
        std::string Filename;
        int LineNumber = -1;
    };


    template<typename T>
    inline T* FindElement(std::vector<TableEntry<T>>& vector, std::wstring name)
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