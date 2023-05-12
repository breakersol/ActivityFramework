#ifndef TA_METASTRINGVIEW_H
#define TA_METASTRINGVIEW_H

#include <type_traits>
#include <string_view>


#define     META_STRING(s) ([]{constexpr std::basic_string_view str{s}; return CoreAsync::TA_StringView<CoreAsync::TA_RawString<typename decltype(str)::value_type, str.size()> {str} > {};} ())

namespace CoreAsync {

template <typename C, std::size_t N>
struct TA_RawString
{
    using StrType = C;

    StrType data[N + 1] {};
    static constexpr std::size_t size {N};

    constexpr TA_RawString(std::basic_string_view<StrType> str)
    {
        for(std::size_t i {0};i < size;++i)
        {
            data[i] = str[i];
        }
    }
};

template <TA_RawString str>
struct TA_StringView
{
    using RawType = typename decltype(str)::StrType;

    template <typename T>
    static constexpr bool isType(T = {})
    {
        return std::is_same_v<RawType, T>;
    }

    static constexpr auto raw()
    {
        return str.data;
    }

    static constexpr std::size_t size()
    {
        return str.size;
    }

    static constexpr std::basic_string_view<RawType> data()
    {
        return str.data;
    }
};

}

#endif // TA_METASTRING_H
