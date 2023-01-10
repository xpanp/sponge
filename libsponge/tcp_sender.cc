#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity), _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const {
    return _next_seqno - _ackno;
}

void TCPSender::fill_window() {
    // 发送窗口减去未被确认的数据大小，为可发送的payload
    uint64_t payload_size =
        _rwnd > (_next_seqno - _ackno) ? (_rwnd - (_next_seqno - _ackno)) : 0;
    // 发送窗口为0，但是还有未确认的数据
    if (_rwnd == 0 && _track.size() != 0) {
        return;
    }
    payload_size = _rwnd == 0 ? 1 : payload_size;
    while (payload_size > 0) {
        TCPSegment segment{};
        TCPHeader &header = segment.header();
        // 如果连接为建立，首先发送SYN
        if (_next_seqno == 0) {
            header.syn = true;
            payload_size--;
        }
        Buffer buf{_stream.read(payload_size > TCPConfig::MAX_PAYLOAD_SIZE
                                    ? TCPConfig::MAX_PAYLOAD_SIZE
                                    : payload_size)};
        segment.payload() = buf;
        payload_size -= buf.size();
        // 如果发送数据全部eof, 发送FIN
        if (_stream.eof() && (!_FIN_send) && payload_size > 0) {
            header.fin = true;
            _FIN_send = true;
            payload_size--;
        }
        if (segment.length_in_sequence_space() != 0) {
            header.seqno = wrap(_next_seqno, _isn);
            _next_seqno += segment.length_in_sequence_space();
            // Push it in _segments_out
            _segments_out.push(segment);
            // Push it in _track
            _track.push(segment);
            // update_checkpoint
            _checkpoint += segment.length_in_sequence_space();
            // Active timer
            _timer.active();
        }
        if (segment.length_in_sequence_space() == 0) {
            break;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_no = unwrap(ackno, _isn, _checkpoint);
    // 忽略掉错误的ack
    if (abs_no > _next_seqno || abs_no < _ackno) {
        return;
    }
    // 收到新的ack以后刷新定时器
    if (abs_no > _ackno) {
        _timer.reset(_initial_retransmission_timeout);
    }
    // 更新拥塞控制窗口大小
    _rwnd = window_size;
    _ackno = abs_no;
    // pop掉已经被确认的数据
    for (; _track.size() > 0;) {
        auto old_segment = _track.front();
        if (unwrap(old_segment.header().seqno, _isn, _checkpoint) +
                old_segment.length_in_sequence_space() <= abs_no) {
            _track.pop();
            continue;
        }
        break;
    }
    if (_track.size() == 0) {
        // 取消定时器
        _timer.deactive();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // 可发送窗口不为0, 则需要指数回退
    if (_timer.tick(ms_since_last_tick, _rwnd != 0)) {
        // 没有可重发的数据，则重置定时器
        if (_track.size() == 0) {
            _timer.deactive();
            _timer.reset(_initial_retransmission_timeout);
            return;
        }
        // retransmit first segment in _track
        auto segment = _track.front();
        _segments_out.push(segment);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _timer.get_consecutive_retransmissions();
}

void TCPSender::send_empty_segment() {
    TCPSegment segment{};
    TCPHeader &header = segment.header();
    header.seqno = wrap(_next_seqno, _isn);
    _segments_out.push(segment);
}
