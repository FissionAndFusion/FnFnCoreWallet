// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_CACHE_H
#define  WALLEVE_CACHE_H

#include "walleve/rwlock.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

#include <boost/thread/thread.hpp>

namespace walleve
{

template <typename K,typename V>
class CWalleveCache
{
    class CKeyValue
    {
    public:
        K key;
        mutable V value;
    public:
        CKeyValue() {}
        CKeyValue(const K& keyIn,const V& valueIn) : key(keyIn), value(valueIn) {}
    };
    typedef boost::multi_index_container<
      CKeyValue,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<boost::multi_index::member<CKeyValue,K,&CKeyValue::key> >,
        boost::multi_index::sequenced<>
      >
    > CKeyValueContainer;
    typedef typename CKeyValueContainer::template nth_index<1>::type CKeyValueList;
public:
    CWalleveCache(std::size_t nMaxCountIn = 0) : nMaxCount(nMaxCountIn) {}
    bool Exists(const K& key) const
    {
        CWalleveReadLock rlock(rwAccess);
        return (!!cntrCache.count(key));
    }
    bool Retrieve(const K& key,V& value) const
    {
        CWalleveReadLock rlock(rwAccess);
        typename CKeyValueContainer::const_iterator it = cntrCache.find(key);
        if (it != cntrCache.end())
        {
            value = (*it).value;
            return true;
        }
        return false;
    }
    void AddNew(const K& key,const V& value)
    {
        CWalleveWriteLock wlock(rwAccess);
        std::pair<typename CKeyValueContainer::iterator,bool> ret = cntrCache.insert(CKeyValue(key,value));
        if (!ret.second)
        {
            (*(ret.first)).value = value;
        }
        if (nMaxCount != 0 && cntrCache.size() > nMaxCount)
        {
            CKeyValueList& listCache = cntrCache.template get<1>();
            listCache.pop_front();
        }
    }
    void Remove(const K& key)
    {
        CWalleveWriteLock wlock(rwAccess);
        cntrCache.erase(key);
    }
    void Clear()
    {
        CWalleveWriteLock wlock(rwAccess);
        cntrCache.clear(); 
    }
protected:
    mutable CWalleveRWAccess rwAccess;
    CKeyValueContainer cntrCache;
    std::size_t nMaxCount;
};

} // namespace walleve

#endif //WALLEVE_CACHE_H

