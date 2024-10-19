#include "CommandLine.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/system/config.h"
#include <string>

std::vector<std::string> splitString(const std::string& str)
{
    std::vector<std::string> output;
    int start= 0;
    for (int i = 0; i < str.length(); i++)
    {
        if(str[i]!=' ')
            continue;
        else if(start!= i)
        {
            output.push_back(std::string(str.begin() + start, str.begin() + i));
            start = i+1;
        }
        else
        {
            start = i+1;
        }
    }
    if(start!=str.length())
        output.push_back(std::string(str.begin() + start, str.end()));
        
    return output;
}

CommandLine::CommandLine()
{
    LogScope("CommandLine initialization");
    system::Filepath filepath = config::g_state.homeDir/"cmdline.txt";
    std::ifstream file(filepath.str());
    if(!file.is_open())
        return;

    std::string tempString;
    std::vector<std::string> fileStr;
    fileStr.reserve(cmdArgsSize);
    while (std::getline(file, tempString))
        fileStr.push_back(tempString);

    for (int i = 0; i < cmdArgsSize; i++)
    {
        if (fileStr[i].find(commandLineStr[i])!=std::string::npos)
        {
            std::vector<std::string> output = splitString(fileStr[i]);
            if (output.size() == 2)
            {
                args[i] = std::stoi(output[1]);
                engine::util::printInfo("command line option {} {}", 
                    commandLineStr[i].c_str(), args[i].value());
            }
            else
                engine::util::printError("incorrect command line arguments {}", fileStr[i].c_str());
        }
    }
}
