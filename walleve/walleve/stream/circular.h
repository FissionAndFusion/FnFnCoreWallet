// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_CIRCULAR_H
#define  WALLEVE_CIRCULAR_H

#include <cctype>
#include <streambuf>
#include <vector>

namespace walleve
{

class circularbuf : public std::streambuf
{
public:
    explicit circularbuf(std::size_t maximum_size);

    std::size_t size() const;
    std::size_t freespace() const;
    pos_type putpos() const;
    void consume(std::size_t n);
protected:
    enum {MIN_SIZE=8,};
    int_type underflow();
    int_type overflow(int_type c);
    pos_type seekoff(off_type, std::ios_base::seekdir,
                     std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
    pos_type seekpos(pos_type,
                     std::ios_base::openmode = std::ios_base::in | std::ios_base::out);
    std::streamsize showmanyc();
private:

    std::size_t buf_size_;
    std::size_t size_mask_;
    std::size_t begin_;
    std::vector<char> buffer_;
};

} // namespace walleve

#endif //WALLEVE_CIRCULAR_H
