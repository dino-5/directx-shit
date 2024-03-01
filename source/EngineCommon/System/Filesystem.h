#pragma once
#include <filesystem>

namespace fs = std::filesystem;
using path = fs::path;
extern path g_homeDir;
extern path g_demoDir;

namespace engine::system
{
class Filepath
{
public:
	Filepath(std::string name);
	Filepath(fs::path path);
	path getPath() { return m_path; }
private:
	path m_path;
};

class File
{
public:
    File(std::string fileName) : m_path(fileName){}
	File(fs::path path) :m_path(path) {}
	std::string readFile();
private:
	Filepath m_path;
};

};
