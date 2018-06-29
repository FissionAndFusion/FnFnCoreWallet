// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_DATASTREAM_H
#define  WALLEVE_DATASTREAM_H

#include <exception>
#include <vector>
#include <boost/type_traits.hpp>

namespace walleve
{

class CWalleveODataStream
{
#define BEGIN(a)            ((unsigned char*)&(a))
#define END(a)              ((unsigned char*)&((&(a))[1]))
public:
    CWalleveODataStream(std::vector<unsigned char>& vchIn) : vch(vchIn) {}
    void Push(const void* p,std::size_t size)
    {
        vch.insert(vch.end(),(const unsigned char*)p,(const unsigned char*)p + size);
    }
     
    template <typename T>
    void Push(const T& data,const boost::true_type&)
    {
        vch.insert(vch.end(),BEGIN(data),END(data));
    }
    template <typename T>
    void Push(const T& data,const boost::false_type&)
    {
        data.ToDataStream(*this);
    }
    
    template <typename T>
    CWalleveODataStream& operator<< (const T& data)
    {
        Push(data,boost::is_fundamental<T>());
        return (*this);
    }
    template<typename T, typename A>
    CWalleveODataStream& operator<< (const std::vector<T, A>& data)
    {
        unsigned int size = data.size();
        vch.insert(vch.end(),BEGIN(size),END(size));
        if (size > 0)
        {
            if (boost::is_fundamental<T>::value)
            {
                vch.insert(vch.end(),BEGIN(data[0]),END(data[size - 1]));
            }
            else
            {
                for (std::size_t i = 0;i < data.size();i++)
                {
                    (*this) << data[i];
                }
            }
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
    void Pop(void* p,std::size_t size)
    {
        if (nPosition + size > vch.size())
        {
            throw std::range_error("out of range");
        }
        std::memmove(p,&vch[nPosition],size);
        nPosition += size;
    }
    template <typename T>
    void Pop(T& data,const boost::true_type&)
    {
        if (nPosition + sizeof(T) > vch.size())
        {
            throw std::range_error("out of range");
        }
        data = *((T*)&vch[nPosition]);
        nPosition += sizeof(T);
    }
    template <typename T>
    void Pop(T& data,const boost::false_type&)
    {
        data.FromDataStream(*this);
    }
    template <typename T>
    CWalleveIDataStream& operator>> (T& data)
    {
        Pop(data,boost::is_fundamental<T>());
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
        if (boost::is_fundamental<T>::value)
        {
            data.assign((T*)&vch[nPosition],(T*)&vch[nPosition] + size);
            nPosition += size * sizeof(T); 
        }
        else
        {
            data.resize(size);
            for (unsigned int i = 0;i < size;i++)
            {
                *this >> data[i];
            }
        }
        return (*this);
    }
protected:
    const std::vector<unsigned char>& vch;
    std::size_t nPosition;
};

}

#endif //WALLEVE_DATASTREAM_H

