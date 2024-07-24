#include "TA_Buffer.h"

namespace CoreAsync {

TA_BufferReader::TA_BufferReader(const std::string &file, std::size_t size)
    : m_buffer(size)
{
    init(file);
}

TA_BufferReader::~TA_BufferReader()
{
    if (m_fileStream.is_open())
        m_fileStream.close();
    m_iStrStream.str("");
    m_iStrStream.clear();
}

bool TA_BufferReader::isValid() const
{
    return m_fileStream.is_open();
}

void TA_BufferReader::init(const std::string &file)
{
    m_fileStream.open(file, std::ios::binary | std::ios::in);
}

bool TA_BufferReader::fillBuffer()
{
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

TA_BufferWriter::TA_BufferWriter(const std::string &file, std::size_t size) : m_buffer(size)
{
    init(file);
}

TA_BufferWriter::~TA_BufferWriter()
{
    flush();
    if(m_fileStream.is_open())
        m_fileStream.close();
}

bool TA_BufferWriter::isValid() const
{
    return m_fileStream.is_open();
}

void TA_BufferWriter::init(const std::string &file)
{
    m_fileStream.open(file, std::ios::binary | std::ios::out);
    m_oStrStream.rdbuf()->pubsetbuf(m_buffer.data(), m_buffer.size());
}

void TA_BufferWriter::flush()
{
    m_fileStream.write(m_oStrStream.str().data(), m_oStrStream.tellp());
    m_oStrStream.str("");
    m_oStrStream.clear();
}

}
