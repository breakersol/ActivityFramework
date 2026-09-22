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

#include "TA_SerializationTest.h"
#include "Components/TA_Serialization.h"

#include <cstdint>
#include <fstream>
#include <ios>
#include <memory_resource>
#include <string>

namespace {
template <typename T>
concept CanSerialize = requires(CoreAsync::TA_Serializer<> &writer, const T &value) { writer << value; };
template <typename T>
concept CanDeserialize = requires(CoreAsync::TA_Serializer<CoreAsync::BufferReader> &reader, T &value) { reader >> value; };

struct Unregistered {};
struct UnsupportedAdaptor {
    using size_type = std::size_t;
    using container_type = std::vector<int>;
};

static_assert(CanSerialize<std::string> && CanDeserialize<std::string>);
static_assert(!CanSerialize<std::vector<bool>> && !CanDeserialize<std::vector<bool>>);
static_assert(!CanSerialize<std::pmr::vector<int>> && !CanDeserialize<std::pmr::vector<int>>);
static_assert(!CanSerialize<UnsupportedAdaptor> && !CanDeserialize<UnsupportedAdaptor>);
static_assert(!CanSerialize<Unregistered> && !CanDeserialize<Unregistered>);
static_assert(CanSerialize<std::vector<int>> && CanDeserialize<std::vector<int>>);
static_assert(CanSerialize<std::array<int, 3>> && CanDeserialize<std::array<int, 3>>);
static_assert(CanSerialize<std::map<int, int>> && CanDeserialize<std::map<int, int>>);
static_assert(CanSerialize<M3Test> && CanDeserialize<M3Test>);

// An open, read-only stream deterministically fails when buffered bytes are
// written, without relying on a full disk or platform-specific device paths.
class ReadOnlyBufferWriter : public CoreAsync::TA_BufferWriter {
  public:
    ReadOnlyBufferWriter(const std::string &path, std::size_t size) : TA_BufferWriter(path, size) {
        m_fileStream.close();
        m_fileStream.open(path, std::ios::binary | std::ios::in);
    }
};
} // namespace

#ifdef __ANDROID__
const std::string TEST_FILE_PATH = "/data/local/tmp/test.afw";
#else
const std::string TEST_FILE_PATH = "./test.afw";
#endif

TA_SerializationTest::TA_SerializationTest() {}

TA_SerializationTest::~TA_SerializationTest() {}

void TA_SerializationTest::SetUp() {}

void TA_SerializationTest::TearDown() {}

TEST_F(TA_SerializationTest, StringsRoundTripAndReplaceContents) {
    const std::vector<std::string> values{
        "hello", "", std::string("a\0b", 3), std::string("\xc3\xa9\xff", 3), std::string(4096, 'x')};
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH, 1, 9);
        for (const auto &value : values)
            output << value;
        output << values << std::map<std::string, std::string>{{"key", "value"}};
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH, 1, 9);
    std::string decoded = "previous contents";
    for (const auto &value : values) {
        input >> decoded;
        EXPECT_EQ(decoded, value);
    }
    std::vector<std::string> decodedValues;
    std::map<std::string, std::string> decodedMap;
    input >> decodedValues >> decodedMap;
    EXPECT_EQ(decodedValues, values);
    EXPECT_EQ(decodedMap, (std::map<std::string, std::string>{{"key", "value"}}));
}

TEST_F(TA_SerializationTest, StringWireBytesUseByteCountWithoutTerminator) {
    CoreAsync::TA_Serializer output(TEST_FILE_PATH);
    output << std::string("A\0\xff", 3) << std::string{};
    ASSERT_NO_THROW(output.close());
    std::ifstream file(TEST_FILE_PATH, std::ios::binary);
    file.seekg(16);
    const std::vector<unsigned char> actual((std::istreambuf_iterator<char>(file)), {});
    EXPECT_EQ(actual, (std::vector<unsigned char>{0, 0, 0, 0, 0, 0, 0, 3, 0x41, 0, 0xff,
                                                 0, 0, 0, 0, 0, 0, 0, 0}));
}

TEST_F(TA_SerializationTest, StringsRejectMissingOrExcessiveCountsAndTruncatedPayloads) {
    for (const int scenario : {0, 1, 2}) {
        SCOPED_TRACE(scenario);
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        if (scenario == 1)
            output << std::numeric_limits<std::uint64_t>::max();
        else if (scenario == 2)
            output << std::uint64_t{3} << 'a';
        ASSERT_NO_THROW(output.close());
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
        std::string decoded = "previous contents";
        EXPECT_THROW(input >> decoded, std::ios_base::failure);
        if (scenario != 2)
            EXPECT_EQ(decoded, "previous contents");
    }
}

TEST_F(TA_SerializationTest, WriterOpenFailureThrows) {
    // A regular file cannot be used as a parent directory.
    {
        std::ofstream file(TEST_FILE_PATH);
        ASSERT_TRUE(file.is_open());
        file.close();
        ASSERT_FALSE(file.fail());
    }
    EXPECT_THROW((CoreAsync::TA_Serializer<>(TEST_FILE_PATH + "/child.afw")), std::ios_base::failure);
}

TEST_F(TA_SerializationTest, ExplicitCloseReportsCompletionAndRejectsFurtherWrites) {
    CoreAsync::TA_Serializer<> output(TEST_FILE_PATH);
    output << std::uint32_t{42};
    ASSERT_NO_THROW(output.close());
    EXPECT_NO_THROW(output.close());
    EXPECT_THROW(output << std::uint32_t{99}, std::ios_base::failure);
    EXPECT_THROW(output.flush(), std::ios_base::failure);
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    std::uint32_t value{};
    input >> value;
    EXPECT_EQ(value, 42u);
    EXPECT_THROW(input >> value, std::ios_base::failure);
}

TEST_F(TA_SerializationTest, FlushMakesDataVisibleBeforeClose) {
    CoreAsync::TA_Serializer<> output(TEST_FILE_PATH);
    output << std::uint32_t{42};
    ASSERT_NO_THROW(output.flush());
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    std::uint32_t value{};
    input >> value;
    EXPECT_EQ(value, 42u);
    ASSERT_NO_THROW(output.close());
}

TEST_F(TA_SerializationTest, BufferFlushAndFinishReportWriteFailure) {
    ReadOnlyBufferWriter output(TEST_FILE_PATH, 8);
    ASSERT_TRUE(output.isValid());
    std::uint64_t value = 42;
    ASSERT_TRUE(output.write(value));
    EXPECT_FALSE(output.flush());
    EXPECT_FALSE(output.write(value));
    EXPECT_FALSE(output.finish());
    EXPECT_FALSE(output.finish());
}

TEST_F(TA_SerializationTest, AutomaticBufferFlushStopsOnFailure) {
    ReadOnlyBufferWriter output(TEST_FILE_PATH, 8);
    std::uint64_t value = 42;
    ASSERT_TRUE(output.write(value));
    EXPECT_FALSE(output.write(value));
    EXPECT_FALSE(output.finish());
}

TEST_F(TA_SerializationTest, EnumUnderlyingTypesRoundTrip) {
    enum class Byte : std::uint8_t { Value = 255 };
    enum class SignedByte : std::int8_t { Value = -128 };
    enum class Wide : std::uint16_t { Value = 256 };
    enum class Signed : std::int32_t { Value = -123456 };
    enum class Large : std::uint64_t { Value = 0xfedcba9876543210ULL };
    enum class SignedLarge : std::int64_t { Value = -0x123456789abcdefLL };
    enum Unscoped { UnscopedValue = 65536 };

    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << Byte::Value << SignedByte::Value << Wide::Value << Signed::Value
               << Large::Value << SignedLarge::Value << UnscopedValue << std::uint32_t{42};
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    Byte byte{};
    SignedByte signedByte{};
    Wide wide{};
    Signed signedValue{};
    Large large{};
    SignedLarge signedLarge{};
    Unscoped unscoped{};
    std::uint32_t following = 0;
    input >> byte >> signedByte >> wide >> signedValue >> large >> signedLarge >> unscoped >> following;

    EXPECT_EQ(byte, Byte::Value);
    EXPECT_EQ(signedByte, SignedByte::Value);
    EXPECT_EQ(wide, Wide::Value);
    EXPECT_EQ(signedValue, Signed::Value);
    EXPECT_EQ(large, Large::Value);
    EXPECT_EQ(signedLarge, SignedLarge::Value);
    EXPECT_EQ(unscoped, UnscopedValue);
    EXPECT_EQ(following, 42u);
}

TEST_F(TA_SerializationTest, TruncatedEnumPreservesDestination) {
    enum class Wide : std::uint32_t { Value = 0x12345678 };
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << std::uint8_t{1};
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    Wide value = Wide::Value;

    EXPECT_THROW(input >> value, std::ios_base::failure);
    EXPECT_EQ(value, Wide::Value);
}

TEST_F(TA_SerializationTest, SmallWriterBuffersPreserveSerializedValues) {
    for (const std::size_t bufferSize : {0u, 1u, 7u, 8u, 9u, 10u}) {
        SCOPED_TRACE(bufferSize);
        {
            CoreAsync::TA_Serializer output(TEST_FILE_PATH, 3, bufferSize);
            for (int i = 0; i < 3; ++i)
                output << std::uint8_t{0x12} << std::uint64_t{0x123456789abcdef0ULL}
                       << std::uint16_t{0x3456};
            ASSERT_NO_THROW(output.close());
        }
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH, 3);
        EXPECT_EQ(input.version(), 3u);
        for (int i = 0; i < 3; ++i) {
            std::uint8_t first = 0;
            std::uint64_t second = 0;
            std::uint16_t third = 0;
            input >> first >> second >> third;
            EXPECT_EQ(first, 0x12u);
            EXPECT_EQ(second, 0x123456789abcdef0ULL);
            EXPECT_EQ(third, 0x3456u);
        }
    }
}

TEST_F(TA_SerializationTest, WriterBufferGrowthPreservesPendingBytes) {
    std::uint8_t prefix = 0x12;
    std::uint64_t value = 0x123456789abcdef0ULL;
    std::uint8_t suffix = 0x34;
    {
        CoreAsync::TA_BufferWriter output(TEST_FILE_PATH, 1);
        ASSERT_TRUE(output.write(prefix));
        ASSERT_TRUE(output.write(value));
        ASSERT_TRUE(output.write(suffix));
        ASSERT_TRUE(output.finish());
    }
    CoreAsync::TA_BufferReader input(TEST_FILE_PATH, 32);
    std::uint8_t actualPrefix = 0;
    std::uint64_t actualValue = 0;
    std::uint8_t actualSuffix = 0;
    ASSERT_TRUE(input.read(actualPrefix));
    ASSERT_TRUE(input.read(actualValue));
    ASSERT_TRUE(input.read(actualSuffix));
    EXPECT_EQ(actualPrefix, prefix);
    EXPECT_EQ(actualValue, value);
    EXPECT_EQ(actualSuffix, suffix);
    EXPECT_FALSE(input.read(actualSuffix));
}

TEST_F(TA_SerializationTest, TruncatedScalarStopsChainedExtraction) {
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << std::uint32_t{42} << std::uint8_t{1};
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    std::uint32_t first = 0;
    std::uint32_t truncated = 0x12345678;
    std::uint32_t following = 99;

    EXPECT_THROW(input >> first >> truncated >> following, std::ios_base::failure);
    EXPECT_EQ(first, 42u);
    EXPECT_EQ(truncated, 0x12345678u);
    EXPECT_EQ(following, 99u);
}

TEST_F(TA_SerializationTest, TruncatedListDoesNotInsertUnreadElement) {
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << std::uint64_t{2} << std::uint32_t{42};
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    std::list<std::uint32_t> values;

    EXPECT_THROW(input >> values, std::ios_base::failure);
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values.front(), 42u);
}

TEST_F(TA_SerializationTest, MissingContainerCountPreservesDestination) {
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        ASSERT_NO_THROW(output.close());
    }
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
    std::vector<int> values = {7, 8};
    const auto original = values;

    EXPECT_THROW(input >> values, std::ios_base::failure);
    EXPECT_EQ(values, original);
}

TEST_F(TA_SerializationTest, TruncatedVersionHeaderThrows) {
    {
        std::ofstream output(TEST_FILE_PATH, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(output.is_open());
        output.put('\0');
        output.close();
        ASSERT_FALSE(output.fail());
    }

    EXPECT_THROW((CoreAsync::TA_Serializer<CoreAsync::BufferReader>(TEST_FILE_PATH)),
                 std::ios_base::failure);
}

TEST_F(TA_SerializationTest, WireBytesUseFixedWidthBigEndianFields) {
    CoreAsync::TA_Serializer output(TEST_FILE_PATH, 0x0102030405060708ULL);
    output << std::vector<std::uint16_t>{0x1234, 0xabcd} << true << false << 1.0f;
    ASSERT_NO_THROW(output.close());
    std::ifstream file(TEST_FILE_PATH, std::ios::binary);
    const std::vector<unsigned char> actual((std::istreambuf_iterator<char>(file)), {});
    const std::vector<unsigned char> expected{
        0x41, 0x46, 0x57, 0x53, 0x00, 0x01, 0x00, 0x00, // magic, revision, flags
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // schema
        0, 0, 0, 0, 0, 0, 0, 2,                       // uint64 count
        0x12, 0x34, 0xab, 0xcd, 1, 0, 0x3f, 0x80, 0, 0};
    EXPECT_EQ(actual, expected);
}

TEST_F(TA_SerializationTest, ReadsIndependentWireFixture) {
    const unsigned char bytes[]{
        0x41, 0x46, 0x57, 0x53, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 2,
        0x12, 0x34, 0xab, 0xcd, 1, 0, 0x3f, 0x80, 0, 0};
    std::ofstream output(TEST_FILE_PATH, std::ios::binary);
    output.write(reinterpret_cast<const char *>(bytes), sizeof(bytes));
    output.close();
    ASSERT_FALSE(output.fail());
    CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH, 2, 9);
    std::vector<std::uint16_t> values;
    bool yes = false, no = true;
    float value{};
    input >> values >> yes >> no >> value;
    EXPECT_EQ(input.version(), 1u);
    EXPECT_EQ(values, (std::vector<std::uint16_t>{0x1234, 0xabcd}));
    EXPECT_TRUE(yes);
    EXPECT_FALSE(no);
    EXPECT_EQ(value, 1.0f);
}

TEST_F(TA_SerializationTest, RejectsInvalidAndTruncatedHeaders) {
    const std::vector<unsigned char> valid{
        0x41, 0x46, 0x57, 0x53, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    auto rejects = [&](const std::vector<unsigned char> &bytes) {
        std::ofstream output(TEST_FILE_PATH, std::ios::binary);
        output.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        output.close();
        ASSERT_FALSE(output.fail());
        EXPECT_THROW((CoreAsync::TA_Serializer<CoreAsync::BufferReader>(TEST_FILE_PATH)), std::ios_base::failure);
    };
    for (std::size_t size = 0; size < valid.size(); ++size) {
        SCOPED_TRACE(size);
        rejects(std::vector<unsigned char>(valid.begin(), valid.begin() + size));
    }
    for (const std::size_t index : {0u, 5u, 7u, 15u}) {
        SCOPED_TRACE(index);
        auto invalid = valid;
        invalid[index] = 2; // bad magic, revision, flags, or unsupported schema
        rejects(invalid);
    }
    auto zeroSchema = valid;
    zeroSchema[15] = 0;
    rejects(zeroSchema);
    // Legacy header followed by arbitrary payload must not be interpreted as AFWS.
    std::vector<unsigned char> legacy(24, 0);
    legacy[0] = 1;
    rejects(legacy);
    EXPECT_THROW((CoreAsync::TA_Serializer<>(TEST_FILE_PATH, 0)), std::ios_base::failure);
}

TEST_F(TA_SerializationTest, CountsAreFixedWidthForArraysAndAdaptors) {
    CoreAsync::TA_Serializer output(TEST_FILE_PATH);
    output << std::array<std::uint8_t, 1>{7};
    std::queue<std::uint8_t> queue;
    queue.push(9);
    output << queue;
    ASSERT_NO_THROW(output.close());
    std::ifstream file(TEST_FILE_PATH, std::ios::binary);
    file.seekg(16);
    const std::vector<unsigned char> actual((std::istreambuf_iterator<char>(file)), {});
    EXPECT_EQ(actual, (std::vector<unsigned char>{0, 0, 0, 0, 0, 0, 0, 1, 7,
                                                 0, 0, 0, 0, 0, 0, 0, 1, 9}));
}

TEST_F(TA_SerializationTest, InvalidCountsAndBooleansPreserveDestination) {
    for (const std::uint64_t count : {std::uint64_t{1}, std::uint64_t{3}}) {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << count;
        ASSERT_NO_THROW(output.close());
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
        std::array<int, 2> values{7, 8};
        EXPECT_THROW(input >> values, std::ios_base::failure);
        EXPECT_EQ(values, (std::array<int, 2>{7, 8}));
    }
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << std::numeric_limits<std::uint64_t>::max();
        ASSERT_NO_THROW(output.close());
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
        std::vector<std::uint64_t> values{7};
        EXPECT_THROW(input >> values, std::ios_base::failure);
        EXPECT_EQ(values, (std::vector<std::uint64_t>{7}));
    }
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH);
        output << std::uint8_t{2};
        ASSERT_NO_THROW(output.close());
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH);
        bool value = true;
        EXPECT_THROW(input >> value, std::ios_base::failure);
        EXPECT_TRUE(value);
    }
}

TEST_F(TA_SerializationTest, CustomTypeTest) {
    float *ptr = new float(5.3);
    M3Test t, p1, p2;
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH, 1, 1024);
        t.setVec({2, 3, 4, 5});
        t.setRawPtr(ptr);
        t.setArray({2, 3, 4, 5, 6});
        t.setList({9, 9, 9});
        t.setForwardList({9, 9, 9});
        t.setDeque({8, 7, 6, 5, 4});
        t.setStack({t.getDeque().begin(), t.getDeque().end()});
        t.mx = 999;
        t.m_vec = {1, 1, 1, 1};
        t.setQueue({t.getDeque().begin(), t.getDeque().end()});
        t.setPrioritQueue({t.getDeque().begin(), t.getDeque().end()});
        output << t << t;
        EXPECT_NO_THROW(output.close());
    }
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH, 1, 1024);
        input >> p1 >> p2;
    }
    EXPECT_EQ(t.getVec(), p2.getVec());
    EXPECT_EQ(*t.getRawPtr(), *p2.getRawPtr());
    EXPECT_EQ(t.getArray(), p2.getArray());
    EXPECT_EQ(t.getList(), p2.getList());
    EXPECT_EQ(t.getForwardList(), p2.getForwardList());
    EXPECT_EQ(t.getDeque(), p2.getDeque());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(5, p2.mx);
    EXPECT_EQ(t.m_vec, p2.m_vec);
    EXPECT_EQ(t.getQueue(), p2.getQueue());

    auto pq1{t.getPriorityQueue()};
    auto pq2{p2.getPriorityQueue()};
    EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));

    delete ptr;
}

TEST_F(TA_SerializationTest, VersionTest) {
    float *ptr = new float(5.3);
    M3Test t, p1, p2;
    {
        CoreAsync::TA_Serializer output(TEST_FILE_PATH, 2);
        t.setVec({2, 3, 4, 5});
        t.setRawPtr(ptr);
        t.setArray({2, 3, 4, 5, 6});
        t.setList({9, 9, 9});
        t.setForwardList({9, 9, 9});
        t.setDeque({8, 7, 6, 5, 4});
        t.setStack({t.getDeque().begin(), t.getDeque().end()});
        t.mx = 999;
        t.m_vec = {1, 1, 1, 1};
        t.setQueue({t.getDeque().begin(), t.getDeque().end()});
        t.setPrioritQueue({t.getDeque().begin(), t.getDeque().end()});
        output << t << t;
        EXPECT_NO_THROW(output.close());
    }
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input(TEST_FILE_PATH, 2);
        input >> p1 >> p2;
    }
    EXPECT_EQ(t.getVec(), p2.getVec());
    EXPECT_EQ(*t.getRawPtr(), *p2.getRawPtr());
    EXPECT_EQ(t.getArray(), p2.getArray());
    EXPECT_EQ(t.getList(), p2.getList());
    EXPECT_EQ(t.getForwardList(), p2.getForwardList());
    EXPECT_EQ(t.getDeque(), p2.getDeque());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.getStack(), p2.getStack());
    EXPECT_EQ(t.mx, p2.mx);
    EXPECT_EQ(t.m_vec, p2.m_vec);
    EXPECT_EQ(t.getQueue(), p2.getQueue());

    auto pq1{t.getPriorityQueue()};
    auto pq2{p2.getPriorityQueue()};
    EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));

    delete ptr;
}

// TEST_F(TA_SerializationTest, LargeScaleTest)
// {
//     CoreAsync::TA_Serializer output("./test.afw", 2, 10);
//     M3Test t;
//     for(std::size_t i = 0;i < 1000;++i)
//     {
//         output << t;
//     }
//     ASSERT_NO_THROW(output.close());
//     std::vector<M3Test> vec(1000);
//     CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw", 2, 10);
//     for(std::size_t i = 0;i < 1000;++i)
//     {
//         input >> vec[i];
//     }

//     EXPECT_EQ(t.getVec(), vec[999].getVec());
//     EXPECT_EQ(*t.getRawPtr(), *vec[999].getRawPtr());
//     EXPECT_EQ(t.getArray(), vec[999].getArray());
//     EXPECT_EQ(t.getList(), vec[999].getList());
//     EXPECT_EQ(t.getForwardList(), vec[999].getForwardList());
//     EXPECT_EQ(t.getDeque(), vec[999].getDeque());
//     EXPECT_EQ(t.getStack(), vec[999].getStack());
//     EXPECT_EQ(t.getStack(), vec[999].getStack());
//     EXPECT_EQ(t.mx, vec[999].mx);
//     EXPECT_EQ(t.m_vec, vec[999].m_vec);
//     EXPECT_EQ(t.getQueue(), vec[999].getQueue());

//     auto pq1 {t.getPriorityQueue()};
//     auto pq2 {vec[999].getPriorityQueue()};
//     EXPECT_TRUE(arePriorityQueueEqual(pq1, pq2));
// }
