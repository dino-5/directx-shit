#include "Timer.h"

namespace engine::util {

    bool s_profilingEnabled = true;

    decltype(clock::now()) g_programStartTime;
}
