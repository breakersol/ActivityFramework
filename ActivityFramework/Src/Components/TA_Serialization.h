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
#include <utility>
#include <vector>
#include <algorithm>

#include "TA_CommonTools.h"
#include "TA_EndianConversion.h"
#include "TA_TypeFilter.h"
#include "TA_MetaReflex.h"

namespace CoreAsync
{
    enum class SerializationType
    {
        BinaryFile,
        // JsonFile
    };

    enum class OperationType
    {
        Deserialization,
        Serialization
    };

    template <OperationType OType = OperationType::Serialization , SerializationType SType = SerializationType::BinaryFile>
    class TA_Serializer
    {
    public:
        explicit TA_Serializer(std::size_t version = 1) : m_version(version)
        {

        }

        TA_Serializer(const std::string &path, std::size_t version = 1) : m_version(version)
        {
            open(path);
        }

        ~TA_Serializer()
        {
            close();
        }

        TA_Serializer(const TA_Serializer &serialzation) = delete;
        TA_Serializer(TA_Serializer &&serialzation) = delete;

        TA_Serializer & operator = (const TA_Serializer &serialzation) = delete;
        TA_Serializer & operator = (TA_Serializer &&serialzation) = delete;

        bool open(const std::string &filePath)
        {
            close();
            if constexpr(OType == OperationType::Deserialization)
            {
                m_fileStream.open(filePath, std::ios::binary | std::ios::in);
            }
            else
            {
                m_fileStream.open(filePath, std::ios::binary | std::ios::out);
            }
            if (!m_fileStream || !m_fileStream.is_open())
            {
                CoreAsync::TA_CommonTools::debugInfo(META_STRING("Cannot open the file.\n"));
                return false;
            }
            if constexpr(OType == OperationType::Deserialization)
            {
                m_fileStream.read(reinterpret_cast<char *>(&m_version), sizeof(m_version));
            }
            else
            {
                m_fileStream.write(reinterpret_cast<const char *>(&m_version), sizeof(m_version));
            }
            return true;
        }

        void close()
        {
            if(m_fileStream.is_open())
            {
                m_fileStream.close();
            }
        }

        std::size_t version() const
        {
            return m_version;
        }

        template <CustomType T>
        TA_Serializer & operator << (const T &t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            callProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size> {});
            return *this;
        }

        template <CustomType T>
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            callProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size> {});
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serializer & operator << (T t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            if(m_fileStream.is_open())
            {
                if(TA_EndianConversion::isSystemLittleEndian())
                    t = TA_EndianConversion::swapEndian(t);
                m_fileStream.write(reinterpret_cast<const char *>(&t), sizeof(t));
            }
            return *this;
        }

        template <EndianConvertedType T>
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            if(m_fileStream.is_open())
            {
                m_fileStream.read(reinterpret_cast<char *>(&t), sizeof(t));
                if (m_fileStream.eof())
                {
                    CoreAsync::TA_CommonTools::debugInfo(META_STRING("End of file reached unexpectedly.\n"));
                }
                else if (m_fileStream.fail())
                {
                    CoreAsync::TA_CommonTools::debugInfo(META_STRING("Logical error on i/o operation.\n"));
                }
                else if (m_fileStream.bad())
                {
                    CoreAsync::TA_CommonTools::debugInfo(META_STRING("Read operation failed due to severe stream error.\n"));
                }
                if(TA_EndianConversion::isSystemLittleEndian())
                    t = TA_EndianConversion::swapEndian(t);
            }
            return *this;
        }

        template <StdContainerType T>
        TA_Serializer & operator << (const T &t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            *this << std::ranges::distance(t);
            std::ranges::for_each(std::as_const(t), [this](const T::value_type &val) {
                *this << val;
            });
            return *this;
        }

        template <StdAdaptorType T>
        TA_Serializer & operator << (const T &t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
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
        TA_Serializer & operator >> (std::array<T, N> &array)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            std::size_t size;
            *this >> size;
            for(auto &v : array)
            {
                *this >> v;
            }
            return *this;
        }

        template <StdContainerType T>
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            std::size_t size {};
            *this >> size;
            if constexpr(std::is_same_v<std::vector<typename T::value_type>, T> ||
                         std::is_same_v<std::deque<typename T::value_type>, T>)
            {
                t.resize(size);
                for(auto i = 0;i < size;++i)
                {
                    *this >> t[i];
                }
            }
            else if constexpr(std::is_same_v<std::list<typename T::value_type>, T>)
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
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
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
                t = {temp.begin(), temp.end()};
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
        TA_Serializer & operator << (const std::pair<K, V> &pair)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            return *this << pair.first << pair.second;
        }

        template <typename K, typename V>
        TA_Serializer & operator >> (std::pair<K, V> &pair)
        {
            static_assert(OType == OperationType::Deserialization , "The operation type isn't Deserialization ");
            return *this >> pair.first >> pair.second;
        }

        template <RawPtr T>
        TA_Serializer & operator << (const T &t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            return *this << *t;
        }

        template <RawPtr T>
        TA_Serializer & operator << (T &&t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            if(t)
            {
                return *this << *t;
            }
            return *this;;
        }

        template <RawPtr T>
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            if(!t)
                return *this;
            return *this >> *t;
        }

        template<typename T, int N>
        TA_Serializer & operator << (const T (&a)[N])
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            for(int i = 0;i < N;++i)
            {
                *this << a[i];
            }
            return *this;
        }

        template<typename T, int N>
        TA_Serializer & operator >> (T (&a)[N])
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            for(int i = 0;i < N;++i)
            {
                *this >> a[i];
            }
            return *this;
        }

        template <EnumType T>
        TA_Serializer & operator << (T t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            *this << static_cast<uint8_t>(t);
            return *this;
        }

        template <EnumType T>
        TA_Serializer & operator >> (T &t)
        {
            static_assert(OType == OperationType::Deserialization, "The operation type isn't Deserialization");
            uint8_t val {};
            *this >> val;
            t = static_cast<T>(val);
            return *this;
        }

        TA_Serializer & operator << (std::nullptr_t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Serialization ");
            return *this;
        }

        TA_Serializer & operator >> (std::nullptr_t)
        {
            static_assert(OType == OperationType::Serialization , "The operation type isn't Deserialization");
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
            static_assert(CoreAsync::Reflex::HasValidString<std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>>::value, "Invalid name retrieved during serialization.");
            if constexpr(OType == OperationType::Serialization )
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
        std::fstream m_fileStream;
        std::size_t m_version;

    };
}

#endif // TA_SERIALIZATION_H
