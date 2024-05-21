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
#include <vector>

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
        Input,
        Output
    };

    template <OperationType OType = OperationType::Output, SerializationType SType = SerializationType::BinaryFile>
    class TA_Serialization
    {
    public:
        TA_Serialization()
        {

        }

        TA_Serialization(const std::string &path)
        {
            openFile(path);
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
            openFile(path);
        }

        template <CustomType T>
        TA_Serialization & operator << (const T &t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            callOperations(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::operationSize()> {});
            return *this;
        }

        template <CustomType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            callOperations(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::operationSize()> {});
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serialization & operator << (T t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            if(TA_EndianConversion::isSystemLittleEndian())
                t = TA_EndianConversion::swapEndian(t);
            m_fileSream.write(reinterpret_cast<const char *>(&t), sizeof(t));
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            m_fileSream.read(reinterpret_cast<char *>(&t), sizeof(t));
            if(TA_EndianConversion::isSystemLittleEndian())
                t = TA_EndianConversion::swapEndian(t);
            return *this;
        }

        template <StdContainerType T>
        TA_Serialization & operator << (const T &t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            *this << std::ranges::distance(t);
            std::ranges::for_each(std::as_const(t), [this](const T::value_type &val) {
                *this << val;
            });
            return *this;
        }

        template <StdAdaptorType T>
        TA_Serialization & operator << (const T &t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            *this << std::ranges::size(t);
            T copyAdaptor = t;
            if constexpr(std::is_same_v<std::stack<typename T::value_type, typename T::container_type>, T>)
            {
                while(!copyAdaptor.empty())
                {
                    *this << copyAdaptor.top();
                    copyAdaptor.pop();
                }
            }
            else if constexpr(std::is_same_v<std::queue<typename T::value_type, typename T::container_type>, T>)
            {
                while(!copyAdaptor.empty())
                {
                    *this << copyAdaptor.front();
                    copyAdaptor.pop();
                }
            }
            else if constexpr(std::is_same_v<std::priority_queue<typename T::value_type, typename T::container_type, typename T::value_compare>, T>)
            {
                while(!copyAdaptor.empty())
                {
                    *this << copyAdaptor.top();
                    copyAdaptor.pop();
                }
            }
            return *this;
        }

        template <typename T, std::size_t N>
        TA_Serialization & operator >> (std::array<T, N> &array)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            std::size_t size;
            *this >> size;
            for(auto &v : array)
            {
                *this >> v;
            }
            return *this;
        }

        template <StdContainerType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            std::size_t size {};
            *this >> size;
            if constexpr(std::is_same_v<std::vector<typename T::value_type>, T> ||
                          std::is_same_v<std::list<typename T::value_type>, T> ||
                          std::is_same_v<std::deque<typename T::value_type>, T>)
            {
                for(auto i = 0;i < size;++i)
                {
                    typename T::value_type val;
                    *this >> val;
                    t.emplace_back(std::move(val));
                }
            }
            else if constexpr(std::is_same_v<std::forward_list<typename T::value_type>, T>)
            {
                typename std::forward_list<typename T::value_type>::iterator beginIter = t.before_begin();
                for(auto i = 0;i < size;++i)
                {
                    typename T::value_type val;
                    *this >> val;
                    beginIter = t.emplace_after(beginIter, std::move(val));
                }
            }
            else if constexpr(std::is_same_v<std::map<typename T::key_type, typename T::mapped_type>, T> ||
                               std::is_same_v<std::unordered_map<typename T::key_type, typename T::mapped_type>, T> ||
                               std::is_same_v<std::multimap<typename T::key_type, typename T::mapped_type>, T> ||
                               std::is_same_v<std::unordered_multimap<typename T::key_type, typename T::mapped_type>, T>)
            {
                for(auto i = 0;i < size;++i)
                {
                    std::pair<typename T::key_type, typename T::mapped_type> val;
                    *this >> val;
                    t.emplace(std::move(val));
                }
            }
            else if constexpr(std::is_same_v<std::set<typename T::key_type>, T> ||
                               std::is_same_v<std::unordered_set<typename T::key_type>, T> ||
                               std::is_same_v<std::multiset<typename T::key_type>, T> ||
                               std::is_same_v<std::unordered_multiset<typename T::key_type>, T>)
            {
                for(auto i = 0;i < size;++i)
                {
                    typename T::key_type val;
                    *this >> val;
                    t.emplace(std::move(val));
                }
            }
            return *this;
        }

        template <StdAdaptorType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            std::size_t size {};
            *this >> size;
            if constexpr(std::is_same_v<std::queue<typename T::value_type, typename T::container_type>, T>)
            {
                for(auto i = 0;i < size;++i)
                {
                    typename T::value_type val;
                    *this >> val;
                    t.emplace(std::move(val));
                }
            }
            else if constexpr(std::is_same_v<std::stack<typename T::value_type, typename T::container_type>, T>)
            {
                std::deque<typename T::value_type> temp;
                for(auto i = 0;i < size;++i)
                {
                    typename T::value_type val;
                    *this >> val;
                    temp.push_front(std::move(val));
                }
                t.push_range(temp);
            }
            else if constexpr(std::is_same_v<std::priority_queue<typename T::value_type, typename T::container_type, typename T::value_compare>, T>)
            {
                for(auto i = 0;i < size;++i)
                {
                    typename T::value_type val;
                    *this >> val;
                    t.emplace(std::move(val));
                }
            }
            return *this;
        }

        template <typename K, typename V>
        TA_Serialization & operator << (const std::pair<K, V> &pair)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            return *this << pair.first << pair.second;
        }

        template <typename K, typename V>
        TA_Serialization & operator >> (std::pair<K, V> &pair)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            return *this >> pair.first >> pair.second;
        }

        template <RawPtr T>
        TA_Serialization & operator << (const T &t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            return *this << *t;
        }

        template <RawPtr T>
        TA_Serialization & operator << (T &&t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            if(t)
            {
                return *this << *t;
            }
            return *this;;
        }

        template <RawPtr T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            if(!t)
                return *this;
            return *this >> *t;
        }

        template<typename T, int N>
        TA_Serialization & operator << (const T (&a)[N])
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            for(int i = 0;i < N;++i)
            {
                *this << a[i];
            }
            return *this;
        }

        template<typename T, int N>
        TA_Serialization & operator >> (T (&a)[N])
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            for(int i = 0;i < N;++i)
            {
                *this >> a[i];
            }
            return *this;
        }

        template <EnumType T>
        TA_Serialization & operator << (T t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            *this << static_cast<uint8_t>(t);
            return *this;
        }

        template <EnumType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            uint8_t val {};
            *this >> val;
            t = static_cast<T>(val);
            return *this;
        }

        TA_Serialization & operator << (std::nullptr_t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            return *this;
        }

        TA_Serialization & operator >> (std::nullptr_t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            return *this;
        }

    private:
        template <typename T>
        constexpr void callOperations(const T &t, std::index_sequence<> = {})
        {
            return;
        }

        template <typename T, std::size_t IDX0, std::size_t ...IDXS>
        constexpr void callOperations (T &t, std::index_sequence<IDX0, IDXS...> = {})
        {
            using Rt = std::remove_cvref_t<T>;
            if constexpr(OType == OperationType::Output)
            {
                *this << std::invoke(std::get<0>(Reflex::TA_TypeInfo<Rt>::template findPropertyOperation<IDX0>()), t);
                callOperations(t, std::index_sequence<IDXS...> {});
            }
            else
            {
                using ParaType =  typename FunctionTypeInfo<decltype(std::get<0>(Reflex::TA_TypeInfo<Rt>::template findPropertyOperation<IDX0>()))>::RetType;
                using RPara = std::remove_cvref_t<ParaType>;
                RPara para {};
                if constexpr(std::is_pointer_v<decltype(para)>)
                {
                    para = new std::remove_cvref_t<decltype(*para)>();
                }
                *this >> para;
                (t.*std::get<1>(Reflex::TA_TypeInfo<Rt>::template findPropertyOperation<IDX0>()))(para);
                callOperations(t, std::index_sequence<IDXS...> {});
            }
        }

    private:
        bool openFile(const std::string &filePath)
        {
            if(m_fileSream.is_open())
            {
                m_fileSream.close();
            }
            if constexpr(OType == OperationType::Input)
            {
                m_fileSream.open(filePath, std::ios::binary | std::ios::in);
            }
            else
            {
                m_fileSream.open(filePath, std::ios::binary | std::ios::out);
            }
            if (!m_fileSream || !m_fileSream.is_open())
            {
                CoreAsync::TA_CommonTools::debugInfo("Cannot open the file.\n");
                return false;
            }
            return true;
        }


    private:
        std::fstream m_fileSream;

    };
}

#endif // TA_SERIALIZATION_H