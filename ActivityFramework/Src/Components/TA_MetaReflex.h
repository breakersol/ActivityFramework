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

#ifndef TA_METAREFLEX_H
#define TA_METAREFLEX_H

#include <optional>
#include <string_view>
#include <utility>

#include "Components/TA_MetaStringView.h"
#include "TA_TypeFilter.h"
#include "TA_TypeList.h"

namespace CoreAsync::Reflex {

template <typename T, typename = std::void_t<>> struct HasValidString : std::false_type {};
template <auto str> struct HasValidString<CoreAsync::TA_StringView<str>, std::void_t<CoreAsync::TA_StringView<str>>> : std::true_type {};

template <typename TypeStr> struct TA_BaseTypeName {
    using TName = TypeStr;
    static constexpr std::string_view name = TName::data();
};

template <typename T, typename N> class TA_MetaTypeName : public TA_BaseTypeName<N> {
  public:
    constexpr TA_MetaTypeName(T value) : TA_BaseTypeName<N>(), m_value(value) {}

    static constexpr bool isValid() { return m_isValid; }

    constexpr T value() const { return m_value; }

    template <typename OT> constexpr bool operator==(OT value) const {
        if constexpr (std::is_same_v<T, OT>) {
            return m_value == value;
        } else {
            return false;
        }
    }

  private:
    const T m_value;
    static constexpr bool m_isValid = true;
};

template <typename N> class TA_MetaTypeName<void, N> : public TA_BaseTypeName<N> {
  public:
    static constexpr bool isValid() { return m_isValid; }

    template <typename OT> constexpr bool operator==(OT) const { return false; }

  private:
    static constexpr bool m_isValid = false;
};

template <typename T> struct TA_MemberTypeTrait {
    static constexpr bool enumFlag = std::is_enum<T>::value;
    static constexpr bool funcFlag = std::is_function_v<T>;
    static constexpr bool instanceVariableFlag = IsInstanceVariable<T>::value;
    static constexpr bool staticVariableFlag = IsStaticVariable<T>::value;
    static constexpr bool instanceMethodFlag = IsInstanceMethod<T>::value;
    static constexpr bool staticMethodFlag = IsStaticMethod<T>::value;
};

// template <typename READ, typename WRITE>
// struct TA_MetaPropertyOperation
// {
//     [[deprecated]] constexpr TA_MetaPropertyOperation(READ = {}, WRITE = {})
//     {

//     }

//     [[deprecated]] static constexpr auto readOperation {READ::data()};
//     [[deprecated]] static constexpr auto writeOperation {WRITE::data()};
// };

// template <typename ...OPERATIONS>
// struct TA_MetaPropertyOperations
// {
//     [[deprecated]] constexpr TA_MetaPropertyOperations(OPERATIONS ...ops) : m_operations(ops...)
//     {

//     }

//     [[deprecated]] constexpr std::size_t size() const
//     {
//         return sizeof...(OPERATIONS);
//     }

//     template <std::size_t IDX>
//     [[deprecated]] constexpr auto getOperation() const
//     {
//         return std::get<IDX>(m_operations);
//     }

// private:
//     [[deprecated]] std::tuple<OPERATIONS...> m_operations;

// };

template <std::size_t value> struct TA_MetaRoleVersion {
    static constexpr std::size_t m_value{value};
};

template <typename Role, typename Version> struct TA_MetaRole {
    static constexpr std::string_view m_role{Role::data()};
    static constexpr std::size_t m_version{Version::m_value};
    static constexpr bool m_isProperty{Role::data() == std::string_view("Property")};
};

template <typename Role, typename Version> using TA_MetaPropertyParameters = std::tuple<Role, Version>;

template <typename T, typename NAME, typename Role = decltype(META_STRING("")),
          typename Version = TA_MetaRoleVersion<1>>
struct TA_MetaField : TA_MemberTypeTrait<T>, TA_MetaTypeName<T, NAME>, TA_MetaRole<Role, Version> {
    constexpr TA_MetaField(T t, NAME, TA_MetaPropertyParameters<Role, Version> = {}) : TA_MetaTypeName<T, NAME>{t} {
        if constexpr (TA_MetaRole<Role, Version>::m_isProperty) {
            static_assert(IsInstanceVariable<T>::value, "Non instance variable can't be declared as property.");
        }
    }

    constexpr TA_MetaField() : TA_MetaTypeName<T, NAME>{nullptr} {}
};

template <typename... FIELDS> class TA_MetaFieldList {
  public:
    using ValueTypes = TA_MetaTypelist<decltype(FIELDS{}.value())...>;

    constexpr TA_MetaFieldList(FIELDS... fs) : m_fields(fs...) {}

    constexpr auto get() const { return m_fields; }

    constexpr std::size_t size() const { return sizeof...(FIELDS); }

    template <std::size_t INDEX> constexpr auto getField() const { return std::get<INDEX>(m_fields); }

    template <typename NAME> constexpr auto findValue(NAME = {}) const {
        constexpr std::size_t index = findFieldIndex<NAME>();
        if constexpr (index < sizeof...(FIELDS)) {
            return std::get<index>(m_fields).value();
        } else if constexpr (sizeof...(FIELDS) == 0) {
            static_assert(sizeof...(FIELDS) != 0, "The field list is empty. Cannot find the type by name.");
            return nullptr;
        } else {
            static_assert(index < sizeof...(FIELDS), "Field not found.");
            return nullptr;
        }
    }

    template <typename VALUE> constexpr auto findName(VALUE &&v) const {
        return findMatchedName(std::forward<VALUE>(v), std::make_index_sequence<sizeof...(FIELDS)>());
    }

    template <typename VALUE> constexpr bool containsType(VALUE &&v) const {
        return containsMatchedType(std::forward<VALUE>(v), std::make_index_sequence<sizeof...(FIELDS)>{});
    }

    template <typename NAME> constexpr bool containsName(NAME = {}) const {
        return findFieldIndex<NAME>() < sizeof...(FIELDS);
    }

    template <FieldType P> struct PropertyFilter {
        static constexpr bool value = P::m_isProperty;
    };

    template <typename M> struct PropertyMapper {
        using type = std::tuple<typename M::TName, TA_MetaRoleVersion<M::m_version>>;
    };

    struct PropertyInfos {
        using Types = MetaFilterMapper<TA_MetaTypelist<FIELDS...>, PropertyFilter, PropertyMapper>::result;
    };

  private:
    template <typename NAME, std::size_t INDEX = 0> static consteval std::size_t findFieldIndex() {
        if constexpr (INDEX == sizeof...(FIELDS)) {
            return INDEX;
        } else {
            using Field = std::tuple_element_t<INDEX, std::tuple<FIELDS...>>;
            if constexpr (NAME::data() == Field::name) {
                return INDEX;
            } else {
                return findFieldIndex<NAME, INDEX + 1>();
            }
        }
    }

    template <typename VALUE> constexpr bool containsMatchedType(VALUE &&, std::index_sequence<>) const {
        return false;
    }

    template <typename VALUE, std::size_t IDX0, std::size_t... IDXS>
    constexpr bool containsMatchedType(VALUE &&v, std::index_sequence<IDX0, IDXS...>) const {
        if constexpr (std::is_same_v<VALUE, decltype(std::get<IDX0>(m_fields).value())>) {
            return v == std::get<IDX0>(m_fields).value()
                       ? true
                       : containsMatchedType(std::forward<VALUE>(v), std::index_sequence<IDXS...>());
        } else {
            return containsMatchedType(std::forward<VALUE>(v), std::index_sequence<IDXS...>());
        }
    }

    template <typename VALUE> constexpr auto findMatchedName(VALUE &&, std::index_sequence<>) const { return ""; }

    template <typename VALUE, std::size_t IDX0, std::size_t... IDXS>
    constexpr auto findMatchedName(VALUE &&v, std::index_sequence<IDX0, IDXS...>) const {
        if constexpr (std::is_same_v<VALUE, decltype(std::get<IDX0>(m_fields).value())>) {
            return v == std::get<IDX0>(m_fields).value()
                       ? std::get<IDX0>(m_fields).name
                       : findMatchedName(std::forward<VALUE>(v), std::index_sequence<IDXS...>());
        } else {
            return findMatchedName(std::forward<VALUE>(v), std::index_sequence<IDXS...>());
        }
    }

  private:
    std::tuple<FIELDS...> m_fields;
};

template <typename T> struct TA_MetaTypeAttribute {
    static constexpr bool isPolymorphic = std::is_polymorphic_v<T>;
    static constexpr bool isAbstract = std::is_abstract_v<T>;
    static constexpr bool isClass = std::is_class_v<T>;
};

template <typename T> struct TA_TypeInfo;

template <typename T, typename... BASES> struct TA_MetaTypeInfo : TA_MetaTypeAttribute<T> {
    using Raw = T;

    static constexpr std::size_t size() {
        return std::tuple_size_v<std::remove_cvref_t<decltype(aggregateCache<>)>>;
    }

    template <typename NAME, typename... PARAS> static constexpr auto invoke(NAME = {}, PARAS &&...paras) {
        constexpr auto target = findType(NAME{});
        using CF = decltype(target);
        if constexpr (std::is_null_pointer_v<CF>) {
            return nullptr;
        } else if constexpr (std::is_enum_v<decltype(target)>) {
            return target;
        } else if constexpr (!std::is_function_v<std::remove_pointer_t<CF>> && !std::is_member_object_pointer_v<CF> &&
                             !std::is_member_function_pointer_v<CF>) {
            return *target;
        } else {
            return std::invoke(target, std::forward<PARAS>(paras)...);
        }
    }

    template <typename NAME, typename OBJ, typename Para>
    static constexpr auto update(OBJ &obj, Para &&para, NAME = {}) {
        constexpr auto target = findType(NAME{});
        using CF = decltype(findType(NAME{}));
        static_assert(IsInstanceVariable<CF>::value, "The target type is not supported to set.");
        using RT = VariableTypeInfo<CF>::RetType;
        if constexpr (std::is_pointer_v<std::remove_cv_t<OBJ>>) {
            if constexpr (std::is_pointer_v<RT>) {
                *(obj->*target) = para;
            } else
                (obj->*target) = para;
        } else {
            if constexpr (std::is_pointer_v<RT>) {
                *(obj.*target) = para;
            } else
                (obj.*target) = para;
        }
    }

    template <typename NAME> static constexpr auto offset(NAME = {}) {
        using TargetType = std::remove_cvref_t<decltype(findType(NAME{}))>;
        static_assert(std::is_standard_layout_v<TargetType>, "The type is not stand layout type.");
        return offsetof(T, TargetType);
    }

    // [[deprecated]] static constexpr std::size_t operationSize()
    // {
    //     return TA_TypeInfo<T>::operations.size();
    // }

    // template <std::size_t IDX>
    // [[deprecated]]static constexpr decltype(auto) findPropertyOperation()
    // {
    //     using OpType = decltype(TA_TypeInfo<T>::operations.template getOperation<IDX>());
    //     constexpr auto read {findType(META_STRING(OpType::readOperation))};
    //     constexpr auto write {findType(META_STRING(OpType::writeOperation))};
    //     static_assert(!std::is_function_v<decltype(read)>, "The read operation found is wrong.");
    //     static_assert(!std::is_function_v<decltype(write)>, "The write operation found is wrong.");
    //     return std::tuple {read, write};
    // }

    template <typename VALUE> static constexpr bool containsValue(VALUE &&v) {
        return containsField(std::forward<VALUE>(v), std::make_index_sequence<size()>{});
    }

    template <auto VALUE>
    static consteval std::string_view findName() {
        return findNamePrivate<VALUE>();
    }

    template <auto VALUE>
    static consteval std::size_t valueIndex() {
        constexpr auto index = valueIndexPrivate<VALUE>(aggregateCache<>, std::make_index_sequence<size()>{});
        static_assert(index < size(), "Value not found in the field list.");
        return index;
    }

    template <auto NAME>
    static consteval std::size_t nameIndex() {
        constexpr auto index = nameIndexPrivate<NAME>(aggregateCache<>, std::make_index_sequence<size()>{});
        static_assert(index < size(), "Name not found in the field list.");
        return index;
    }

    template <std::size_t INDEX> static consteval auto valueAt() {
        if constexpr (INDEX < size()) {
            return std::get<INDEX>(aggregateCache<>).value();
        } else {
            static_assert(INDEX < size(), "Index out of range.");
            return nullptr;
        }
    }

    template <std::size_t INDEX> static consteval auto nameAt() {
        if constexpr (INDEX < size()) {
            return std::get<INDEX>(aggregateCache<>).name;
        } else {
            static_assert(INDEX < size(), "Index out of range.");
            return std::string_view{};
        }
    }

    template <std::size_t INDEX> static consteval auto nameValueAt() {
        if constexpr (INDEX < size()) {
            return std::pair{std::get<INDEX>(aggregateCache<>).name, std::get<INDEX>(aggregateCache<>).value()};
        } else {
            static_assert(INDEX < size(), "Index out of range.");
            return std::pair{std::string_view{}, nullptr};
        }
    }

    template <typename VALUE> static constexpr std::string_view findName(VALUE &&v) {
        return findNamePrivate(std::forward<VALUE>(v), std::make_index_sequence<size()>{});
    }

    static constexpr auto findTypeValue(std::string_view str) { // run time finding
        return findTypeValuePrivate(str, std::make_index_sequence<size()>{});
    }

    template <typename NAME> static constexpr bool containsName(NAME = {}) {
        if constexpr (!TA_MetaTypeInfo<T>::isClass) {
            return TA_TypeInfo<T>::fields.containsName(NAME{});
        }
        if constexpr (!TA_MetaTypeInfo<T>::isPolymorphic && sizeof...(BASES) == 0) {
            return TA_TypeInfo<T>::fields.containsName(NAME{});
        } else if constexpr (TA_TypeInfo<T>::fields.containsName(NAME{})) {
            return true;
        } else {
            return containsNameFromBase(std::make_index_sequence<sizeof...(BASES)>{}, NAME{});
        }
    }

    template <typename NAME>
    static constexpr auto findType(NAME = {}) // compile time finding
    {
        if constexpr (!TA_MetaTypeInfo<T>::isClass) {
            if constexpr (TA_TypeInfo<T>::fields.containsName(NAME{})) {
                return TA_TypeInfo<T>::fields.findValue(NAME{});
            } else {
                return nullptr;
            }
        }
        if constexpr (!TA_MetaTypeInfo<T>::isPolymorphic && sizeof...(BASES) == 0) {
            if constexpr (TA_TypeInfo<T>::fields.containsName(NAME{})) {
                return TA_TypeInfo<T>::fields.findValue(NAME{});
            } else {
                return nullptr;
            }
        } else {
            if constexpr (TA_TypeInfo<T>::fields.containsName(NAME{})) {
                return TA_TypeInfo<T>::fields.findValue(NAME{});
            } else {
                return findTypeFromBase(std::make_index_sequence<sizeof...(BASES)>{}, NAME{});
            }
        }
    }

    struct TA_Values {
        using VariantTypes = typename MetaMerge<typename decltype(TA_TypeInfo<T>::fields)::ValueTypes,
                                                typename TA_TypeInfo<BASES>::TA_Values::VariantTypes...>::type;
    };

    struct TA_PropertyInfos {
        using List = MetaRemoveDuplicate<typename MetaAppend<
            typename std::remove_cv_t<decltype(TA_TypeInfo<T>::fields)>::PropertyInfos::Types,
            typename std::remove_cv_t<decltype(TA_TypeInfo<BASES>::fields)>::PropertyInfos::Types...>::type>::result;
        static constexpr auto size = MetaSize<List>::value;
    };

    template <typename BASE> struct TA_MetaInfoPack {
        using type = TA_TypeInfo<BASE>;
    };

    struct TA_BaseTypes {
        using types = TA_MetaTypelist<BASES...>;
        static constexpr std::size_t size{sizeof...(BASES)};

        static constexpr auto typeNames() {
            constexpr std::array<std::string_view, Reflex::TA_TypeInfo<std::decay_t<T>>::TA_BaseTypes::size> arr{
                typeid(BASES).name()...};
            return arr;
        }
    };

    struct TA_MetaBaseTypesInfo {
        using types = typename MetaMapper<TA_MetaTypelist<BASES...>, TA_MetaInfoPack>::type;
        static constexpr std::size_t size = types::size;
    };

    static constexpr auto aggregate() {
        return std::tuple_cat(TA_MetaInfoPack<T>::type::fields.get(), TA_TypeInfo<BASES>::aggregate()...);
    }

  private:
    template <typename = void> static constexpr auto aggregateCache = aggregate();

    template <typename NAME> static constexpr bool containsNameFromBase(std::index_sequence<>, NAME = {}) {
        return false;
    }

    template <std::size_t IDX0, std::size_t... IDXS, typename NAME>
    static constexpr bool containsNameFromBase(std::index_sequence<IDX0, IDXS...>, NAME = {}) {
        if constexpr (MetaTypeAt<typename TA_MetaBaseTypesInfo::types, IDX0>::type::containsName(NAME{})) {
            return true;
        } else {
            return containsNameFromBase(std::index_sequence<IDXS...>{}, NAME{});
        }
    }

    template <typename NAME> static constexpr auto findTypeFromBase(std::index_sequence<>, NAME = {}) {
        return nullptr;
    }

    template <std::size_t IDX0, std::size_t... IDXS, typename NAME>
    static constexpr auto findTypeFromBase(std::index_sequence<IDX0, IDXS...>, NAME = {}) {
        return [](NAME = {}) {
            if constexpr (MetaTypeAt<typename TA_MetaBaseTypesInfo::types, IDX0>::type::containsName(NAME{})) {
                return MetaTypeAt<typename TA_MetaBaseTypesInfo::types, IDX0>::type::findType(NAME{});
            } else {
                return findTypeFromBase(std::index_sequence<IDXS...>{}, NAME{});
            }
        }();
    }

    template <typename VALUE> static constexpr bool containsField(VALUE &&, std::index_sequence<>) { return false; }

    template <typename VALUE, std::size_t IDX0, std::size_t... IDXS>
    static constexpr bool containsField(VALUE &&v, std::index_sequence<IDX0, IDXS...> = {}) {
        if constexpr (
            std::is_same_v<decltype(std::get<IDX0>(aggregateCache<>).value()), std::decay_t<VALUE>>) {
            if (v == std::get<IDX0>(aggregateCache<>).value())
                return true;
        }
        return containsField(std::forward<VALUE>(v), std::index_sequence<IDXS...>{});
    }

    template <typename VALUE> static constexpr std::string_view findNamePrivate(VALUE &&, std::index_sequence<>) {
        return {};
    }

    template <typename VALUE, std::size_t IDX0, std::size_t... IDXS>
    static constexpr std::string_view findNamePrivate(VALUE &&v, std::index_sequence<IDX0, IDXS...> = {}) {
        if constexpr (std::is_same_v<decltype(std::get<IDX0>(aggregateCache<>).value()), std::decay_t<VALUE>>) {
            if (v == std::get<IDX0>(aggregateCache<>).value())
                return std::get<IDX0>(aggregateCache<>).name;
        }
        return findNamePrivate(std::forward<VALUE>(v), std::index_sequence<IDXS...>{});
    }

    template <auto VALUE> static consteval std::string_view findNamePrivate() {
        return findNamePrivate(VALUE, std::make_index_sequence<size()>{});
    }

    template <auto VALUE, typename VALUES>
    static consteval std::size_t valueIndexPrivate(const VALUES &, std::index_sequence<> = {}) {
        return std::tuple_size_v<VALUES>;
    }

    template <auto VALUE, typename VALUES, std::size_t IDX0, std::size_t... IDXS>
    static consteval std::size_t valueIndexPrivate(const VALUES &values,
                                                   std::index_sequence<IDX0, IDXS...> = {}) {
        if constexpr (
            std::is_same_v<decltype(std::get<IDX0>(values).value()), std::decay_t<decltype(VALUE)>>) {
            if (VALUE == std::get<IDX0>(values).value())
                return IDX0;
        }
        return valueIndexPrivate<VALUE>(values, std::index_sequence<IDXS...>{});
    }

    template <auto NAME, typename VALUES>
    static consteval std::size_t nameIndexPrivate(const VALUES &, std::index_sequence<> = {}) {
        return std::tuple_size_v<VALUES>;
    }

    template <auto NAME, typename VALUES, std::size_t IDX0, std::size_t... IDXS>
    static consteval std::size_t nameIndexPrivate(const VALUES &values,
                                                  std::index_sequence<IDX0, IDXS...> = {}) {
        if (decltype(NAME)::data() == std::get<IDX0>(values).name) {
            return IDX0;
        }
        return nameIndexPrivate<NAME>(values, std::index_sequence<IDXS...>{});
    }

    static constexpr auto findTypeValuePrivate(std::string_view, std::index_sequence<> = {}) {
        using RuntimeValue = typename MetaVariant<typename TA_Values::VariantTypes>::Var;
        return std::optional<RuntimeValue>{};
    }

    template <std::size_t IDX0, std::size_t... IDXS>
    static constexpr auto findTypeValuePrivate(std::string_view str, std::index_sequence<IDX0, IDXS...> = {}) {
        using RuntimeValue = typename MetaVariant<typename TA_Values::VariantTypes>::Var;
        if (str == std::get<IDX0>(aggregateCache<>).name) {
            return std::optional<RuntimeValue>{RuntimeValue{std::get<IDX0>(aggregateCache<>).value()}};
        }
        return findTypeValuePrivate(str, std::index_sequence<IDXS...>{});
    }
};

#define ENABLE_REFLEX template <typename T> friend struct CoreAsync::Reflex::TA_TypeInfo;

#define TA_DEFAULT_PROPERTY                                                                                            \
    CoreAsync::Reflex::TA_MetaPropertyParameters<decltype(META_STRING("Property")), TA_MetaRoleVersion<1>> {}

#define TA_PROPERTY(VALUE)                                                                                             \
    CoreAsync::Reflex::TA_MetaPropertyParameters<decltype(META_STRING("Property")), TA_MetaRoleVersion<VALUE>> {}

#define REGISTER_FIELD(FIELD, ...)                                                                                     \
    TA_MetaField { &Raw::FIELD, META_STRING(#FIELD), ##__VA_ARGS__ }

#define REGISTER_ENUM(FIELD, ...)                                                                                      \
    TA_MetaField { Raw::FIELD, META_STRING(#FIELD), ##__VA_ARGS__ }

#define REGISTER_METHOD_OVERLOAD(FIELD, RET, ...)                                                                      \
    TA_MetaField { static_cast<RET (Raw::*)(__VA_ARGS__)>(&Raw::FIELD), META_STRING(#FIELD "<" #__VA_ARGS__ ">") }

#define REGISTER_METHOD_OVERLOAD_CONST(FIELD, RET, ...)                                                                \
    TA_MetaField { static_cast<RET (Raw::*)(__VA_ARGS__) const>(&Raw::FIELD), META_STRING(#FIELD "<" #__VA_ARGS__ ">") }

#define REGISTER_METHOD_OVERLOAD_GENERIC(FIELD, CONST_QUAL, RET, ...)                                                  \
    TA_MetaField {                                                                                                     \
        static_cast<RET (MetaTest::*)(__VA_ARGS__) CONST_QUAL>(&MetaTest::FIELD),                                      \
            META_STRING(#FIELD "<" #__VA_ARGS__ ">")                                                                   \
    }

#define DEFINE_TYPE_INFO(CLASS_TYPE, ...)                                                                              \
    template <> struct CoreAsync::Reflex::TA_TypeInfo<CLASS_TYPE> : public TA_MetaTypeInfo<CLASS_TYPE, ##__VA_ARGS__>

#define AUTO_META_FIELDS(...) static constexpr TA_MetaFieldList fields = {__VA_ARGS__};

#define PRINT_FIELD(...) std::cout << "<" #__VA_ARGS__ ">" << std::endl;

} // namespace CoreAsync::Reflex

#endif // TA_METAREFLEX_H
