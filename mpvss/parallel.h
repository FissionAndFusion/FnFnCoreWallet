// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PARALLEL_H
#define PARALLEL_H

#include <algorithm>
#include <atomic>
#include <exception>
#include <future>
#include <iterator>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

/**
 * Parallel computer for CPU intensive computing
 */
class ParallelComputer
{
public:
    ParallelComputer(uint8_t nNum = std::thread::hardware_concurrency())
      : nParallelNum(nNum)
    {
        if (nParallelNum == 0)
        {
            nParallelNum = std::thread::hardware_concurrency();
        }
    }

    /**
     * transform [nTotal] data from [fInput] to [fOutput] by [fTrans].
     * All transform success return true, or false.
     * nTotal, number of data.
     * fInput, function "T (index)" or "tuple<T1, T2...> (index)" fetch original data "T" or "T1,T2..." of "index".
     * fOutput, function "void (index, T)" save transformed data "T" of "index".
     * fTrans, function "T (U)" or "T (U1, U2...)" transformed data from "U" or "U1,U2..." to "T".
     * 
     * NOTICE: All data about fInput, fOutput, fTrans will be used by multi-threads, confirm they are visited(heap, global)
     * NOTICE: Impletation is without mutex, so suggest to use [vector] instead of [map]
     */
    template<typename InputFunc, typename OutputFunc, typename TransFunc>
    bool Transform(const uint32_t nTotal, InputFunc fInput, OutputFunc fOutput, TransFunc fTrans)
    {
        std::atomic_size_t nCurrent;
        nCurrent.store(0);

        uint8_t nThreads = std::min(nTotal, (uint32_t)nParallelNum);
        std::vector<std::future<bool> > vFuture(nThreads);

        for (int i = 0; i < nThreads; ++i)
        {
            vFuture[i] = async(std::launch::async, [&] {
                try
                {
                    size_t nIndex;
                    while ((nIndex = nCurrent.fetch_add(1)) < nTotal)
                    {
                        auto params = fInput(nIndex);
                        auto result = CallFunction(fTrans, params, IsTuple<typename std::decay<decltype(params)>::type>());
                        fOutput(nIndex, result);
                    }
                    return true;
                }
                catch (std::exception& e)
                {
                    return false;
                }
            });
        }

        bool ret = true;
        for_each(vFuture.begin(), vFuture.end(), [&ret] (std::future<bool>& f) { ret &= f.get(); });
        return ret;
    }

    /**
     * transform [nTotal] data from [fInput] without output
     * All transform success return true, or false.
     * nTotal, number of data.
     * fInput, function "T (index)" or "tuple<T1, T2...> (index)" fetch original data "T" or "T1,T2..." of "index".
     * fTrans, function "T (U)" or "T (U1, U2...)" transformed data from "U" or "U1,U2..." to "T".
     * 
     * NOTICE: All data about fInput, fTrans will be used by multi-threads, confirm they are visited(heap, global)
     * NOTICE: Impletation is without mutex, so suggest to use [vector] instead of [map]
     */
    template<typename InputFunc, typename TransFunc>
    bool Transform(const uint32_t nTotal, InputFunc fInput, TransFunc fTrans)
    {
        std::atomic_size_t nCurrent;
        nCurrent.store(0);

        uint8_t nThreads = std::min(nTotal, (uint32_t)nParallelNum);
        std::vector<std::future<bool> > vFuture(nThreads);
        for (int i = 0; i < nThreads; ++i)
        {
            vFuture[i] = async(std::launch::async, [&] {
                try
                {
                    size_t nIndex;
                    while ((nIndex = nCurrent.fetch_add(1)) < nTotal)
                    {
                        auto params = fInput(nIndex);
                        ExecuteFunction(fTrans, params, IsTuple<typename std::decay<decltype(params)>::type>());
                    }
                    return true;
                }
                catch (std::exception& e)
                {
                    return false;
                }
            });
        }

        bool ret = true;
        for_each(vFuture.begin(), vFuture.end(), [&ret] (std::future<bool>& f) { ret &= f.get(); });
        return ret;
    }

    /**
     * transform [iBegin, iEnd) data to [oBegin, ) by [fTrans].
     * All transform success return true, or false.
     * iBegin, begin iterator of original data.
     * iEnd, end iterator of original data.
     * oBegin, begin iterator of transformed data. Confirm reserved enough space from [oBegin], at least diff(iBegin, iEnd)
     * fTrans, function "T (U)" or "T (U1, U2...)" transformed data from "U" or "U1,U2..." to "T".
     * 
     * NOTICE: Impletation is without mutex, so suggest to use [random iterator] instead of [forward iterator].
     */
    template<typename InputIterator, typename OutputIterator, typename TransFunc>
    bool Transform(InputIterator iBegin, InputIterator iEnd, OutputIterator oBegin, TransFunc fTrans)
    {
        uint32_t nTotal = IteratorDifferece(iBegin, iEnd, typename std::iterator_traits<InputIterator>::iterator_category());

        std::atomic_size_t nCurrent;
        nCurrent.store(0);

        uint8_t nThreads = std::min(nTotal, (uint32_t)nParallelNum);
        std::vector<std::future<bool> > vFuture(nThreads);

        for (int i = 0; i < nThreads; ++i)
        {
            vFuture[i] = async(std::launch::async, [&] {
                try
                {
                    size_t nIndex;
                    while ((nIndex = nCurrent.fetch_add(1)) < nTotal)
                    {
                        auto& params = *IteratorIncrease(iBegin, nIndex, typename std::iterator_traits<InputIterator>::iterator_category());
                        auto result = CallFunction(fTrans, params, IsTuple<typename std::decay<decltype(params)>::type>());
                        *IteratorIncrease(oBegin, nIndex, typename std::iterator_traits<OutputIterator>::iterator_category()) = result;
                    }
                    return true;
                }
                catch (std::exception& e)
                {
                    return false;
                }
            });
        }

        bool ret = true;
        for_each(vFuture.begin(), vFuture.end(), [&ret] (std::future<bool>& f) { ret &= f.get(); });
        return ret;
    }

    /**
     * transform [iBegin, iEnd) data by [fTrans].
     * All transform success return true, or false.
     * iBegin, begin iterator of original data.
     * iEnd, end iterator of original data.
     * fTrans, function "T (U)" or "T (U1, U2...)" transformed data from "U" or "U1,U2..." to "T".
     * 
     * NOTICE: Impletation is without mutex, so suggest to use [random iterator] instead of [forward iterator].
     */
    template<typename InputIterator, typename TransFunc>
    bool Transform(InputIterator iBegin, InputIterator iEnd, TransFunc fTrans)
    {
        uint32_t nTotal = IteratorDifferece(iBegin, iEnd, typename std::iterator_traits<InputIterator>::iterator_category());

        std::atomic_size_t nCurrent;
        nCurrent.store(0);

        uint8_t nThreads = std::min(nTotal, (uint32_t)nParallelNum);
        std::vector<std::future<bool> > vFuture(nThreads);

        for (int i = 0; i < nThreads; ++i)
        {
            vFuture[i] = async(std::launch::async, [&] {
                try
                {
                    size_t nIndex;
                    while ((nIndex = nCurrent.fetch_add(1)) < nTotal)
                    {
                        auto params = *IteratorIncrease(iBegin, nIndex, typename std::iterator_traits<InputIterator>::iterator_category());
                        ExecuteFunction(fTrans, params, IsTuple<typename std::decay<decltype(params)>::type>());
                    }
                    return true;
                }
                catch (std::exception& e)
                {
                    return false;
                }
            });
        }

        bool ret = true;
        for_each(vFuture.begin(), vFuture.end(), [&ret] (std::future<bool>& f) { ret &= f.get(); });
        return ret;
    }
protected:
    uint8_t nParallelNum;

protected:
    struct NoTuple
    {
    };

    template<int...>
    struct IndexTuple
    {
    };

    template<int N, int... S>
    struct MakeIndex : MakeIndex<N - 1, N - 1, S...>
    {
    };

    template<int... S>
    struct MakeIndex<0, S...>
    {
        typedef IndexTuple<S...> type;
    };

    template<typename T>
    struct IsTuple : NoTuple
    {
    };

    template<typename... T>
    struct IsTuple<std::tuple<T...>> : MakeIndex<sizeof...(T)>::type
    {
    };

    template<typename Fun, typename Tuple, int... S>
    typename std::result_of<Fun(typename std::tuple_element<S, Tuple>::type...)>::type
    CallFunction(Fun f, Tuple& t, IndexTuple<S...>)
    {
        return f(std::get<S>(t)...);
    }

    template<typename Fun, typename Param>
    typename std::result_of<Fun(Param)>::type
    CallFunction(Fun f, Param& p, NoTuple)
    {
        return f(p);
    }

    template<typename Fun, typename Tuple, int... S>
    void ExecuteFunction(Fun f, Tuple& t, IndexTuple<S...>)
    {
        f(std::get<S>(t)...);
    }

    template<typename Fun, typename Param>
    void ExecuteFunction(Fun f, Param& p, NoTuple)
    {
        f(p);
    }

    template<typename Iterator>
    std::ptrdiff_t IteratorDifferece(Iterator begin, Iterator end, std::random_access_iterator_tag)
    {
        return (end - begin);
    }

    template<typename Iterator>
    std::ptrdiff_t IteratorDifferece(Iterator begin, Iterator end, std::forward_iterator_tag)
    {
        std::ptrdiff_t count = 0;
        for (Iterator it = begin; it != end; it++, count++);
        return count;
    }

    template<typename Iterator>
    Iterator IteratorIncrease(Iterator begin, std::ptrdiff_t diff, std::random_access_iterator_tag)
    {
        return begin + diff;
    }

    template<typename Iterator>
    Iterator IteratorIncrease(Iterator begin, std::ptrdiff_t diff, std::forward_iterator_tag)
    {
        Iterator it = begin;
        for (std::ptrdiff_t i = 0; i < diff; i++, it++);
        return it;
    }
};

// InputFunc for vector.
// usage: std::bind(std::cref(vector), std::placeholder::_1)
template<typename T>
T LoadVectorData(const std::vector<T>& v, const int i)
{
    return v[i];
}

// OutputFunc for vector.
// usage: std::bind(std::ref(vector), std::placeholder::_1, std::placeholder::_2)
template<typename T>
void StoreVectorData(std::vector<T>& v, const int i, const T& t)
{
    v[i] = t;
}

#endif // PARALLEL_H