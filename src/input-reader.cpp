/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "input-reader.hpp"

#include <spdlog/spdlog.h>
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