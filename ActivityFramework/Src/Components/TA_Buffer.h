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


#ifndef TA_BUFFER_H
#define TA_BUFFER_H

#include <sstream>
#include <fstream>
#include <vector>

#include "TA_EndianConversion.h"
#include "TA_CommonTools.h"
#include "TA_MetaStringView.h"

namespace CoreAsync {

class TA_BufferReader;
class TA_BufferWriter;

template <typename Opt>
class TA_BasicBufferOperator
{
public:
    using OperatorType = Opt;

    TA_BasicBufferOperator() = delete;
    TA_BasicBufferOperator(const std::string &file, std::size_t size) : m_buffer(size)
    {

    }

    virtual ~TA_BasicBufferOperator()
    {
        close();
    }

    TA_BasicBufferOperator(const TA_BasicBufferOperator &op) = delete;
    TA_BasicBufferOperator(TA_BasicBufferOperator &&op) = delete;

    bool isValid() const
    {
        return m_fileStream.is_open();
    }

    void close()
    {
        if(m_fileStream.is_open())
            m_fileStream.close();
    }

    template <typename T>
    bool read(T &t)
    {
        static_assert(std::is_same_v<Opt, TA_BufferReader>, "Read is not the member of current type");
        return static_cast<Opt *>(this)->read(t);
    }

    template <typename T>
    bool write(T &t)
    {
        static_assert(std::is_same_v<Opt, TA_BufferWriter>, "Write is not the member of current type");
        return static_cast<Opt *>(this)->write(t);
    }

    void flush()
    {
        static_assert(std::is_same_v<Opt, TA_BufferWriter>, "Flush is not the member of current type");
        return static_cast<Opt *>(this)->flush();
    }

protected:
    std::fstream m_fileStream;
    std::vector<char> m_buffer;

};


class TA_BufferReader : public TA_BasicBufferOperator<TA_BufferReader>
{
public:
    TA_BufferReader() = delete;
    TA_BufferReader(const std::string &file, std::size_t size) : CoreAsync::TA_BasicBufferOperator<TA_BufferReader>(file, size)
    {
        init(file);
    }

    ~TA_BufferReader()
    {
        m_iStrStream.str("");
        m_iStrStream.clear();
    }

    TA_BufferReader(const TA_BufferReader &writer) = delete;
    TA_BufferReader(TA_BufferReader &&writer) = delete;

    template <EndianConvertedType T>
    bool read(T &t)
    {
        if(m_iStrStream.str().empty() || m_validSize - m_iStrStream.tellg() < sizeof(t))
        {
            if(!fillBuffer() || m_validSize < sizeof(t))
                return false;
        }
        m_iStrStream.read(reinterpret_cast<char*>(&t), sizeof(t));
        return true;
    }

private:
    void init(const std::string &file)
    {
        m_fileStream.open(file, std::ios::binary | std::ios::in);
    }

    bool fillBuffer()
    {
        if (m_fileStream.eof())
        {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("End of file reached unexpectedly.\n"));
            return false;
        }
        else if (m_fileStream.fail())
        {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("Logical error on i/o operation.\n"));
            return false;
        }
        else if (m_fileStream.bad())
        {
            CoreAsync::TA_CommonTools::debugInfo(META_STRING("Read operation failed due to severe stream error.\n"));
            return false;
        }
        std::size_t bufferPos = m_iStrStream.tellg();
        std::size_t occupiedPos {m_validSize - bufferPos};
        if (bufferPos != 0)
        {
            memmove(m_buffer.data(), m_buffer.data() + bufferPos, occupiedPos);
        }
        m_fileStream.read(m_buffer.data() + occupiedPos, m_buffer.size() - occupiedPos);
        std::size_t byteRead = m_fileStream.gcount();
        m_validSize = occupiedPos + byteRead;

        m_iStrStream.str({m_buffer.data(), m_validSize});
        m_iStrStream.clear();

        return byteRead > 0;
    }

private:
    std::istringstream m_iStrStream {};
    std::size_t m_validSize {0};

};

class TA_BufferWriter : public TA_BasicBufferOperator<TA_BufferWriter>
{
public:
    TA_BufferWriter() = delete;
    TA_BufferWriter(const std::string &file, std::size_t size) : TA_BasicBufferOperator<TA_BufferWriter>(file, size)
    {
        init(file);
    }

    ~TA_BufferWriter()
    {
        flush();
    }

    TA_BufferWriter(const TA_BufferWriter &writer) = delete;
    TA_BufferWriter(TA_BufferWriter &&writer) = delete;

    TA_BufferWriter & operator = (const TA_BufferWriter &writer) = delete;
    TA_BufferWriter & operator = (TA_BufferWriter &&writer) = delete;

    template <EndianConvertedType T>
    bool write(T &t)
    {
        if(!isValid())
            return false;
        m_oStrStream.write(reinterpret_cast<const char *>(&t), sizeof(t));
        if(m_oStrStream.tellp() + static_cast<std::streampos>(sizeof(t)) > static_cast<std::streampos>(m_buffer.size()))
        {
            flush();
        }
        return true;
    }

    void flush()
    {
        m_fileStream.write(m_oStrStream.str().data(), m_oStrStream.tellp());
        m_oStrStream.str("");
        m_oStrStream.clear();
    }

private:
    void init(const std::string &file)
    {  
        m_oStrStream.rdbuf()->pubsetbuf(m_buffer.data(), m_buffer.size());
        m_fileStream.open(file, std::ios::binary | std::ios::out);
    }

private:
    std::ostringstream m_oStrStream {};

};

template <typename T>
concept BufferOperatorType = requires()
{
    typename T::OperatorType;
};

using BufferReader = TA_BasicBufferOperator<TA_BufferReader>;
using BufferWriter = TA_BasicBufferOperator<TA_BufferWriter>;

}


#endif // TA_BUFFER_H
