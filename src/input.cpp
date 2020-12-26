/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "input.hpp"

#include <spdlog/spdlog.h>
#include <memory>
#include <csignal>

InputReader::InputReader(std::istream * stream, std::size_t block_size_bytes)
    : stream_(stream), block_size_bytes_(block_size_bytes)
{
}

SyncInputReader::SyncInputReader(std::istream * stream, std::size_t block_size_bytes)
    : InputReader(stream, block_size_bytes)
{
}

bool
SyncInputReader::ReachedEOF() const
{
    return stream_->eof();
}

std::optional<std::vector<char>>
SyncInputReader::GetBlock()
{
    auto buffer = this->GetBuffer();
    if (buffer.size() == this->block_size_bytes_) {
        return buffer;
    } else {
        /* we assume the only time we're reading less than a block is at EOF */
        assert(this->stream_->eof());
        return {};
    }
}

std::vector<char>
SyncInputReader::GetBuffer()
{
    std::vector<char> local_buffer;
    local_buffer.resize(this->block_size_bytes_);

    this->stream_->read(local_buffer.data(), this->block_size_bytes_);
    return std::vector<char>(local_buffer.data(), local_buffer.data() + this->stream_->gcount());
}

AsyncInputReader::AsyncInputReader(std::istream * stream, std::size_t block_size_bytes)
    : InputReader(stream, block_size_bytes)
{
    assert(block_size_bytes > 0);
    this->buffer_ = new char[block_size_bytes];
    assert(this->buffer_ != nullptr);
    this->bytes_in_buffer_ = 0;

    /* start reader thread */
    this->running_ = true;
    this->reader_thread_ = std::thread(&AsyncInputReader::Read, this);
}

AsyncInputReader::~AsyncInputReader()
{
    /* end reader thread */
    this->mutex_.lock();
    this->running_ = false;
    this->mutex_.unlock();

    /* send SIGINT so we interrupt any blocking reads */
    pthread_kill(this->reader_thread_.native_handle(), SIGINT);
    this->reader_thread_.join();

    /* dealloc buffer */
    if (this->buffer_ != nullptr) {
        delete[] this->buffer_;
        this->buffer_ = nullptr;
    }
}

void
AsyncInputReader::Read()
{
    char *local_buffer = new char[this->block_size_bytes_];

    while (true) {
        /* find out how much we need to populate in the buffer */
        this->mutex_.lock();
        long long int to_read = this->block_size_bytes_ - this->bytes_in_buffer_;
        if (!this->running_) {
            this->mutex_.unlock();
            break;
        }
        this->mutex_.unlock();
        assert(to_read >= 0);

        /* yield execution if nothing to read */
        if (to_read == 0) {
            std::this_thread::yield();
            continue;
        }

        /* blocking read */
        this->stream_->read(local_buffer, to_read);
        if (this->stream_->fail()) {
            break;
        } else {
            assert(this->stream_->gcount() == to_read);
        }

        /* write to buffer */
        this->mutex_.lock();
        if (!this->running_) {
            this->mutex_.unlock();
            break;
        }
        assert(to_read + this->bytes_in_buffer_ <= this->block_size_bytes_);
        auto k = to_read;
        for (volatile char *a = local_buffer, *b = this->buffer_ + this->bytes_in_buffer_; k > 0; *b++ = *a++, k--) /* nop */;
        this->bytes_in_buffer_ = this->bytes_in_buffer_ + to_read;
        this->mutex_.unlock();
    }
}

bool
AsyncInputReader::ReachedEOF() const
{
    /* we will never reach end of file in async mode */
    /* only way out is SIGINT from user */
    return false;
}

std::optional<std::vector<char>>
AsyncInputReader::GetBlock()
{
    const std::lock_guard<std::mutex> lock(this->mutex_);
    if (this->bytes_in_buffer_ < this->block_size_bytes_) {
        return {};
    } else {
        auto bcount = this->bytes_in_buffer_;
        this->bytes_in_buffer_ = 0;
        return std::vector<char>(this->buffer_, this->buffer_ + bcount);
    }
}

std::vector<char>
AsyncInputReader::GetBuffer()
{
    const std::lock_guard<std::mutex> lock(this->mutex_);
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
        spdlog::error("Internal error: unknown datatype!");
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
        double real = (double) start[i] / std::numeric_limits<T>::max();
        /* make signed domain [-0.5..0.5] so that we can use a maximum amplitude of 1.0 */
        real *= std::numeric_limits<T>::is_signed ? 0.5f : 1.0f;
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