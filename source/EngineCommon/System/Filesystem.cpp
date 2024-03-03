#include "Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/Util.h"
#include "third_party/fmt/include/fmt/printf.h"
#include "third_party/fmt/include/fmt/core.h"

#include <fstream>
fs::path g_homeDir;
fs::path g_demoDir;
namespace engine::system
{
Filepath::Filepath(std::string name):m_path(name)
{
	if (!fs::exists(m_path))
	{
		engine::util::PrintError("{} is not exist", name);
		throw std::runtime_error(fmt::format("{} is not exist", name));
	}
}

Filepath::Filepath(fs::path path):m_path(path)
{
	if (!fs::exists(m_path))
	{
		std::string error = engine::util::to_string(path.native());
		engine::util::PrintError("{} is not exist", error);
		throw std::runtime_error(fmt::format("{} is not exist", error));
	}
}

std::string Filepath::readFile()
{
	std::ifstream file(m_path);
	std::ostringstream s_str;
	s_str << file.rdbuf();
	return s_str.str();
}

};
