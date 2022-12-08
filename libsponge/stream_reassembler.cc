#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _buffer(capacity, 0), _flag(capacity, false), 
    _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t first_unassembled = _output.bytes_written();
    size_t first_unaccept = _output.bytes_read() + _capacity;
    // 超出可写入范围
    if(index >= first_unaccept || 
        index + data.length() < first_unassembled){
        return ;
    }
    
    // 截取字符串开始和结尾的index
    size_t begin_index = index;
    size_t end_index = index + data.length();

    if (begin_index < first_unassembled) {
        begin_index = first_unassembled;
    }
    if (end_index >= first_unaccept){
        end_index = first_unaccept;
    }

    // 数据加入unassembled部分
    for(size_t i = begin_index; i < end_index; i++){
        if(!_flag[i - first_unassembled]) {
            _buffer[i - first_unassembled] = data[i - index];
            _unas_bytes++;
            _flag[i - first_unassembled] = true;
        }
    }

    // 将assembled的数据写入ByteStream
    // 不需要担心可写入的数据超过ByteStream
    string str = "";
    while(_flag.front()) {
        str += _buffer.front();
        _buffer.pop_front();
        _flag.pop_front();
        // 为了保持队列容量不变需要在后面添加空元素占位
        _buffer.emplace_back('\0');
        _flag.emplace_back(false);
    }

    if(str.length() > 0) {
        stream_out().write(str);
        _unas_bytes -= str.length();
    }

    if(eof) {
        _is_eof = true;
        _eof_index = index + data.length();
    }
    
    if(_is_eof && _eof_index == _output.bytes_written()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unas_bytes; }

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }
