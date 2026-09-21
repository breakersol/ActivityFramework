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

#ifndef TA_BUFFER_H
#define TA_BUFFER_H

#include <fstream>
#include <vector>

#include "TA_EndianConversion.h"
#include "TA_CommonTools.h"
#include "TA_MetaStringView.h"

namespace CoreAsync {

class TA_BufferReader;
class TA_BufferWriter;

template <typename Opt> class TA_BasicBufferOperator {
  public:
    using OperatorType = Opt;

    TA_BasicBufferOperator() = delete;

    virtual ~TA_BasicBufferOperator() { close(); }

    TA_BasicBufferOperator(const TA_BasicBufferOperator &op) = delete;
    TA_BasicBufferOperator(TA_BasicBufferOperator &&op) = delete;

    bool isValid() const { return m_fileStream.is_open(); }

    template <typename T> bool read(T &t) {
        static_assert(std::is_same_v<Opt, TA_BufferReader>, "Read is not the member of current type");
        return static_cast<Opt *>(this)->read(t);
    }

    template <typename T> bool write(T &t) {
        static_assert(std::is_same_v<Opt, TA_BufferWriter>, "Write is not the member of current type");
        return static_cast<Opt *>(this)->write(t);
    }

    bool flush() {
        static_assert(std::is_same_v<Opt, TA_BufferWriter>, "Flush is not the member of current type");
        return static_cast<Opt *>(this)->flush();
    }

    bool finish() {
        static_assert(std::is_same_v<Opt, TA_BufferWriter>, "Finish is not the member of current type");
        return static_cast<Opt *>(this)->finish();
    }

  protected:
    TA_BasicBufferOperator(const std::string &file, std::size_t size) : m_buffer(size) {}

  private:
    void close() {
        if (m_fileStream.is_open())
            m_fileStream.close();
    }

  protected:
    std::fstream m_fileStream;
    std::vector<char> m_buffer;
    std::size_t m_offset{0}, m_validSize{0};
};

class TA_BufferReader : public TA_BasicBufferOperator<TA_BufferReader> {
  public:
    TA_BufferReader() = delete;
    TA_BufferReader(const std::string &file, std::size_t size, std::size_t pos = 0)
        : CoreAsync::TA_BasicBufferOperator<TA_BufferReader>(file, size) {
        init(file, pos);
    }

    ~TA_BufferReader() {}

    TA_BufferReader(const TA_BufferReader &writer) = delete;
    TA_BufferReader(TA_BufferReader &&writer) = delete;

    template <EndianConvertedType T> bool read(T &t) {
        if (isEnd() || m_validSize - m_offset < sizeof(t)) {
            if (!fillBuffer() || m_validSize < sizeof(t))
                return false;
        }
        memcpy(&t, m_buffer.data() + m_offset, sizeof(T));
        m_offset += sizeof(T);
        return true;
    }

  private:
    void init(const std::string &file, std::size_t pos) {
        m_fileStream.open(file, std::ios::binary | std::ios::in);
        m_offset = pos;
    }

    bool isEnd() const { return m_offset == m_validSize; }

    bool fillBuffer() {
        if (m_fileStream.eof()) {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("End of file reached unexpectedly.\n"));
            return false;
        } else if (m_fileStream.fail()) {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("Logical error on i/o operation.\n"));
            return false;
        } else if (m_fileStream.bad()) {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("Read operation failed due to severe stream error.\n"));
            return false;
        }
        std::size_t restedSize{m_validSize - m_offset};
        if (m_offset != 0) {
            memmove(m_buffer.data(), m_buffer.data() + m_offset, restedSize);
        }
        m_fileStream.read(m_buffer.data() + restedSize, m_buffer.size() - restedSize);
        std::size_t byteRead = m_fileStream.gcount();
        m_validSize = restedSize + byteRead;
        m_offset = 0;
        return byteRead > 0;
    }
};

class TA_BufferWriter : public TA_BasicBufferOperator<TA_BufferWriter> {
  public:
    TA_BufferWriter() = delete;
    TA_BufferWriter(const std::string &file, std::size_t size) : TA_BasicBufferOperator<TA_BufferWriter>(file, size) {
        init(file);
    }

    // Explicit finish() reports errors; destruction is best-effort and never throws.
    ~TA_BufferWriter() noexcept { finish(); }

    TA_BufferWriter(const TA_BufferWriter &writer) = delete;
    TA_BufferWriter(TA_BufferWriter &&writer) = delete;

    TA_BufferWriter &operator=(const TA_BufferWriter &writer) = delete;
    TA_BufferWriter &operator=(TA_BufferWriter &&writer) = delete;

    template <EndianConvertedType T> bool write(T &t) {
        if (!isValid() || !m_fileStream.good())
            return false;
        // A single value must fit even when the requested buffer size is zero.
        if (sizeof(t) > m_buffer.size())
            m_buffer.resize(sizeof(t));
        if (sizeof(t) > m_buffer.size() - m_offset) {
            if (!flush())
                return false;
        }
        memcpy(m_buffer.data() + m_offset, &t, sizeof(std::remove_cvref_t<T>));
        m_offset += sizeof(std::remove_cvref_t<T>);
        m_validSize += sizeof(std::remove_cvref_t<T>);
        return true;
    }

    bool flush() {
        if (!isValid() || !m_fileStream.good())
            return false;
        if (m_validSize != 0)
            m_fileStream.write(m_buffer.data(), static_cast<std::streamsize>(m_validSize));
        if (!m_fileStream.good())
            return false;
        m_offset = 0;
        m_validSize = 0;
        m_fileStream.flush();
        return m_fileStream.good();
    }

    bool finish() {
        if (m_finished)
            return m_finishSucceeded;
        const bool flushed = flush();
        if (m_fileStream.is_open())
            m_fileStream.close();
        m_finished = true;
        m_finishSucceeded = flushed && !m_fileStream.fail();
        return m_finishSucceeded;
    }

  private:
    bool m_finished{false};
    bool m_finishSucceeded{false};
    void init(const std::string &file) { m_fileStream.open(file, std::ios::binary | std::ios::out); }
};

template <typename T>
concept BufferOperatorType = requires() { typename T::OperatorType; };

using BufferReader = TA_BasicBufferOperator<TA_BufferReader>;
using BufferWriter = TA_BasicBufferOperator<TA_BufferWriter>;

} // namespace CoreAsync

#endif // TA_BUFFER_H
