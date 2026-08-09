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

#ifndef TA_ENDIANCONVERSION_H
#define TA_ENDIANCONVERSION_H

#include "TA_TypeList.h"

#include <algorithm>
#include <bit>
#include <concepts>
#include <span>

namespace CoreAsync {
using EndianConversionTypes = TA_MetaTypelist<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t,
                                              float, double, /*long double,*/
                                              char, signed char, unsigned char, wchar_t, char16_t, char32_t, bool>;

template <typename T>
concept EndianVerifyExp = MetaContains<EndianConversionTypes, std::decay_t<T>>::value;

template <typename CType>
concept EndianConvertedType = requires(CType ct) {
    { ct } -> EndianVerifyExp;
};

struct TA_EndianConversion {
    static bool isSystemLittleEndian() { return std::endian::native == std::endian::little; }

    template <EndianConvertedType T> static constexpr auto swapEndian(T value) -> std::decay_t<T> {
        using Rt = std::remove_cvref_t<T>;
        if constexpr (std::is_integral<Rt>::value) {
            swapEndian(&value);
        } else if constexpr (std::is_floating_point<Rt>::value) {
            swapEndian(&value);
        } else if constexpr (std::is_same_v<long double, Rt>) {
            
        }
        return value;
    }

    template <typename T>
    requires std::floating_point<T>
    static constexpr void swapEndian(T *value) {
        using Rt = std::remove_cvref_t<T>;
        if (!value) {
            return;
        }
        if constexpr (sizeof(Rt) == sizeof(float)) {
            auto temp = std::bit_cast<uint32_t>(*value);
            swapEndian(&temp);
            *value = std::bit_cast<Rt>(temp);
        } else if constexpr (sizeof(Rt) == sizeof(double)) {
            auto temp = std::bit_cast<uint64_t>(*value);
            swapEndian(&temp);
            *value = std::bit_cast<Rt>(temp);
        } else {
            std::span<unsigned char> bytes {reinterpret_cast<unsigned char *>(value), sizeof(Rt)};
            std::ranges::reverse(bytes);
        }
    }

    template <typename T>
    requires std::integral<T>
    static constexpr void swapEndian(T *value) {
        if (value) {
            *value = std::byteswap(*value);
        }
    }
};
} // namespace CoreAsync

#endif // TA_ENDIANCONVERSION_H
