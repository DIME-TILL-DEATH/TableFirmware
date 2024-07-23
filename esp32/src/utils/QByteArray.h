#ifndef QBYTEARRAY_H
#define QBYTEARRAY_H

#include <stdint.h>
#include <vector>

class QByteArray : public std::vector<uint8_t>
{
public:
    void append(const char* data_ptr, uint32_t dataSize);
    void append(const QByteArray& data);

    QByteArray right(uint32_t dataSize);
    QByteArray mid(uint32_t position, uint32_t dataSize);
};

#endif