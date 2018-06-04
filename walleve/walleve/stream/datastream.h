// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_DATASTREAM_H
#define  WALLEVE_DATASTREAM_H

#include <exception>
#include <vector>

namespace walleve
{

class CWalleveODataStream
{
#define BEGIN(a)            ((unsigned char*)&(a))
#define END(a)              ((unsigned char*)&((&(a))[1]))
public:
    CWalleveODataStream(std::vector<unsigned char>& vchIn) : vch(vchIn) {}
    template <typename T>
    CWalleveODataStream& operator<< (const T& data)
    {
        vch.insert(vch.end(),BEGIN(data),END(data));
        return (*this);
    }
    template<typename T, typename A>
    CWalleveODataStream& operator<< (const std::vector<T, A>& data)
    {
        unsigned int size = data.size();
        vch.insert(vch.end(),BEGIN(size),END(size));
        if (size > 0)
        {
            vch.insert(vch.end(),BEGIN(data[0]),END(data[size - 1]));
        }
        return (*this);
    }
protected:
    std::vector<unsigned char>& vch;
    std::size_t nPosition;
};

class CWalleveIDataStream
{
public:
    CWalleveIDataStream(const std::vector<unsigned char>& vchIn) : vch(vchIn),nPosition(0) {}
    std::size_t GetSize()
    {
        return (vch.size() - nPosition);
    }
    template <typename T>
    CWalleveIDataStream& operator>> (T& data)
    {
        if (nPosition + sizeof(T) > vch.size())
        {
            throw std::range_error("out of range");
        }
        data = *((T*)&vch[nPosition]);
        nPosition += sizeof(T);
        return (*this);
    }
    template<typename T, typename A>
    CWalleveIDataStream& operator>> (std::vector<T, A>& data)
    {
        unsigned int size;
        *this >> size;
        if (nPosition + size * sizeof(T) > vch.size())
        {
            throw std::range_error("out of range");
        }
        for (unsigned int i = 0;i < size;i++)
        {
            data.push_back(*(T*)&vch[nPosition]);
            nPosition += sizeof(T);
        }
        return (*this);
    }
protected:
    const std::vector<unsigned char>& vch;
    std::size_t nPosition;
};

}

#endif //WALLEVE_DATASTREAM_H

