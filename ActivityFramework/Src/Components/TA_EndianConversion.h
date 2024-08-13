#ifndef TA_ENDIANCONVERSION_H
#define TA_ENDIANCONVERSION_H

#include "TA_TypeList.h"

namespace CoreAsync
{
    using EndianConversionTypes = TA_MetaTypelist<
        uint8_t, uint16_t, uint32_t, uint64_t,
        int8_t, int16_t, int32_t, int64_t,
        float, double, /*long double,*/
        char, signed char, unsigned char, wchar_t,
        char16_t, char32_t, bool
        >;

    template <typename T>
    concept EndianVerifyExp = MetaContains<EndianConversionTypes, std::decay_t<T>>::value;

    template <typename CType>
    concept EndianConvertedType = requires(CType ct)
    {
        {ct} -> EndianVerifyExp;
    };

    struct TA_EndianConversion
    {
        static bool isSystemLittleEndian()
        {
            return std::endian::native == std::endian::little;
        }

        template <EndianConvertedType T>
        static constexpr auto swapEndian(T value) -> std::decay_t<T>
        {
            using Rt = std::decay_t<T>;
            if constexpr (std::is_integral<Rt>::value && sizeof(Rt) == 1)
            {
                return value;
            }
            else if constexpr (std::is_integral<Rt>::value && sizeof(Rt) == 2)
            {
                return (value >> 8) | (value << 8);
            }
            else if constexpr (std::is_integral<Rt>::value && sizeof(Rt) == 4)
            {
                return ((value >> 24) & 0x000000FF) |
                       ((value << 8)  & 0x00FF0000) |
                       ((value >> 8)  & 0x0000FF00) |
                       ((value << 24) & 0xFF000000);
            }
            else if constexpr (std::is_integral<Rt>::value && sizeof(Rt) == 8)
            {
                return ((value >> 56) & 0x00000000000000FF) |
                       ((value << 40) & 0x00FF000000000000) |
                       ((value >> 40) & 0x000000000000FF00) |
                       ((value << 24) & 0x0000FF0000000000) |
                       ((value >> 24) & 0x0000000000FF0000) |
                       ((value << 8)  & 0x000000FF00000000) |
                       ((value >> 8)  & 0x00000000FF000000) |
                       ((value << 56) & 0xFF00000000000000);
            }
            else if constexpr (std::is_floating_point<Rt>::value && sizeof(Rt) == 4)
            {
                auto temp = std::bit_cast<uint32_t>(value);
                temp = swapEndian(temp);
                return std::bit_cast<float>(temp);
            }
            else if constexpr (std::is_floating_point<Rt>::value && sizeof(Rt) == 8)
            {
                auto temp = std::bit_cast<uint64_t>(value);
                temp = swapEndian(temp);
                return std::bit_cast<double>(temp);
            }
            else if constexpr (std::is_same_v<long double, Rt>)
            {

            }
            return value;
        }

        template<typename T>
        static constexpr void swapEndian(T *value) {
            if constexpr(std::is_integral<T>::value || std::is_floating_point<T>::value)
            {
                unsigned char *bytes = reinterpret_cast<unsigned char *>(value);
                size_t length = sizeof(T);

                for (size_t i = 0; i < length / 2; ++i)
                {
                    std::swap(bytes[i], bytes[length - 1 - i]);
                }
            }
        }
    };
}

#endif // TA_ENDIANCONVERSION_H
