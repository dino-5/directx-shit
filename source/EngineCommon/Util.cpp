#include "EngineCommon/util/Util.h"
#include <comdef.h>
#include <fstream>
#include "include/common.h"
#include "EngineGfx/dx12_includes.hpp"
#include "EngineCommon/include/types.h"

namespace engine::util {


    DxException::DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber) :
        ErrorCode(hr),
        FunctionName(functionName),
        Filename(filename),
        LineNumber(lineNumber)
    {
    }

    bool IsKeyDown(int vkeyCode)
    {
        return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
    }

    std::string DxException::ToString()const
    {
        // Get the string description of the error code.
        _com_error err(ErrorCode);
        std::string msg = err.ErrorMessage();

        return FunctionName + " failed in " + Filename + "; line " + std::to_string(LineNumber) + "; error: " + msg;
    }

};
