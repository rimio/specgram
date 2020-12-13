/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "input.hpp"

#include <spdlog/spdlog.h>
#include <memory>

InputReader::InputReader(std::istream &stream, std::size_t block_size_bytes)
    : stream_(stream), block_size_bytes_(block_size_bytes)
{
    assert(block_size_bytes > 0);
    this->buffer_ = new char[block_size_bytes];
    assert(this->buffer_ != nullptr);
    this->bytes_in_buffer_ = 0;
    this->running_ = true;

    /* start reader thread */
    this->reader_thread_ = std::thread(&InputReader::Read, this);
}

InputReader::~InputReader()
{
    /* end reader thread */
    this->running_mutex_.lock();
    this->running_ = false;
    this->running_mutex_.unlock();

    /* thread may be stuck in a istream::read() forever, so just forget about it */
    this->reader_thread_.detach();

    /* dealloc buffer */
    if (this->buffer_ != nullptr) {
        delete[] this->buffer_;
        this->buffer_ = nullptr;
    }
}

void
InputReader::Read()
{
    char *local_buffer = new char[this->block_size_bytes_];
    std::size_t local_count = 0;

    bool running_copy = true;

    while (running_copy) {
        /* find out how much we need to populate in the buffer */
        this->buffer_mutex_.lock();
        long long int to_read = this->block_size_bytes_ - this->bytes_in_buffer_;
        this->buffer_mutex_.unlock();
        assert(to_read >= 0);

        /* yield execution if nothing to read */
        if (to_read == 0) {
            std::this_thread::yield();
        }

        /* blocking read */
        this->stream_.read(local_buffer, to_read);

        /* write to buffer */
        this->buffer_mutex_.lock();
        assert(to_read + this->bytes_in_buffer_ <= this->block_size_bytes_);
        auto k = to_read;
        for (volatile char *a = local_buffer, *b = this->buffer_ + this->bytes_in_buffer_; k > 0; *a++ = *b++, k--) /* nop */;
        this->bytes_in_buffer_ += to_read;
        this->buffer_mutex_.unlock();

        /* check if we're exiting */
        this->running_mutex_.lock();
        running_copy = this->running_;
        this->running_mutex_.unlock();
    }
}

bool
InputReader::HasBlock()
{
    const std::lock_guard<std::mutex> lock(this->buffer_mutex_);
    return (this->bytes_in_buffer_ == this->block_size_bytes_);
}

std::optional<std::vector<char>>
InputReader::GetBlock()
{
    const std::lock_guard<std::mutex> lock(this->buffer_mutex_);
    if (this->bytes_in_buffer_ < this->block_size_bytes_) {
        return {};
    } else {
        return std::vector<char>(this->buffer_, this->buffer_ + this->bytes_in_buffer_);
    }
}

std::vector<char>
InputReader::GetBuffer()
{
    const std::lock_guard<std::mutex> lock(this->buffer_mutex_);
    std::vector<char> wrapper(this->buffer_, this->buffer_ + this->bytes_in_buffer_);
    this->bytes_in_buffer_ = 0;
    return wrapper;
}

std::size_t
InputParser::GetBufferedValueCount() const
{
    return values_.size();
}

std::vector<std::complex<double>>
InputParser::PeekValues(std::size_t count) const
{
    count = std::min<std::size_t>(count, this->values_.size());
    return std::vector<std::complex<double>> (this->values_.begin(), this->values_.begin() + count);
}

void
InputParser::RemoveValues(std::size_t count)
{
    count = std::min<std::size_t>(count, this->values_.size());
    this->values_.erase(this->values_.begin(), this->values_.begin() + count);
}

std::unique_ptr<InputParser>
InputParser::FromDataType(DataType dtype)
{
    if (dtype == DataType::kSignedInt8) {
        return std::make_unique<IntegerInputParser<char>>();
    } else if (dtype == DataType::kSignedInt16) {
        return std::make_unique<IntegerInputParser<short>>();
    } else if (dtype == DataType::kSignedInt32) {
        return std::make_unique<IntegerInputParser<int>>();
    } else if (dtype == DataType::kUnsignedInt8) {
        return std::make_unique<IntegerInputParser<unsigned char>>();
    } else if (dtype == DataType::kUnsignedInt16) {
        return std::make_unique<IntegerInputParser<unsigned short>>();
    } else if (dtype == DataType::kUnsignedInt32) {
        return std::make_unique<IntegerInputParser<unsigned int>>();
    } else {
        assert(false);
        spdlog::error("Unknown datatype!");
        return nullptr;
    }
}

template <class T>
std::size_t
IntegerInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    assert(block.size() % sizeof(T) == 0);

    std::size_t count = block.size() / sizeof(T);
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        /* TODO: figure out correct normalization for signed types? */
        double real = start[i] / std::numeric_limits<T>::max();
        this->values_.emplace_back(std::complex<double>(real, 0.0f));
    }

    return count;
}

template class IntegerInputParser<char>;
template class IntegerInputParser<short>;
template class IntegerInputParser<int>;
template class IntegerInputParser<unsigned char>;
template class IntegerInputParser<unsigned short>;
template class IntegerInputParser<unsigned int>;