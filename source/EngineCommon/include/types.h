#pragma once
#include<wrl/client.h>
#include <utility>
#include <string>
#include <type_traits>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;

using uint = unsigned int;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;

template<typename T>
using TableEntry = std::pair<std::wstring, T>;

template<typename Enum>
constexpr u32 castEnum(Enum value)
{
	static_assert(std::is_enum<Enum>::value, "Enum must be an enum type");
	return static_cast<u32>(value);
}

