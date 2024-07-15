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
        explicit TA_Serialization(std::size_t version = 1) : m_version(version)
        {

        }

        TA_Serialization(const std::string &path, std::size_t version = 1) : m_version(version)
        {
            open(path);
        }

        ~TA_Serialization()
        {
            close();
        }

        TA_Serialization(const TA_Serialization &serialzation) = delete;
        TA_Serialization(TA_Serialization &&serialzation) = delete;

        TA_Serialization & operator = (const TA_Serialization &serialzation) = delete;
        TA_Serialization & operator = (TA_Serialization &&serialzation) = delete;

        bool open(const std::string &filePath)
        {
            close();
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
            if constexpr(OType == OperationType::Input)
            {
                m_fileSream.read(reinterpret_cast<char *>(&m_version), sizeof(m_version));
            }
            else
            {
                m_fileSream.write(reinterpret_cast<const char *>(&m_version), sizeof(m_version));
            }
            return true;
        }

        void close()
        {
            if(m_fileSream.is_open())
            {
                m_fileSream.close();
            }
        }

        std::size_t version() const
        {
            return m_version;
        }

        template <CustomType T>
        TA_Serialization & operator << (const T &t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            callProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size> {});
            return *this;
        }

        template <CustomType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            callProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size> {});
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serialization & operator << (T t)
        {
            static_assert(OType == OperationType::Output, "The operation type isn't OUTPUT");
            if(m_fileSream.is_open())
            {
                if(TA_EndianConversion::isSystemLittleEndian())
                    t = TA_EndianConversion::swapEndian(t);
                m_fileSream.write(reinterpret_cast<const char *>(&t), sizeof(t));
            }
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serialization & operator >> (T &t)
        {
            static_assert(OType == OperationType::Input, "The operation type isn't INPUT");
            if(m_fileSream.is_open())
            {
                m_fileSream.read(reinterpret_cast<char *>(&t), sizeof(t));
                if(TA_EndianConversion::isSystemLittleEndian())
                    t = TA_EndianConversion::swapEndian(t);
            }
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
            static_assert(OType == OperationType::Output, "The operation type isn't INPUT");
            return *this;
        }

    private:
        template <typename T>
        constexpr void callProperty(const T &t, std::index_sequence<> = {})
        {
            return;
        }

        template <typename T, std::size_t IDX0, std::size_t ...IDXS>
        constexpr void callProperty (T &t, std::index_sequence<IDX0, IDXS...> = std::make_index_sequence<Reflex::TA_TypeInfo<std::remove_cvref_t<T>>::TA_PropertyInfos::size> {})
        {
            using Rt = std::remove_cvref_t<T>;
            using Properties = Reflex::TA_TypeInfo<Rt>::TA_PropertyInfos::List;
            if constexpr(OType == OperationType::Output)
            {
                if(m_version >= std::tuple_element_t<1, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>::m_value)
                {
                    *this << Reflex::TA_TypeInfo<Rt>::invoke(std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type> {}, t);
                }
            }
            else
            {
                if(std::tuple_element_t<1, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>::m_value <= m_version)
                {
                    using ValType = std::remove_pointer_t<typename VariableTypeInfo<std::remove_cvref_t<decltype(CoreAsync::Reflex::TA_TypeInfo<Rt>::findType(std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type> {}))>>::RetType>;
                    ValType val {};
                    // std::cout << typeid(ValType).name() << std::endl;
                    *this >> val;
                    Reflex::TA_TypeInfo<Rt>::update(t, val, std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type> {});
                }
            }
            callProperty(t, std::index_sequence<IDXS...> {});
        }

    private:
        std::fstream m_fileSream;
        std::size_t m_version;

    };
}

#endif // TA_SERIALIZATION_H
