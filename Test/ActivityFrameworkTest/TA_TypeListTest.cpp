#include "Components/TA_TypeList.h"

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>

namespace {
using namespace CoreAsync;

template <typename... T> using List = TA_MetaTypelist<T...>;
template <typename Source, typename Dest = List<>>
using IntegralTypes = typename MetaFilter<Source, std::is_integral, Dest>::result;
template <typename Source, typename Dest = List<>>
using IntegralPointers = typename MetaFilterMapper<Source, std::is_integral, std::add_pointer, Dest>::result;

struct IntMember { using type = int; };
struct LongMember { using type = long; };
template <typename T> struct MemberType { using type = typename T::type; };
template <typename T> struct RejectAll : std::false_type {};
template <typename T> struct UndefinedMapper;

// Restore cout even if a test exits early or throws.
class CoutCapture {
  public:
    CoutCapture() : m_previous(std::cout.rdbuf(m_output.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(m_previous); }
    CoutCapture(const CoutCapture &) = delete;
    CoutCapture &operator=(const CoutCapture &) = delete;
    std::string str() const { return m_output.str(); }

  private:
    std::ostringstream m_output;
    std::streambuf *m_previous;
};

TEST(TA_TypeListTest, sizeAndTypeAliases) {
    EXPECT_EQ(List<>::size, 0u);
    EXPECT_EQ((MetaSize<List<int, double>>::value), 2u);
    EXPECT_EQ((TA_MetaValue<int, 42>::m_value), 42);
    EXPECT_TRUE((std::is_same_v<List<int, double>::Variant, std::variant<int, double>>));
    EXPECT_TRUE((std::is_same_v<List<const int &, double &&, const char>::Tuple,
                              std::tuple<const int &, double, char>>));
    EXPECT_TRUE((std::is_same_v<TA_MetaPair<int, double>::First, int>));
    EXPECT_TRUE((std::is_same_v<TA_MetaPair<int, double>::Second, double>));
}

TEST(TA_TypeListTest, appendAndMergePreserveOrderAndDuplicates) {
    EXPECT_TRUE((std::is_same_v<MetaPushBack<List<>, int>::type, List<int>>));
    EXPECT_TRUE((std::is_same_v<MetaPushBack<List<int>, int>::type, List<int, int>>));
    EXPECT_TRUE((std::is_same_v<MetaAppend<List<>, List<>>::type, List<>>));
    EXPECT_TRUE((std::is_same_v<MetaAppend<List<int>>::type, List<int>>));
    EXPECT_TRUE((std::is_same_v<MetaAppend<List<int>, List<double>>::type, List<int, double>>));
    EXPECT_TRUE((std::is_same_v<MetaMerge<List<int>>::type, List<int>>));
    EXPECT_TRUE((std::is_same_v<MetaMerge<List<int>, List<>, List<double, int>>::type,
                              List<int, double, int>>));
}

TEST(TA_TypeListTest, sameHandlesEmptyAndUnequalLengths) {
    EXPECT_TRUE((MetaSame<List<>, List<>>::value));
    EXPECT_FALSE((MetaSame<List<>, List<int>>::value));
    EXPECT_FALSE((MetaSame<List<int>, List<>>::value));
    EXPECT_FALSE((MetaSame<List<int>, List<int, double>>::value));
    EXPECT_FALSE((MetaSame<List<int, double>, List<int>>::value));
}

TEST(TA_TypeListTest, sameComparesDecayedTypesInOrder) {
    EXPECT_TRUE((MetaSame<List<int, double>, List<int, double>>::value));
    EXPECT_FALSE((MetaSame<List<int, double>, List<double, int>>::value));
    EXPECT_FALSE((MetaSame<List<int, double>, List<int, float>>::value));
    EXPECT_TRUE((MetaSame<List<const int &, volatile double &&>, List<int, double>>::value));
    EXPECT_TRUE((MetaSame<List<int[3], void(int)>, List<int *, void (*)(int)>>::value));
}

TEST(TA_TypeListTest, containsAndTypeAtUseExactTypes) {
    EXPECT_FALSE((MetaContains<List<>, int>::value));
    EXPECT_TRUE((MetaContains<List<float, int &, double>, int &>::value));
    EXPECT_FALSE((MetaContains<List<int &>, int>::value));
    EXPECT_TRUE((std::is_same_v<MetaTypeAt<List<int &, float, const double>, 0>::type, int &>));
    EXPECT_TRUE((std::is_same_v<MetaTypeAt<List<int &, float, const double>, 2>::type, const double>));
}

TEST(TA_TypeListTest, mapperTransformsEveryType) {
    EXPECT_TRUE((std::is_same_v<MetaMapper<List<>, std::add_pointer>::type, List<>>));
    EXPECT_TRUE((std::is_same_v<MetaMapper<List<int, double, int>, std::add_pointer>::type,
                              List<int *, double *, int *>>));
}

TEST(TA_TypeListTest, filterPreservesDestinationAndMatchingOrder) {
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<int, float, long>, List<char, bool>>,
                              List<char, bool, int, long>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<int, float, int>>, List<int, int>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<int, long>, List<char>>, List<char, int, long>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<int, List<char>>, List<char, int>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<float, List<char>>, List<char>>));
}

TEST(TA_TypeListTest, filterHandlesEmptyAndRejectedLists) {
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<>>, List<>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<>, List<char>>, List<char>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<float, double>>, List<>>));
    EXPECT_TRUE((std::is_same_v<IntegralTypes<List<float, double>, List<char>>, List<char>>));
}

TEST(TA_TypeListTest, filterMapperIncludesDestinationOnlyOnce) {
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<int, long>, List<char>>, List<char, int *, long *>>));
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<float, int, double, long>, List<char, bool>>,
                              List<char, bool, int *, long *>>));
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<int, int>, List<int *>>, List<int *, int *, int *>>));
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<int, float, long>>, List<int *, long *>>));
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<>, List<char>>, List<char>>));
    EXPECT_TRUE((std::is_same_v<IntegralPointers<List<float, double>, List<char>>, List<char>>));
}

TEST(TA_TypeListTest, filterMapperDoesNotInstantiateRejectedMappings) {
    // MemberType<int> is ill-formed: this must compile without instantiating it.
    using Mixed = MetaFilterMapper<List<int, IntMember, double, LongMember>,
                                   std::is_class, MemberType, List<char>>::result;
    using Rejected = MetaFilterMapper<List<int>, std::is_class, MemberType>::result;
    using AllRejected = MetaFilterMapper<List<int, double>, RejectAll, UndefinedMapper, List<char>>::result;
    EXPECT_TRUE((std::is_same_v<Mixed, List<char, int, long>>));
    EXPECT_TRUE((std::is_same_v<Rejected, List<>>));
    EXPECT_TRUE((std::is_same_v<AllRejected, List<char>>));
}

TEST(TA_TypeListTest, findAndMatchReturnFirstMatchOrNull) {
    EXPECT_TRUE((std::is_same_v<MetaFind<List<float, int, long>, std::is_integral>::result, int>));
    EXPECT_TRUE((std::is_same_v<MetaFind<List<float>, std::is_integral>::result, std::nullptr_t>));
    EXPECT_TRUE((std::is_same_v<MetaFind<List<>, std::is_integral>::result, std::nullptr_t>));
    EXPECT_TRUE((std::is_same_v<MetaFind<int, std::is_integral>::result, int>));
    EXPECT_TRUE((std::is_same_v<MetaMatch<List<float, int, long>, double, std::is_convertible>::type, float>));
    EXPECT_TRUE((std::is_same_v<MetaMatch<List<float>, int, std::is_same>::type, std::nullptr_t>));
    EXPECT_TRUE((std::is_same_v<MetaMatch<List<>, int, std::is_same>::type, std::nullptr_t>));
}

TEST(TA_TypeListTest, deduplicationKeepsLastOccurrences) {
    EXPECT_TRUE((std::is_same_v<MetaRemoveDuplicate<List<>>::result, List<>>));
    EXPECT_TRUE((std::is_same_v<MetaRemoveDuplicate<List<int>>::result, List<int>>));
    EXPECT_TRUE((std::is_same_v<MetaRemoveDuplicate<List<int, double, int, char, double>>::result,
                              List<int, char, double>>));
    EXPECT_TRUE((std::is_same_v<MetaRemoveDuplicate<List<int, const int, int &>>::result,
                              List<int, const int, int &>>));
}

TEST(TA_TypeListTest, variantUsesDeduplicatedAlternatives) {
    using Variant = MetaVariant<List<int, double, int>>::Var;
    EXPECT_TRUE((std::is_same_v<Variant, std::variant<double, int>>));
    Variant value{42};
    EXPECT_EQ(value.index(), 1u);
    EXPECT_EQ(std::get<int>(value), 42);
}

TEST(TA_TypeListTest, printEmptyListProducesNoOutput) {
    CoutCapture capture;
    printTypeList(List<>{});
    EXPECT_TRUE(capture.str().empty());
}

TEST(TA_TypeListTest, printSingleTypeTerminates) {
    CoutCapture capture;
    printTypeList(List<int>{});
    EXPECT_EQ(capture.str(), std::string(typeid(int).name()) + '\n');
}

TEST(TA_TypeListTest, printMultipleTypesPreservesOrder) {
    CoutCapture capture;
    printTypeList(List<int, double, int>{});
    EXPECT_EQ(capture.str(), std::string(typeid(int).name()) + '\n' + typeid(double).name() + '\n'
                                 + typeid(int).name() + '\n');
}
} // namespace
