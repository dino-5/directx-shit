#include "Filesystem.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/util/Util.h"
#include "third_party/fmt/include/fmt/printf.h"
#include "third_party/fmt/include/fmt/core.h"

#include <filesystem>
#include <fstream>

bool isFileExists(std::string_view file)
{
	if (!fs::exists(file))
	{
		engine::util::printError("{} is not exist", file);
		return false;
	}
	return true;
}

void copyFile(std::string_view src, std::string_view dst)
{
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
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
	if(isFileExists(m_path))
	{
        std::ifstream file(m_path);
        std::ostringstream s_str;
        s_str << file.rdbuf();
        return s_str.str();
	}
	return "";
}
};

