#pragma once
#include <array>
#include <string>
#include <optional>
#include "EngineCommon/include/defines.h"
#include "EngineCommon/include/types.h"
#undef GetCommandLine


enum class CommandLineOption : u8
{
    ADAPTER,
    LIST_ADAPTER,
    COUNT
};

inline u8 castType(CommandLineOption option) {
    return static_cast<u8>(option);
}

constexpr const size_t cmdArgsSize = static_cast<size_t>(CommandLineOption::COUNT);
const std::string commandLineStr[cmdArgsSize] =
{
    "adapter",
    "list_adapter"
};

class CommandLine
{
public: 
    SHIT_ENGINE_SINGLETONE(CommandLine);
    bool isSet(CommandLineOption option) { return args[castType(option)].has_value(); }
    i32 getValue(CommandLineOption option) {
        if(isSet(option))
            return args[castType(option)].value();
        return -1;
    }
private:
    std::array<std::optional<i32>, cmdArgsSize> args;
};

