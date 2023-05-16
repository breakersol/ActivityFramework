/*
 * Copyright [2023] [Shuang Zhu / Sol]
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

#include <list>

namespace CoreAsync
{
	class TA_CommonTools
	{
	public:
        template <typename T, typename Container = std::list<std::decay_t<T> > >
		static auto at(const Container &container, std::size_t index)
		{
			if (index >= container.size())
			{
				return T{};
			}
            typename Container::const_iterator pIter = container.begin();
            for (std::size_t i = 0; i < index; i++)
			{
				pIter++;
			}
			return *pIter;
		}

		template <typename T, typename Container = std::list<std::decay_t<T> > >
		static bool replace(Container &container, std::size_t index, T &&elem)
		{
			if (index >= container.size())
			{
				return false;
			}
            typename Container::iterator pIter = container.begin();
            for (std::size_t i = 0; i < index; i++)
			{
				pIter++;
			}
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
            for (std::size_t i = 0; i < index; ++i)
			{
				pIter++;
			}
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
	};
}

#endif
