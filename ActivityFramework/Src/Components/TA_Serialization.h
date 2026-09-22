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

#ifndef TA_SERIALIZATION_H
#define TA_SERIALIZATION_H

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
#include <ios>
#include <array>
#include <list>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <climits>
#include <limits>
#include <string>

#include "TA_CommonTools.h"
#include "TA_EndianConversion.h"
#include "TA_TypeFilter.h"
#include "TA_MetaReflex.h"
#include "TA_Buffer.h"

namespace CoreAsync {
template <typename T>
concept IsSerializable = EndianVerifyExp<T>;

template <typename T>
concept SerializableType = requires(T t) {
    { t } -> IsSerializable;
};

namespace SerializationDetail {
// Keep this list aligned with the decoding branches. The general container
// concepts also accept strings, views and user-defined containers.
template <typename T> struct SupportedContainer : std::false_type {};
template <> struct SupportedContainer<std::string> : std::true_type {};
template <typename T> struct SupportedContainer<std::vector<T>> : std::bool_constant<!std::is_same_v<T, bool>> {};
template <typename T> struct SupportedContainer<std::deque<T>> : std::true_type {};
template <typename T> struct SupportedContainer<std::list<T>> : std::true_type {};
template <typename T> struct SupportedContainer<std::forward_list<T>> : std::true_type {};
template <typename T, std::size_t N> struct SupportedContainer<std::array<T, N>> : std::true_type {};
template <typename K, typename V, typename C, typename A>
struct SupportedContainer<std::map<K, V, C, A>> : std::true_type {};
template <typename K, typename V, typename C, typename A>
struct SupportedContainer<std::multimap<K, V, C, A>> : std::true_type {};
template <typename K, typename C, typename A>
struct SupportedContainer<std::set<K, C, A>> : std::true_type {};
template <typename K, typename C, typename A>
struct SupportedContainer<std::multiset<K, C, A>> : std::true_type {};
template <typename K, typename V, typename H, typename E, typename A>
struct SupportedContainer<std::unordered_map<K, V, H, E, A>> : std::true_type {};
template <typename K, typename V, typename H, typename E, typename A>
struct SupportedContainer<std::unordered_multimap<K, V, H, E, A>> : std::true_type {};
template <typename K, typename H, typename E, typename A>
struct SupportedContainer<std::unordered_set<K, H, E, A>> : std::true_type {};
template <typename K, typename H, typename E, typename A>
struct SupportedContainer<std::unordered_multiset<K, H, E, A>> : std::true_type {};

template <typename T> struct SupportedAdaptor : std::false_type {};
template <typename T, typename C> struct SupportedAdaptor<std::stack<T, C>> : std::true_type {};
template <typename T, typename C> struct SupportedAdaptor<std::queue<T, C>> : std::true_type {};
template <typename T, typename C, typename Compare>
struct SupportedAdaptor<std::priority_queue<T, C, Compare>> : std::true_type {};

template <typename T>
concept ReflectedType = CustomType<T> && requires { typename Reflex::TA_TypeInfo<T>::TA_PropertyInfos; };
} // namespace SerializationDetail

template <BufferOperatorType OType = BufferWriter> class TA_Serializer {
  public:
    static constexpr std::uint32_t formatMagic = 0x41465753; // AFWS
    static constexpr std::uint16_t formatRevision = 1;
    static constexpr std::size_t headerSize = 16;
    static_assert(CHAR_BIT == 8);
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
    static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559);

    // Writer: schema version to emit. Reader: maximum supported schema version.
    // The wire layout and compatibility rules are in docs/serialization-format.md.
    explicit TA_Serializer(const std::string &path, std::uint64_t version = 1, std::size_t bufferSize = 1024 * 1024 * 2)
        : m_pDataOperator(std::make_unique<typename OType::OperatorType>(path, bufferSize)), m_version(version) {
        const bool initialized = init();
        if (!initialized) {
            if constexpr (std::is_same_v<BufferReader, OType>)
                throw std::ios_base::failure("Cannot read the serialization version header");
            else
                throw std::ios_base::failure("Cannot write the serialization version header");
        }
    }

    ~TA_Serializer() = default;

    TA_Serializer(const TA_Serializer &serialzation) = delete;
    TA_Serializer(TA_Serializer &&serialzation) = delete;

    TA_Serializer &operator=(const TA_Serializer &serialzation) = delete;
    TA_Serializer &operator=(TA_Serializer &&serialzation) = delete;

    std::uint64_t version() const { return m_version; }

    // Call close() before reporting a successful save. Destruction cannot report
    // failures, and flush() alone does not check the final file close operation.
    void flush() requires std::is_same_v<BufferWriter, OType> {
        if (!m_pDataOperator->flush())
            throw std::ios_base::failure("Cannot flush the serialized data");
    }

    void close() requires std::is_same_v<BufferWriter, OType> {
        if (!m_pDataOperator->finish())
            throw std::ios_base::failure("Cannot close the serialized file");
    }

    template <SerializationDetail::ReflectedType T> TA_Serializer &operator<<(const T &t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        extractProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size>{});
        return *this;
    }

    template <SerializationDetail::ReflectedType T> TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        extractProperty(t, std::make_index_sequence<Reflex::TA_TypeInfo<T>::TA_PropertyInfos::size>{});
        return *this;
    }

    template <SerializableType T> TA_Serializer &operator<<(T t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        if constexpr (std::is_same_v<T, bool>) {
            return *this << static_cast<std::uint8_t>(t);
        } else {
            if (TA_EndianConversion::isSystemLittleEndian())
                TA_EndianConversion::swapEndian(&t);
            if (!m_pDataOperator->write(t))
                throw std::ios_base::failure("Cannot write the serialized value");
            return *this;
        }
    }

    template <SerializableType T> TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        if constexpr (std::is_same_v<T, bool>) {
            std::uint8_t value{};
            *this >> value;
            if (value > 1)
                throw std::ios_base::failure("Invalid serialized boolean");
            t = value != 0;
            return *this;
        } else {
            // Stop nested and chained extraction before using an unread value.
            if (!m_pDataOperator->read(t))
                throw std::ios_base::failure("Cannot read the serialized value");
            if (TA_EndianConversion::isSystemLittleEndian())
                TA_EndianConversion::swapEndian(&t);
            return *this;
        }
    }

    template <StdContainerType T> requires SerializationDetail::SupportedContainer<T>::value
    TA_Serializer &operator<<(const T &t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        writeCount(std::ranges::distance(t));
        std::ranges::for_each(std::as_const(t), [this](const T::value_type &val) { *this << val; });
        return *this;
    }

    template <StdAdaptorType T> requires SerializationDetail::SupportedAdaptor<T>::value
    TA_Serializer &operator<<(const T &t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        writeCount(std::ranges::size(t));
        T copyAdaptor = t;
        if constexpr (std::is_same_v<std::stack<typename T::value_type, typename T::container_type>, T>) {
            while (!copyAdaptor.empty()) {
                *this << copyAdaptor.top();
                copyAdaptor.pop();
            }
        } else if constexpr (std::is_same_v<std::queue<typename T::value_type, typename T::container_type>, T>) {
            while (!copyAdaptor.empty()) {
                *this << copyAdaptor.front();
                copyAdaptor.pop();
            }
        } else if constexpr (std::is_same_v<std::priority_queue<typename T::value_type, typename T::container_type,
                                                                typename T::value_compare>,
                                            T>) {
            while (!copyAdaptor.empty()) {
                *this << copyAdaptor.top();
                copyAdaptor.pop();
            }
        }
        return *this;
    }

    template <typename T, std::size_t N> TA_Serializer &operator>>(std::array<T, N> &array) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        const auto size = readCount(N);
        if (size != N)
            throw std::ios_base::failure("Serialized array extent does not match destination");
        std::ranges::for_each(array, [this](T &val) { *this >> val; });
        return *this;
    }

    template <StdContainerType T> requires SerializationDetail::SupportedContainer<T>::value
    TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        const auto size = readCount(t.max_size());
        if constexpr (std::is_same_v<std::vector<typename T::value_type>, T> ||
                      std::is_same_v<std::deque<typename T::value_type>, T> ||
                      std::is_same_v<std::string, T>) {
            t.resize(size);
            for (auto &v : t) {
                *this >> v;
            }
        } else if constexpr (std::is_same_v<std::list<typename T::value_type>, T>) {
            for (std::size_t i = 0; i < size; ++i) {
                typename T::value_type val;
                *this >> val;
                t.emplace_back(std::move(val));
            }
        } else if constexpr (std::is_same_v<std::forward_list<typename T::value_type>, T>) {
            typename std::forward_list<typename T::value_type>::iterator beginIter = t.before_begin();
            for (std::size_t i = 0; i < size; ++i) {
                typename T::value_type val;
                *this >> val;
                beginIter = t.emplace_after(beginIter, std::move(val));
            }
        } else if constexpr (requires { typename T::key_type; typename T::mapped_type; }) {
            for (std::size_t i = 0; i < size; ++i) {
                typename T::key_type key{};
                typename T::mapped_type val{};
                *this >> key >> val;
                t.emplace_hint(t.end(), std::move(key), std::move(val));
            }
        } else if constexpr (requires { typename T::key_type; }) {
            for (std::size_t i = 0; i < size; ++i) {
                typename T::key_type val;
                *this >> val;
                t.emplace_hint(t.end(), std::move(val));
            }
        }
        return *this;
    }

    template <StdAdaptorType T> requires SerializationDetail::SupportedAdaptor<T>::value
    TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        const auto size = readCount(typename T::container_type{}.max_size());
        if constexpr (std::is_same_v<std::queue<typename T::value_type, typename T::container_type>, T>) {
            for (std::size_t i = 0; i < size; ++i) {
                typename T::value_type val;
                *this >> val;
                t.emplace(std::move(val));
            }
        } else if constexpr (std::is_same_v<std::stack<typename T::value_type, typename T::container_type>, T>) {
            std::deque<typename T::value_type> temp;
            for (std::size_t i = 0; i < size; ++i) {
                typename T::value_type val;
                *this >> val;
                temp.emplace_front(std::move(val));
            }
            t = {temp.begin(), temp.end()};
        } else if constexpr (std::is_same_v<std::priority_queue<typename T::value_type, typename T::container_type,
                                                                typename T::value_compare>,
                                            T>) {
            for (std::size_t i = 0; i < size; ++i) {
                typename T::value_type val;
                *this >> val;
                t.emplace(std::move(val));
            }
        }
        return *this;
    }

    template <typename K, typename V> TA_Serializer &operator<<(const std::pair<K, V> &pair) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        return *this << pair.first << pair.second;
    }

    template <typename K, typename V> TA_Serializer &operator>>(std::pair<K, V> &pair) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization ");
        return *this >> pair.first >> pair.second;
    }

    template <RawPtr T> TA_Serializer &operator<<(const T &t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        return *this << *t;
    }

    template <RawPtr T> TA_Serializer &operator<<(T &&t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        if (t) {
            return *this << *t;
        }
        return *this;
        ;
    }

    template <RawPtr T> TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        if (!t)
            return *this;
        return *this >> *t;
    }

    template <typename T, int N> TA_Serializer &operator<<(const T (&a)[N]) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        for (int i = 0; i < N; ++i) {
            *this << a[i];
        }
        return *this;
    }

    template <typename T, int N> TA_Serializer &operator>>(T (&a)[N]) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        for(auto &ele : a) {
            *this >> ele;
        }
        return *this;
    }

    template <EnumType T> TA_Serializer &operator<<(T t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        *this << static_cast<std::underlying_type_t<T>>(t);
        return *this;
    }

    template <EnumType T> TA_Serializer &operator>>(T &t) {
        static_assert(std::is_same_v<BufferReader, OType>, "The operation type isn't Deserialization");
        std::underlying_type_t<T> val{};
        *this >> val;
        t = static_cast<T>(val);
        return *this;
    }

    TA_Serializer &operator<<(std::nullptr_t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Serialization ");
        return *this;
    }

    TA_Serializer &operator>>(std::nullptr_t) {
        static_assert(std::is_same_v<BufferWriter, OType>, "The operation type isn't Deserialization");
        return *this;
    }

  private:
    template <typename Size> void writeCount(Size size) {
        if (!std::in_range<std::uint64_t>(size))
            throw std::ios_base::failure("Container count exceeds the wire format limit");
        *this << static_cast<std::uint64_t>(size);
    }

    std::size_t readCount(std::size_t maximum) {
        std::uint64_t size{};
        *this >> size;
        if (!std::in_range<std::size_t>(size) || size > maximum)
            throw std::ios_base::failure("Serialized count exceeds the destination limit");
        return static_cast<std::size_t>(size);
    }

    template <typename T> constexpr void extractProperty(const T &t, std::index_sequence<> = {}) { return; }

    template <typename T, std::size_t IDX0, std::size_t... IDXS>
    constexpr void extractProperty(
        T &t, std::index_sequence<IDX0, IDXS...> =
                  std::make_index_sequence<Reflex::TA_TypeInfo<std::remove_cvref_t<T>>::TA_PropertyInfos::size>{}) {
        using Rt = std::remove_cvref_t<T>;
        using Properties = Reflex::TA_TypeInfo<Rt>::TA_PropertyInfos::List;
        static_assert(CoreAsync::Reflex::HasValidString<
                          std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>>::value,
                      "Invalid name retrieved during serialization.");
        if constexpr (std::is_same_v<BufferWriter, OType>) {
            if (m_version >= std::tuple_element_t<1, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>::m_value) {
                *this << Reflex::TA_TypeInfo<Rt>::invoke(
                    std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>{}, t);
            }
        } else {
            if (std::tuple_element_t<1, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>::m_value <= m_version) {
                using ValType = std::remove_pointer_t<
                    typename VariableTypeInfo<std::remove_cvref_t<decltype(CoreAsync::Reflex::TA_TypeInfo<Rt>::findType(
                        std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>{}))>>::RetType>;
                ValType val{};
                // std::cout << typeid(ValType).name() << std::endl;
                *this >> val;
                Reflex::TA_TypeInfo<Rt>::update(
                    t, std::move(val),
                    std::tuple_element_t<0, typename CoreAsync::MetaTypeAt<Properties, IDX0>::type>{});
            }
        }
        extractProperty(t, std::index_sequence<IDXS...>{});
    }

    bool init() {
        if (!m_pDataOperator->isValid()) {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("Cannot open the file.\n"));
            return false;
        }
        if constexpr (std::is_same_v<BufferReader, OType>) {
            std::uint32_t magic{};
            std::uint16_t revision{}, flags{};
            std::uint64_t schema{};
            *this >> magic >> revision >> flags >> schema;
            if (magic != formatMagic)
                throw std::ios_base::failure("Invalid serialization magic; legacy files require migration");
            if (revision != formatRevision || flags != 0)
                throw std::ios_base::failure("Unsupported serialization format revision or flags");
            if (schema == 0 || schema > m_version)
                throw std::ios_base::failure("Unsupported serialization schema version");
            m_version = schema;
        } else {
            if (m_version == 0)
                throw std::ios_base::failure("Serialization schema versions start at 1");
            *this << formatMagic << formatRevision << std::uint16_t{0} << m_version;
        }
        return true;
    }

  private:
    std::unique_ptr<OType> m_pDataOperator;
    std::uint64_t m_version;
};
} // namespace CoreAsync

#endif // TA_SERIALIZATION_H
