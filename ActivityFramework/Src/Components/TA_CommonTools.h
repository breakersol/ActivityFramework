/*
 * Copyright [2025] [Shuang Zhu / Sol]
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

#ifndef TA_COMMONTOOLS_H
#define TA_COMMONTOOLS_H

#include <cstdint>
#include <list>
#include <map>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <sstream>
#include <format>
#include <concepts>

#include "TA_ActivityFramework_global.h"

namespace CoreAsync {
class TA_CommonTools {
  public:
    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static auto at(const Container &container, std::size_t index) -> std::decay_t<T> {
        typename Container::const_iterator pIter = container.begin();
        std::advance(pIter, index);
        return *pIter;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static auto ref(Container &container, std::size_t index) -> std::decay_t<T> & {
        typename Container::iterator pIter = container.begin();
        std::advance(pIter, index);
        return *pIter;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static auto insert(Container &container, std::size_t index, const T &val) {
        typename Container::const_iterator pIter = container.begin();
        std::advance(pIter, index);
        return container.insert(pIter, val);
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static auto insert(Container &container, std::size_t index, T &&val) {
        typename Container::const_iterator pIter = container.begin();
        std::advance(pIter, index);
        return container.insert(pIter, val);
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool replace(Container &container, std::size_t index, const T &elem) {
        if (index >= container.size()) {
            return false;
        }
        typename Container::iterator pIter = container.begin();
        std::advance(pIter, index);
        *pIter = elem;
        return true;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool replace(Container &container, std::size_t index, T &&elem) {
        if (index >= container.size()) {
            return false;
        }
        typename Container::iterator pIter = container.begin();
        std::advance(pIter, index);
        *pIter = elem;
        return true;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool remove(Container &container, std::size_t index) {
        if (index >= container.size()) {
            return false;
        }
        typename Container::iterator pIter{container.begin()};
        std::advance(pIter, index);
        if constexpr (std::is_pointer_v<decltype(*pIter)>) {
            if (*pIter) {
                delete *pIter;
                *pIter = nullptr;
            }
        }
        container.erase(pIter);
        return true;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static T takeAt(Container &container, std::size_t index) {
        if (index >= container.size()) {
            return T{};
        }
        typename Container::iterator pIter{container.begin()};
        std::advance(pIter, index);
        T res{*pIter};
        if constexpr (std::is_pointer_v<decltype(*pIter)>) {
            if (*pIter) {
                delete *pIter;
                *pIter = nullptr;
            }
        }
        container.erase(pIter);
        return res;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool removeOne(Container &container, const T &t) {
        auto iter = std::ranges::find(container, t);
        if (iter == container.end()) {
            return false;
        }
        container.erase(iter);
        return true;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool removeOne(Container &container, T &&t) {
        auto iter = std::ranges::find(container, t);
        if (iter == container.end()) {
            return false;
        }
        container.erase(iter);
        return true;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool contains(const Container &container, const T &t) {
        return std::ranges::find(container, t) != container.end();
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static bool contains(const Container &container, T &&t) {
        return std::ranges::find(container, t) != container.end();
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
    static int indexOf(const Container &container, const T &t) {
        int index{0};
        for (auto it = container.begin(); it != container.end(); ++it, ++index) {
            if (*it == t) {
                return index;
            }
        }
        return -1;
    }

    template <typename T, typename Container = std::list<std::decay_t<T>>>
static Container subRanges(const Container &container, std::size_t startIndex, std::size_t length) {
    return container 
         | std::views::drop(startIndex) 
         | std::views::take(length)
         | std::ranges::to<Container>(); 
}

    template <typename Text, typename... Paras> static void debugInfo(Text text, Paras &&...paras) {
#ifdef DEBUG_INFO_ON
        std::printf(std::string_view{Text::data()}.data(), std::forward<Paras>(paras)...);
#endif
    }

    template <std::integral Num>
    static std::string decimalToBinary(Num n) {
        return std::format("{:b}", n);
    }
};

class MapUtils {
  public:
    static auto keys(const auto &map, const auto &val) {
        return map | std::views::filter([&val](const auto &pair) { return pair.second == val; })
         | std::views::keys | std::ranges::to<std::list>();
    }

    static auto values(const auto &map, const auto &key) {
        return map | std::views::filter([&key](const auto &pair) { return pair.first == key; })
         | std::views::values | std::ranges::to<std::list>();
    }

    static auto value(const auto &map, const auto &key) {
        using valueType = typename std::decay_t<decltype(map)>::mapped_type;
        if(auto iter = map.find(key); iter != map.end()) {
            return iter->second;
        }
        return valueType{};
    }

    static auto remove(auto &map, const auto &key) {
        return map.erase(key);
    }

    static auto first(const auto &map) {
        return *map.begin();
    }

    static auto last(const auto &map) {
        return *std::prev(map.end());
    }

    template <typename Map> static auto key(const Map &map, const typename std::decay_t<Map>::mapped_type &val) {
        using keyType = typename std::decay_t<Map>::key_type;
        auto iter = std::ranges::find_if(map, [&val](const auto &pair) { return pair.second == val; });
        if (iter != map.end()) {
            return iter->first;
        }
        return keyType{};
    }

    static auto contains(const auto &map, const auto &key) {
        return map.contains(key);
    }
};

class StringUtils {
  public:
    static ACTIVITY_FRAMEWORK_EXPORT std::vector<std::string> split(const std::string &source, char delimiter);
};
} // namespace CoreAsync

#endif
