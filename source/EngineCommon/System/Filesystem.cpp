#include "Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/Util.h"
#include "third_party/fmt/include/fmt/printf.h"
#include "third_party/fmt/include/fmt/core.h"

#include <fstream>

inline bool checkFileExists(const fs::path& path)
{
	if (!fs::exists(path))
	{
		std::string error = engine::util::to_string(path.native());
		engine::util::printError("{} is not exist", error);
		return false;
	}
	return true;
}

namespace engine::system
{

Filepath::Filepath(std::string path)
{
	init(path);
}

Filepath::Filepath(fs::path path)
{
	init(path);
}

void Filepath::init(fs::path path)
{
	m_path = path;
}

std::string Filepath::readFile()
{
	if(checkFileExists(m_path))
	{
        std::ifstream file(m_path);
        std::ostringstream s_str;
        s_str << file.rdbuf();
        return s_str.str();
	}
	return "";
}

};
