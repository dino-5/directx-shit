#pragma once
#include <filesystem>
#include "EngineCommon/util/Util.h"

namespace fs = std::filesystem;
using path = fs::path;

namespace engine::system
{
class Filepath
{
public:
	Filepath() = default;
	Filepath(std::string name);
	Filepath(fs::path path);
	void init(fs::path path);

	path getPath() { return m_path; }
	std::string readFile();
	std::string str() { return util::to_string(m_path); }
	std::wstring wstr() { return m_path.c_str(); }
	std::wstring wfilename() { return m_path.filename(); }
	std::string filename() { return util::to_string(m_path.filename()); }
	Filepath parent() const { return Filepath(m_path.parent_path()) ; }
	Filepath operator/(const std::string& name)
	{
		return Filepath(m_path / name);
	}
	Filepath operator/(const std::wstring& name)
	{
		return Filepath(m_path / name);
	}

	Filepath operator/(const Filepath& path)
	{
		return Filepath(m_path / path.m_path);
	}
private:
	path m_path;
};

enum DirectoryPath
{
	DirectoryPath_HomeDir,
	DirectoryPath_DemoDir,
	DirectoryPath_ShaderDir,
	DirectoryPath_Count
};
};
