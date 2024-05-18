/*
 * Copyright [2024] [Shuang Zhu / Sol]
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

#ifndef TA_TYPEFILTER_H
#define TA_TYPEFILTER_H

#include "Components/TA_TypeList.h"

#include <memory>
#include <functional>
#include <concepts>

namespace CoreAsync {

template <typename Ins>
struct IsSmartPointer
{
    static constexpr bool value = false;
};

template <typename Ins>
struct IsSmartPointer<std::shared_ptr<Ins> >
{
    static constexpr bool value = true;
};

template <typename Ins>
struct IsSmartPointer<std::unique_ptr<Ins> >
{
    static constexpr bool value = true;
};

template <typename Ins>
struct IsSmartPointer<std::weak_ptr<Ins> >
{
    static constexpr bool value = true;
};

template <typename T>
struct IsNonStaticMemberVariable
{
   static constexpr bool value = false;
};

template <typename CL, typename T>
struct IsNonStaticMemberVariable<T CL::*>
{
     static constexpr bool value =!std::is_function_v<T> && std::is_member_pointer_v<T CL::*>;
};

template <typename T>
struct IsNonStaticMemberFunc
{
   static constexpr bool value = false;
};

template <typename CL, typename T>
struct IsNonStaticMemberFunc<T CL::*>
{
   static constexpr bool value = std::is_function_v<T> && std::is_member_pointer_v<T CL::*>;
};

template <typename ST, typename FUNC, template <typename S, typename D> class FILTER>
struct IsReturnTypeEqual;

template <typename ST, typename R, typename ...PARA, template <typename S, typename D> class FILTER>
struct IsReturnTypeEqual<ST,R(*)(PARA...),FILTER>
{
    using SelectType = ST;
    using Func = R(*)(PARA...);
    using RetType = R;

    static constexpr bool value = FILTER<SelectType, RetType>::value;
};

template <typename ST, typename C, typename R ,typename ...PARA, template <typename S, typename D> class FILTER>
struct IsReturnTypeEqual<ST,R(C::*)(PARA...),FILTER>
{
    using SelectType = ST;
    using Func = R(*)(PARA...);
    using RetType = R;

    static constexpr bool value = FILTER<SelectType, RetType>::value;
};

template <typename ST, typename C, typename R ,typename ...PARA, template <typename S, typename D> class FILTER>
struct IsReturnTypeEqual<ST,R(C::*)(PARA...) const, FILTER>
{
    using SelectType = ST;
    using Func = R(*)(PARA...);
    using RetType = R;

    static constexpr bool value = FILTER<SelectType, RetType>::value;
};

template <typename MemberFunc>
struct ParentObject;

template <typename C, typename R ,typename ...PARA>
struct ParentObject<R(C::*)(PARA...)>
{
    using type = C;
};

template <typename C, typename R ,typename ...PARA>
struct ParentObject<R(C::*)(PARA...) const>
{
    using type = C;
};

template <typename Func>
struct FunctionTypeInfo;

template <typename C, typename R, typename ...PARA>
struct FunctionTypeInfo<R(C::* &&)(PARA...)>
{
    using RetType = R;
    using ParentClass = C;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename C, typename R, typename ...PARA>
struct FunctionTypeInfo<R(C::*)(PARA...)>
{
    using RetType = R;
    using ParentClass = C;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename C, typename R, typename ...PARA>
struct FunctionTypeInfo<R(C::* &&)(PARA...) const>
{
    using RetType = R;
    using ParentClass = C;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename C, typename R, typename ...PARA>
struct FunctionTypeInfo<R(C::*)(PARA...) const>
{
    using RetType = R;
    using ParentClass = C;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename R, typename ...PARA>
struct FunctionTypeInfo<R(* &&)(PARA...)>
{
    using RetType = R;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename R, typename ...PARA>
struct FunctionTypeInfo<R(*)(PARA...)>
{
    using RetType = R;
    using ParaGroup = TA_MetaTypelist<std::decay_t<PARA>...>;
    static constexpr std::size_t paraSize = sizeof...(PARA);
};

template <typename T>
concept Iterator = requires (T t)
{
    {*std::declval<std::decay_t<T>>()};
    {++std::declval<std::decay_t<T> &>()};
};

template <typename AType>
concept StdAdaptorType = requires(AType at)
{
    typename std::decay_t<AType>::size_type;
    typename std::decay_t<AType>::container_type;
};

template <typename CType>
concept StdContainerType = requires(CType ct)
{
    typename std::decay_t<CType>::value_type;
    typename std::decay_t<CType>::iterator;
    typename std::decay_t<CType>::const_iterator;
    typename std::decay_t<CType>::size_type;
};

template <typename CSType>
concept ValidCST = !std::is_fundamental<std::remove_cvref_t<CSType>>::value &&
                   !std::is_enum<std::remove_cvref_t<CSType>>::value &&
                   !std::is_union<std::remove_cvref_t<CSType>>::value &&
                   !std::is_array<std::remove_cvref_t<CSType>>::value &&
                   !std::is_pointer<std::remove_cvref_t<CSType>>::value &&
                   !std::is_null_pointer<std::remove_cvref_t<CSType>>::value &&
                   !StdContainerType<CSType> &&
                   !StdAdaptorType<CSType> &&
                   std::is_class<std::remove_cvref_t<CSType>>::value;

template <typename CSType>
concept CustomType = requires(CSType ct)
{
    {ct} -> ValidCST;
};

template <typename NCSType>
concept NonCustomType = !CustomType<NCSType>;

template <typename T>
concept IsRawPtr = std::is_pointer_v<std::decay_t<T>>;

template <typename T>
concept RawPtr = requires(T t)
{
    {t} -> IsRawPtr;
};

template <typename T>
concept IsEnumType = std::is_enum_v<std::decay_t<T>>;

template <typename T>
concept EnumType = requires(T t)
{
    {t} -> IsEnumType;
};


}

#endif // TA_TYPEFILTER_H
