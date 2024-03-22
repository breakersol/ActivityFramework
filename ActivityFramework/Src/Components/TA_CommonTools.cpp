#include "TA_CommonTools.h"

namespace  CoreAsync {

std::vector<std::string> StringUtils::split(const std::string &source, char delimiter)
{
    std::vector<std::string> res;
    std::string tr;
    std::istringstream tokenStream(source);
    while(std::getline(tokenStream, tr, delimiter))
    {
        res.push_back(tr);
    }
    return res;
}

}
