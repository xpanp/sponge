#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    // 忽略收到SYN之前的数据
    if (!_syn_received && !header.syn) {
        return;
    }
    // 忽略重复的SYN
    if (_syn_received && header.syn) {
        return;
    }

    string payload = seg.payload().copy();
    WrappingInt32 seqno = header.seqno;
    // 收到 SYN
    if (header.syn) {
        _syn_received = true;
        _isn = header.seqno;
        seqno = header.seqno + 1;
    }
    // 收到 FIN
    if (header.fin) {
        _fin_received = true;
    }

    uint64_t checkpoint = _reassembler.stream_out().bytes_written();
    uint64_t absolute_seqno = unwrap(seqno, _isn, checkpoint);
    // stream_index = absolute_seqno - 1
    _reassembler.push_substring(payload, absolute_seqno - 1, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received) {
        return nullopt;
    }
    uint64_t absolute_seqno = stream_out().bytes_written() + 1;
    stream_out().input_ended() ? ++absolute_seqno : absolute_seqno;
    return wrap(absolute_seqno, _isn);
}

size_t TCPReceiver::window_size() const {
    return stream_out().remaining_capacity();
}
