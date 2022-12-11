#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return isn + static_cast<uint32_t>(n);
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t offset = static_cast<uint64_t>(n.raw_value() - isn.raw_value());
    uint64_t scale = checkpoint >> 32;
    uint64_t base = 1ul << 32;
    uint64_t offset_1 = offset + scale * base;
    // 若scale为0，则不需要考虑减一的情况
    uint64_t offset_2 = scale !=0 ? offset + (scale - 1) * base : offset_1; 
    uint64_t offset_3 = offset + (scale + 1) * base;
    uint64_t abs_1 = offset_1 > checkpoint ? offset_1 - checkpoint : checkpoint - offset_1;
    uint64_t abs_2 = offset_2 > checkpoint ? offset_2 - checkpoint : checkpoint - offset_2;
    uint64_t abs_3 = offset_3 > checkpoint ? offset_3 - checkpoint : checkpoint - offset_3;

    uint64_t min_abs = min(min(abs_1, abs_2), abs_3);
    if(min_abs == abs_1) 
        offset = offset_1;
    else if(min_abs == abs_2)    
        offset = offset_2;
    else if(min_abs == abs_3)    
        offset = offset_3;
    return offset;
}
