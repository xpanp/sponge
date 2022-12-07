#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) :  
    _cap(capacity), _write_cnt(0), _read_cnt(0), _write_ptr(0), 
    _read_ptr(0), _used(0), _input_end(false), _data(capacity, 0) {}

size_t ByteStream::write(const string &data) {
    if (remaining_capacity() == 0 || data.size() == 0) {
        return 0;
    }

    size_t size = min(data.size(), remaining_capacity());
    size_t start = _write_ptr;
    size_t end = (start + size) % _cap; // 写入的终点
    
    if (end > start) {
        _data.replace(_data.begin() + start, _data.begin() + end, data.begin(), data.begin() + size);
    } else {
        _data.replace(_data.begin() + start, _data.begin() + _cap, data.begin(), data.begin() + (_cap - start));
        _data.replace(_data.begin(), _data.begin() + end, data.begin() + (_cap - start), data.begin() + size);
    }
    _used += size;
    _write_cnt += size;
    _write_ptr = end;

    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t size = min(len, _used);

    string ret;
    ret.reserve(size);
    if (_read_ptr + size <= _cap) {
        ret.append(_data.begin() + _read_ptr, _data.begin() + _read_ptr + size);
    } else {
        ret.append(_data.begin() + _read_ptr, _data.begin() + _cap);
        ret.append(_data.begin(), _data.begin()+(size - (_cap - _read_ptr)));
    }

    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t size =  min(len, _used);
    _used -= size;
    _read_ptr = (_read_ptr + size) % _cap;
    _read_cnt += size;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret = peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() { _input_end = true; }

bool ByteStream::input_ended() const { return _input_end; }

size_t ByteStream::buffer_size() const { return _used; }

bool ByteStream::buffer_empty() const { return _used == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _write_cnt; }

size_t ByteStream::bytes_read() const { return _read_cnt; }

size_t ByteStream::remaining_capacity() const { return _cap - _used; }
