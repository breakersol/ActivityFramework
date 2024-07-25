#include "TA_Buffer.h"

namespace CoreAsync {

TA_BufferReader::TA_BufferReader(const std::string &file, std::size_t size) : CoreAsync::TA_BasicBufferOperator<TA_BufferReader>(file, size)
{

}

TA_BufferReader::~TA_BufferReader()
{
    m_iStrStream.str("");
    m_iStrStream.clear();
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

TA_BufferWriter::TA_BufferWriter(const std::string &file, std::size_t size) : TA_BasicBufferOperator<TA_BufferWriter>(file, size)
{
    init(file);
}

TA_BufferWriter::~TA_BufferWriter()
{
    flush();
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
