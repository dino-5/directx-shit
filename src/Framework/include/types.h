#pragma once
#include<wrl/client.h>
#include<utility>
#include<string>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr <T>;
using uint = unsigned int;
using uint8 = unsigned char;

template<typename T>
using TableEntry = std::pair<std::string, T>;
