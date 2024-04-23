#ifndef TA_SERIALIZATION_H
#define TA_SERIALIZATION_H

#include <fstream>
#include <type_traits>
#include <forward_list>
#include <deque>
#include <map>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>

#include "TA_CommonTools.h"
#include "TA_EndianConversion.h"
#include "TA_TypeFilter.h"
#include "TA_MetaReflex.h"

namespace CoreAsync
{
    enum class SerializationType
    {
        BinaryFile,
        JsonFile
    };

    enum class OperationType
    {
        Read,
        Write
    };

    template <SerializationType SType = SerializationType::BinaryFile>
    class TA_Serialization
    {
        template <typename P>
        using NormalPtr = std::enable_if_t<std::is_pointer_v<std::decay_t<P>>, std::decay_t<P>>;

        template <typename E>
        using EnumType = std::enable_if_t<std::is_enum_v<std::decay_t<E>>, E>;
    public:
        TA_Serialization()
        {

        }

        TA_Serialization(const std::string &path)
        {
            m_fileSream.open(path, std::ios::binary);
            if (!m_fileSream || !m_fileSream.is_open())
            {
                CoreAsync::TA_CommonTools::debugInfo("Cannot open the file for writing.\n");
                return;
            }
        }

        ~TA_Serialization()
        {

        }

        TA_Serialization(const TA_Serialization &serialzation) = delete;
        TA_Serialization(TA_Serialization &&serialzation) = delete;

        TA_Serialization & operator = (const TA_Serialization &serialzation) = delete;
        TA_Serialization & operator = (TA_Serialization &&serialzation) = delete;

        void setFilePath(const std::string &path)
        {
            if(m_fileSream.is_open())
            {
                m_fileSream.close();
            }
            m_fileSream.open(path, std::ios::binary);
            if (!m_fileSream || !m_fileSream.is_open())
            {
                CoreAsync::TA_CommonTools::debugInfo("Cannot open the file for writing.\n");
                return;
            }
        }

        template <CustomType T>
        TA_Serialization & operator << (const T &t)
        {
            callOperations<OperationType::Read>(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::operationSize()> {});
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serialization & operator << (T t)
        {
            if(TA_EndianConversion::isSystemLittleEndian())
                t = TA_EndianConversion::swapEndian(t);
            m_fileSream.write(reinterpret_cast<const char*>(&t), sizeof(t));
            return *this;
        }

        template <StdContainerType T, int64_t N = -1>
        TA_Serialization & operator << (const T &t)
        {
            *this << t.size();
            if constexpr(std::is_same_v<std::vector<typename T::value_type>, T> ||
                         std::is_same_v<std::list<typename T::value_type>, T> ||
                         std::is_same_v<std::forward_list<typename T::value_type>, T> ||
                         std::is_same_v<std::deque<typename T::value_type>, T> ||
                         N >= 0)
            {
                for(auto &element : t)
                {
                    *this << element;
                }
            }
            else if constexpr(std::is_same_v<std::map<typename T::key_type, typename T::mapped_type>, T> ||
                              std::is_same_v<std::unordered_map<typename T::key_type, typename T::mapped_type>, T> ||
                              std::is_same_v<std::multimap<typename T::key_type, typename T::mapped_type>, T> ||
                              std::is_same_v<std::unordered_multimap<typename T::key_type, typename T::mapped_type>, T>)
            {
                for(auto &p : t)
                {
                    *this << p;
                }
            }
            else if constexpr(std::is_same_v<std::set<typename T::key_type>, T> ||
                             std::is_same_v<std::unordered_set<typename T::key_type>, T> ||
                             std::is_same_v<std::multiset<typename T::key_type>, T> ||
                             std::is_same_v<std::unordered_multiset<typename T::key_type>, T>)
            {
                for(auto &p : t)
                {
                    *this << p;
                }
            }
            else if constexpr(std::is_same_v<std::stack<typename T::value_type>, T>)
            {
                std::vector<typename T::value_type> cache;
                while(!t.empty())
                {
                    cache.emplace_back(t.top());
                    *this << t.top();
                    t.pop();
                }
                while(!cache.empty())
                {
                    t.emplace(cache.back());
                    cache.pop_back();
                }
            }
            else if constexpr(std::is_same_v<std::queue<typename T::value_type>, T>)
            {
                std::queue<typename T::value_type> cache;
                while(!t.empty())
                {
                    cache.emplace(t.front());
                    *this << t.front();
                    t.pop();
                }
                cache.swap(t);
            }
            else if constexpr(std::is_same_v<std::priority_queue<typename T::value_type>, T>)
            {
                std::priority_queue<typename T::value_type> cache;
                while(!t.empty())
                {
                    cache.emplace(t.top());
                    *this << t.top();
                    t.pop();
                }
                cache.swap(t);
            }
        }

        template <typename K, typename V>
        TA_Serialization & operator << (const std::pair<K, V> &pair)
        {
            return *this << pair.first << pair.second;
        }

        template <typename T, std::size_t Length = 1>
        TA_Serialization & operator << (const NormalPtr<T> &t)
        {
            return *this << Length << (*t);
        }

        template<typename T, int N>
        TA_Serialization & operator << (T (&a)[N])
        {
            for(int i = 0;i < N;++i)
            {
                *this << a[i];
            }
            return *this;
        }

        template <typename T>
        TA_Serialization & operator << (EnumType<T> t)
        {
            *this << static_cast<uint8_t>(t);
            return *this;
        }

        TA_Serialization & operator << (std::nullptr_t)
        {
            return *this;
        }

    private:
        template <OperationType Operation, typename T>
        constexpr void callOperations(const T &t, std::index_sequence<> = {})
        {
            return;
        }

        template <OperationType Operation, typename T, std::size_t IDX0, std::size_t ...IDXS>
        constexpr void callOperations (const T &t, std::index_sequence<IDX0, IDXS...> = {})
        {
            if constexpr(Operation == OperationType::Read)
            {
                *this << std::invoke(std::get<0>(Reflex::TA_TypeInfo<T>::template findPropertyOperation<IDX0>()), t);
                callOperations(t, std::index_sequence<IDXS...> {});
            }
            else
            {

            }
        }



    private:
        std::ofstream m_fileSream;

    };
}

#endif // TA_SERIALIZATION_H
