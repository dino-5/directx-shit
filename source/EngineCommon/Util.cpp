#include "EngineCommon/util/Util.h"
#include <comdef.h>
#include <fstream>
#include "EngineCommon/include/types.h"

namespace engine::util {

    bool IsKeyDown(int vkeyCode)
    {
        return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
    }
};
