#include "TA_Buffer.h"

namespace CoreAsync {

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
