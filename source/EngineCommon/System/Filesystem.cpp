#include "Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "third_party/fmt/include/fmt/printf.h"
#include "third_party/fmt/include/fmt/core.h"

#include <fstream>
namespace engine::util
{

Filepath::Filepath(std::string name):m_path(name)
{
	if (!filesystem::exists(m_path))
	{
		PrintError("{} is not exist", name);
		throw std::runtime_error(fmt::format("{} is not exist", name));
	}
}

std::string File::readFile()
{
	std::ifstream file(m_path.getPath());
	std::ostringstream s_str;
	s_str << file.rdbuf();
	return s_str.str();
}

};
