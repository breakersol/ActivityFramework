/*
 * Copyright [2024] [Shuang Zhu / Sol]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
