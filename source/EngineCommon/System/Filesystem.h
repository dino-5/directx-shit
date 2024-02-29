#pragma once
#include <filesystem>

namespace filesystem = std::filesystem;
using path = std::filesystem::path;
namespace engine::util
{
class Filepath
{
public:
	Filepath(std::string name);
	path getPath() { return m_path; }
private:
	path m_path;
};

class File
{
public:
    File(std::string fileName) : m_path(fileName){}
	std::string readFile();
private:
	Filepath m_path;
};

};
