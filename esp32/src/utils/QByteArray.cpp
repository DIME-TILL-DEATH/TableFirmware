#include "QByteArray.h"

void QByteArray::append(const char* data_ptr, uint32_t dataSize)
{
    for(int i=0; i<dataSize; i++)
    {
        push_back(data_ptr[i]);
    }
}

void QByteArray::append(const QByteArray& newArray)
{
//    append(newArray.data(), newArray.size());
    for(auto it = newArray.begin(); it != newArray.end(); ++it)
    {
        push_back(*it);
    }
}

QByteArray QByteArray::right(uint32_t dataSize)
{
    return QByteArray(std::vector(end() - dataSize, end()));
}

QByteArray QByteArray::mid(uint32_t position, uint32_t dataSize)
{
    return QByteArray(std::vector(begin() + position, begin() + position + dataSize));    
}