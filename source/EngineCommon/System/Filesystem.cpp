#include "Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/Util.h"
#include "third_party/fmt/include/fmt/printf.h"
#include "third_party/fmt/include/fmt/core.h"

#include <fstream>
fs::path g_homeDir;
fs::path g_demoDir;
fs::path g_shaderDir;

namespace engine::system
{
Filepath::Filepath(std::string path)
{
	Init(path);
}

Filepath::Filepath(fs::path path)
{
	Init(path);
}

void Filepath::Init(fs::path path)
{
	m_path = path;
	if (!fs::exists(m_path))
	{
		std::string error = engine::util::to_string(path.native());
		engine::util::PrintError("{} is not exist", error);
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
