#include "Timer.h"

namespace engine::util {

    bool s_profilingEnabled = false;

    decltype(clock::now()) g_programStartTime;
}
