#ifndef TA_ENDIANCONVERSION_H
#define TA_ENDIANCONVERSION_H

#include "TA_TypeList.h"

namespace CoreAsync
{
    using EndianConversionTypes = TA_MetaTypelist<
        uint16_t, uint32_t, uint64_t, float, double, wchar_t , char16_t, char32_t, char8_t, char
        >;

    template <typename T>
    concept EndianVerifyExp = MetaContains<EndianConversionTypes, T>::value;

    template <typename CType>
    concept EndianConvertedType = requires(CType ct)
    {
        {ct} -> EndianVerifyExp;
    };

    struct EndianConversion
    {
        template <EndianConvertedType T>
        static constexpr T swapEndian(T value)
        {
            if constexpr(std::is_same_v<uint16_t, T>)
            {
                return (value >> 8) | (value << 8);
            }
            else if constexpr(std::is_same_v<uint32_t, T>)
            {
                return ((value >> 24) & 0x000000FF) |
                       ((value << 8) & 0x00FF0000) |
                       ((value >> 8) & 0x0000FF00) |
                       ((value << 24) & 0xFF000000);
            }
            else if constexpr(std::is_same_v<uint64_t, T>)
            {
                return ((value >> 56) & 0x00000000000000FF) |
                       ((value << 40) & 0x00FF000000000000) |
                       ((value >> 40) & 0x000000000000FF00) |
                       ((value << 24) & 0x0000FF0000000000) |
                       ((value >> 24) & 0x0000000000FF0000) |
                       ((value << 8) & 0x000000FF00000000) |
                       ((value >> 8) & 0x00000000FF000000) |
                       ((value << 56) & 0xFF00000000000000);
            }
            else if constexpr(std::is_same_v<float, T>)
            {
                uint32_t temp = *reinterpret_cast<uint32_t *>(&value);
                temp = swapEndian(temp);
                return *reinterpret_cast<float *>(&temp);
            }
            else if constexpr(std::is_same_v<double, T>)
            {
                uint64_t temp = *reinterpret_cast<uint64_t *>(&value);
                temp = swapEndian(temp);
                return *reinterpret_cast<double *>(&temp);
            }
            else if constexpr(std::is_same_v<wchar_t, T>)
            {
                return swapEndian(static_cast<uint16_t>(value));
            }
            else if constexpr(std::is_same_v<char16_t, T>)
            {
                return static_cast<char16_t>(swapEndian(static_cast<uint16_t>(value)));
            }
            else if constexpr(std::is_same_v<char32_t, T>)
            {
                return static_cast<char32_t>(swapEndian(static_cast<uint32_t>(value)));
            }
            return value;
        }
    };
}

#endif // TA_ENDIANCONVERSION_H
