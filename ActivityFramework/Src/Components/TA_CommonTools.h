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

#ifndef TA_COMMONTOOLS_H
#define TA_COMMONTOOLS_H

#include <cstdint>
#include <list>
#include <map>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <sstream>

#include "TA_MarcoDefine.h"
#include "TA_ActivityFramework_global.h"

namespace CoreAsync
{
	class TA_CommonTools
	{
	public:
        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static auto at(const Container &container, std::size_t index) -> std::decay_t<T>
		{
            typename Container::const_iterator pIter = container.begin();
            std::advance(pIter, index);
			return *pIter;
		}

        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static auto ref(Container &container, std::size_t index) -> std::decay_t<T> &
        {
            typename Container::iterator pIter = container.begin();
            std::advance(pIter, index);
            return *pIter;
        }

        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static auto insert(Container &container, std::size_t index, const T &val)
        {
            typename Container::const_iterator pIter = container.begin();
            std::advance(pIter, index);
            return container.insert(pIter, val);
        }

        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static auto insert(Container &container, std::size_t index, T &&val)
        {
            typename Container::const_iterator pIter = container.begin();
            std::advance(pIter, index);
            return container.insert(pIter, val);
        }

        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static bool replace(Container &container, std::size_t index, const T &elem)
        {
            if (index >= container.size())
            {
                return false;
            }
            typename Container::iterator pIter = container.begin();
            std::advance(pIter, index);
            *pIter = elem;
            return true;
        }

		template <typename T, typename Container = std::list<std::decay_t<T> > >
		static bool replace(Container &container, std::size_t index, T &&elem)
		{
			if (index >= container.size())
			{
				return false;
			}
            typename Container::iterator pIter = container.begin();
            std::advance(pIter, index);
			*pIter = elem;
			return true;
		}

		template <typename T, typename Container = std::list<std::decay_t<T> > >
		static bool remove(Container &container, std::size_t index)
		{
			if (index >= container.size())
			{
				return false;
			}
            typename Container::iterator pIter{ container.begin() };
            std::advance(pIter, index);
            if constexpr (std::is_pointer_v<decltype(*pIter)>)
			{
				if (*pIter)
				{
					delete* pIter;
					*pIter = nullptr;
				}
			}
			container.erase(pIter);
			return true;
		}

        template <typename T, typename Container = std::list<std::decay_t<T> > >
        static T takeAt(Container &container, std::size_t index)
        {
            if (index >= container.size())
            {
                return T {};
            }
            typename Container::iterator pIter{ container.begin() };
            std::advance(pIter, index);
            T res {*pIter};
            if constexpr (std::is_pointer_v<decltype(*pIter)>)
            {
                if (*pIter)
                {
                    delete* pIter;
                    *pIter = nullptr;
                }
            }
            container.erase(pIter);
            return res;
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static bool removeOne(Container &container, const T &t)
        {
            for(auto iter = container.begin();iter != container.end();++iter)
            {
                if(*iter == t)
                {
                    container.erase(iter);
                    return true;
                }
            }
            return false;
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static bool removeOne(Container &container, T &&t)
        {
            for(auto iter = container.begin();iter != container.end();++iter)
            {
                if(*iter == t)
                {
                    container.erase(iter);
                    return true;
                }
            }
            return false;
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static bool contains(const Container &container, const T &t)
        {
            return std::find(container.begin(), container.end(), t) != container.end();
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static bool contains(const Container &container, T &&t)
        {
            return std::find(container.begin(), container.end(), t) != container.end();
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static int indexOf(const Container &container, const T &t)
        {
            int index {0};
            for (auto it = container.begin(); it != container.end(); ++it, ++index)
            {
                if (*it == t)
                {
                    return index;
                }
            }
            return -1;
        }

        template <typename T, typename Container = std::list<std::decay_t<T>>>
        static Container mid(const Container &container, std::size_t startIndex, std::size_t length)
        {
            auto start {container.begin()};
            auto end {container.begin()};
            std::advance(start, startIndex);
            std::advance(end, startIndex + length - 1);
            Container res;
            res.assign(start, end);
            return res;
        }

        template <typename Text, typename ...Paras>
        static void debugInfo(Text text, Paras &&...paras)
        {
#ifdef  DEBUG_INFO_ON
            std::printf(std::string_view{Text::data()}.data(), std::forward<Paras>(paras)...);
#endif
        }

        template <typename Num = std::int_fast64_t>
        static std::string decimalToBinary(Num n) {
            static std::string zero {"0"}, one {"1"};
            if (n == 0) {
                return zero;
            }

            std::string binary = "";
            while (n > 0) {
                binary = (n % 2 == 0 ? zero : one) + binary;
                n /= 2;
            }

            return binary;
        }
	};

    class MapUtils
    {
    public:
        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<Key> keys(const MapType<Key, T, Cmp, Allocator> &map)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<Key> keys(const MapType<Key, T, Hasher, Eq, Allocator> &map)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<Key> keys(const MapType<Key, T, Cmp, Allocator> &map, const T &val)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                if(val == v)
                    list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<Key> keys(const MapType<Key, T, Cmp, Allocator> &map, T &&val)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                if(val == v)
                    list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<Key> keys(const MapType<Key, T, Hasher, Eq, Allocator> &map, const T &val)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                if(val == v)
                    list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<Key> keys(const MapType<Key, T, Hasher, Eq, Allocator> &map, T &&val)
        {
            std::list<Key> list;
            for(auto &&[k,v] : map)
            {
                if(val == v)
                    list.emplace_back(k);
            }
            return list;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static T value(const MapType<Key, T, Cmp, Allocator> &map, const Key &k)
        {
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                return iter->second;
            }
            return T {};
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static T value(const MapType<Key, T, Cmp, Allocator> &map, Key &&k)
        {
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                return iter->second;
            }
            return T {};
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static T value(const MapType<Key, T, Hasher, Eq, Allocator> &map, const Key &k)
        {
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                return iter->second;
            }
            return T {};
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static T value(const MapType<Key, T, Hasher, Eq, Allocator> &map, Key &&k)
        {
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                return iter->second;
            }
            return T {};
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<T> values(const MapType<Key, T, Cmp, Allocator> &map, const Key &k)
        {
            std::list<T> vec;
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                vec.emplace_back(iter->second);
            }
            return vec;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<T> values(const MapType<Key, T, Cmp, Allocator> &map, Key &&k)
        {
            std::list<T> vec;
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                vec.emplace_back(iter->second);
            }
            return vec;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<T> values(const MapType<Key, T, Hasher, Eq, Allocator> &map, const Key &k)
        {
            std::list<T> vec;
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                vec.emplace_back(iter->second);
            }
            return vec;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<T> values(const MapType<Key, T, Hasher, Eq, Allocator> &map, Key &&k)
        {
            std::list<T> vec;
            auto &&[start, end] = map.equal_range(k);
            for(auto iter = start; iter != end;++iter)
            {
                vec.emplace_back(iter->second);
            }
            return vec;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>> ,template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::list<T> values(const MapType<Key, T, Cmp, Allocator> &map)
        {
            std::list<T> vec;
            for(auto &[k, v] : map)
            {
                vec.emplace_back(v);
            }
            return vec;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::list<T> values(const MapType<Key, T, Hasher, Eq, Allocator> &map)
        {
            std::list<T> vec;
            for(auto &[k, v] : map)
            {
                vec.emplace_back(v);
            }
            return vec;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static void remove(MapType<Key, T, Cmp, Allocator> &map, const Key &k)
        {
            for(typename std::decay_t<decltype(map)>::iterator pIter = map.begin(); pIter != map.end();)
            {
                if(pIter->first == k)
                {
                    pIter = map.erase(pIter);
                }
                else
                    ++pIter;
            }
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static void remove(MapType<Key, T, Cmp, Allocator> &map, Key &&k)
        {
            for(typename std::decay_t<decltype(map)>::iterator pIter = map.begin(); pIter != map.end();)
            {
                if(pIter->first == k)
                {
                    pIter = map.erase(pIter);
                }
                else
                    ++pIter;
            }
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static void remove(MapType<Key, T, Hasher, Eq, Allocator> &map, const Key &k)
        {
            for(typename std::decay_t<decltype(map)>::iterator pIter = map.begin(); pIter != map.end();)
            {
                if(pIter->first == k)
                {
                    pIter = map.erase(pIter);
                }
                else
                    ++pIter;
            }
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static void remove(MapType<Key, T, Hasher, Eq, Allocator> &map, Key &&k)
        {
            for(typename std::decay_t<decltype(map)>::iterator pIter = map.begin(); pIter != map.end();)
            {
                if(pIter->first == k)
                {
                    pIter = map.erase(pIter);
                }
                else
                    ++pIter;
            }
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::pair<Key, T> first(const MapType<Key, T, Cmp, Allocator> &map)
        {
            return *map.begin();
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::pair<Key, T> first(const MapType<Key, T, Hasher, Eq, Allocator> &map)
        {
            return *map.begin();
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static std::pair<Key, T> last(const MapType<Key, T, Cmp, Allocator> &map)
        {
            auto iter {map.begin()};
            std::advance(iter, map.size() - 1);
            return *iter;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static std::pair<Key, T> last(const MapType<Key, T, Hasher, Eq, Allocator> &map)
        {
            auto iter {map.begin()};
            std::advance(iter, map.size() - 1);
            return *iter;   
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static Key key(const MapType<Key, T, Cmp, Allocator> &map, const T &val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val)
                    return k;
            }
            return Key {};
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static Key key(const MapType<Key, T, Cmp, Allocator> &map, T &&val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val)
                    return k;
            }
            return Key {};
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static Key key(const MapType<Key, T, Hasher, Eq, Allocator> &map, const T &val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val)
                    return k;
            }
            return Key {};
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static Key key(const MapType<Key, T, Hasher, Eq, Allocator> &map, T &&val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val)
                    return k;
            }
            return Key {};
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static bool contains(const MapType<Key, T, Cmp, Allocator> &map, const Key &key, const T &val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val && k == key)
                    return true;
            }
            return false;
        }

        template <typename Key, typename T, typename Cmp = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename C, typename A> class MapType = std::map>
        static bool contains(const MapType<Key, T, Cmp, Allocator> &map, Key &&key, T &&val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val && k == key)
                    return true;
            }
            return false;
        }


        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static bool contains(const MapType<Key, T, Hasher, Eq, Allocator> &map, const Key &key, const T &val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val && k == key)
                    return true;
            }
            return false;
        }

        template <typename Key, typename T, typename Hasher = std::hash<Key>, typename Eq = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>, template <typename K, typename V, typename H, typename E, typename A> class MapType = std::unordered_map>
        static bool contains(const MapType<Key, T, Hasher, Eq, Allocator> &map, Key &&key, T &&val)
        {
            for(auto &&[k, v] : map)
            {
                if(v == val && k == key)
                    return true;
            }
            return false;
        }
    };

    class StringUtils
    {
    public:
        static ACTIVITY_FRAMEWORK_EXPORT std::vector<std::string> split(const std::string &source, char delimiter);
    };
}

#endif
