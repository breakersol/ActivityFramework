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

#ifndef TA_METALIST_H
#define TA_METALIST_H

#include <type_traits>
#include <iostream>
#include <variant>

namespace CoreAsync {
    struct TA_MetaNull
    {

    };

    template <typename V, V value>
    struct TA_MetaValue
    {
        static constexpr V m_value = value;
    };

    template <typename ...T>
    struct TA_MetaTypelist
    {
        using Variant = typename std::variant<T...>;
        static constexpr std::size_t size = sizeof...(T);
    };

    template <typename N>
    class MetaSize;

    template <typename ...para>
    class MetaSize<TA_MetaTypelist<para...> >
    {
    public:
        static constexpr std::size_t value = sizeof... (para);
    };

    template <typename Source, typename NewEle>
    class MetaPushBack;

    template <typename ...Source, typename NewEle>
    class MetaPushBack<TA_MetaTypelist<Source...>, NewEle>
    {
    public:
        using type = TA_MetaTypelist<Source..., NewEle>;
    };

    template <typename Source, typename NewEle = TA_MetaTypelist<>>
    class MetaAppend;

    template <typename ...Source, typename ...NewEle>
    class MetaAppend<TA_MetaTypelist<Source...>, TA_MetaTypelist<NewEle...> >
    {
    public:
        using type = TA_MetaTypelist<Source...,NewEle...>;
    };

    template <>
    class MetaAppend<TA_MetaTypelist<>, TA_MetaTypelist<> >
    {
    public:
        using type = TA_MetaTypelist<>;
    };

    template <typename Se, typename ...Rm>
    class MetaMerge
    {
      public:
        using type = typename MetaAppend<typename MetaAppend<TA_MetaTypelist<>, Se>::type, typename MetaMerge<Rm...>::type>::type;
    };

    template <typename Se, typename Ne>
    class MetaMerge<Se,Ne>
    {
      public:
        using type = typename MetaAppend<Se,Ne>::type;
    };

    template <typename Se>
    class MetaMerge<Se>
    {
      public:
        using type = Se;
    };

    template <typename Source, typename Dest>
    class MetaSame;

    template <typename SPARA, typename ...SPARAS, typename DPARA, typename ...DPARAS>
    class MetaSame<TA_MetaTypelist<SPARA, SPARAS...>, TA_MetaTypelist<DPARA, DPARAS...> >
    {
    public:
        static constexpr bool value = sizeof...(SPARAS) != sizeof...(DPARAS) ? false : std::is_same_v<std::decay_t<SPARA>,std::decay_t<DPARA> > ? MetaSame<TA_MetaTypelist<SPARAS...>, TA_MetaTypelist<DPARAS...> >::value : false;
    };

    template <>
    class MetaSame<TA_MetaTypelist<>, TA_MetaTypelist<> >
    {
    public:
        static constexpr bool value {true};
    };

    template <typename Source, typename T> class MetaContains;

    template <typename Head,typename ... Source, typename T>
    class MetaContains<TA_MetaTypelist<Head,Source...>, T> {
    private:
        using Result = MetaContains<TA_MetaTypelist<Source...>, T>;
    public:
        static constexpr bool value = std::is_same<Head, T>::value ? true : Result::value;
    };

    template <typename T>
    class MetaContains<TA_MetaTypelist<>, T> {
    public:
        static constexpr bool value = false;
    };

    template <typename Source, unsigned int Idx>
    class MetaTypeAt;

    template <typename Head, typename ...Source>
    class MetaTypeAt<TA_MetaTypelist<Head,Source...>, 0 >
    {
    public:
        using type = Head;
    };

    template <typename Head, typename ...Source, unsigned int i>
    class MetaTypeAt<TA_MetaTypelist<Head,Source...>, i >
    {
    public:
        static_assert (i < sizeof... (Source) + 1, "Index out of range.");
        using type = typename MetaTypeAt<TA_MetaTypelist<Source...>, i - 1 >::type;
    };

//    template <unsigned int i>
//    class TypeAt<TA_MetaTypelist<>, i>
//    {
//    public:
//        using type = std::nullptr_t;
//    };

    template <typename M, typename N>
    class TA_MetaPair
    {
    public:
        using First = M;
        using Second = N;
    };

    template <typename Source, template <typename T> class FUNC>
    struct MetaMapper;

    template<typename ...ELE, template <typename T> class FUNC>
    struct MetaMapper<TA_MetaTypelist<ELE...>, FUNC>
    {
        using type = TA_MetaTypelist<typename FUNC<ELE>::type... >;
    };

    template <typename Source, template <typename T> class FUNC, typename Dest = TA_MetaTypelist<> >
    struct MetaFilter
    {
        using result = typename std::conditional<FUNC<Source>::value,typename MetaPushBack<Dest,Source>::type,Dest>::type;
    };

    template <typename Head, typename ...Source, template <typename T> class FUNC>
    struct MetaFilter<TA_MetaTypelist<Head, Source...>, FUNC>
    {
        using result = typename MetaAppend<typename MetaFilter<Head,FUNC>::result, typename MetaFilter<TA_MetaTypelist<Source...>,FUNC>::result>::type;
    };

    template <template <typename T> class FUNC, typename Dest>
    struct MetaFilter<TA_MetaTypelist<>, FUNC, Dest>
    {
        using result = Dest;
    };

    template <typename List, template <typename T> class FilterFunc, template <typename M> class MapFunc, typename Dest = TA_MetaTypelist<> >
    struct MetaFilterMapper;

    template <typename Single, template <typename T> class FilterFunc, template <typename M> class MapFunc, typename Dest>
    struct MetaFilterMapper<TA_MetaTypelist<Single>, FilterFunc, MapFunc, Dest>
    {
        using result = std::conditional_t<FilterFunc<Single>::value, typename MetaPushBack<Dest, typename MapFunc<Single>::type>::type, Dest>;
    };

    template <template <typename T> class FilterFunc, template <typename M> class MapFunc, typename Dest>
    struct MetaFilterMapper<TA_MetaTypelist<>, FilterFunc, MapFunc, Dest>
    {
        using result = Dest;
    };

    template <typename Head, typename ...Tails, template <typename T> class FilterFunc, template <typename M> class MapFunc, typename Dest>
    struct MetaFilterMapper<TA_MetaTypelist<Head, Tails...>, FilterFunc, MapFunc, Dest>
    {
        using result = typename MetaAppend<typename MetaFilterMapper<TA_MetaTypelist<Head>, FilterFunc, MapFunc, Dest>::result, typename MetaFilterMapper<TA_MetaTypelist<Tails...>, FilterFunc, MapFunc, Dest>::result>::type;
    };

    template <typename Source, template <typename T> class FUNC>
    struct MetaFind
    {
        using result = typename std::conditional<FUNC<Source>::value,Source,std::nullptr_t>::type;
    };

    template <typename Head, typename ...Tail, template <typename T> class FUNC>
    struct MetaFind<TA_MetaTypelist<Head,Tail...>, FUNC>
    {
        using result = typename std::conditional<FUNC<Head>::value, Head, typename MetaFind<TA_MetaTypelist<Tail...>,FUNC>::result>::type;
    };

    template <template <typename T> class FUNC>
    struct MetaFind<TA_MetaTypelist<>, FUNC>
    {
        using result = std::nullptr_t;
    };

    template <typename Source, typename Dest,template <typename S, typename D> class FUNC>
    struct MetaMatch;

    template <typename Head, typename ...Tail, typename Dest, template <typename S, typename D> class FUNC>
    struct MetaMatch<TA_MetaTypelist<Head,Tail...>, Dest, FUNC>
    {
        using type = typename std::conditional<FUNC<Head,Dest>::value, Head, typename MetaMatch<TA_MetaTypelist<Tail...>,Dest,FUNC>::type >::type;
    };

    template <typename Dest, template <typename S, typename D> class FUNC>
    struct MetaMatch<TA_MetaTypelist<>, Dest, FUNC>
    {
        using type = std::nullptr_t;
    };

    template <typename TList>
    struct MetaRemoveDuplicate;

    template <typename Fir, typename ...Ele>
    struct MetaRemoveDuplicate<TA_MetaTypelist<Fir,Ele...>>
    {
        using result = typename MetaAppend<std::conditional_t<MetaContains<TA_MetaTypelist<Ele...>,Fir>::value,TA_MetaTypelist<>,TA_MetaTypelist<Fir>>, typename MetaRemoveDuplicate<TA_MetaTypelist<Ele...>>::result>::type;
    };

    template <typename Last>
    struct MetaRemoveDuplicate<TA_MetaTypelist<Last> >
    {
        using result = TA_MetaTypelist<Last>;
    };

    template <>
    struct MetaRemoveDuplicate<TA_MetaTypelist<> >
    {
        using result = TA_MetaTypelist<>;
    };

    template <typename L>
    struct MetaVariant;

    template <typename ...LT>
    struct MetaVariant<TA_MetaTypelist<LT...>>
    {
        using Var = typename MetaRemoveDuplicate<TA_MetaTypelist<LT...>>::result::Variant;
    };

    template <typename T>
    void printType(T)
    {
        std::cout << typeid (T).name() << std::endl;
    }

    template <typename Head, typename ...Tail>
    void printTypeList(TA_MetaTypelist<Head,Tail...>)
    {
        printType(Head());
        printTypeList(TA_MetaTypelist<Tail...>());
    }

//    void printTypeList(TA_MetaTypelist<>)
//    {
//        std::cout << "At type list end." << std::endl;
//    }
}

#endif // TA_METALIST_H
