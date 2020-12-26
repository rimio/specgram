/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _INPUT_READER_HPP_
#define _INPUT_READER_HPP_

#include <istream>
#include <vector>
#include <mutex>
#include <thread>

/*
 * Input reader base class
 */
class InputReader {
protected:
    std::istream * const stream_;
    const std::size_t block_size_bytes_;

public:
    InputReader() = delete;
    InputReader(const InputReader&) = delete;
    InputReader(InputReader&&) = delete;
    InputReader & operator=(const InputReader&) = delete;

    InputReader(std::istream * stream, std::size_t block_size_bytes);
    virtual ~InputReader() = default;

    virtual bool ReachedEOF() const = 0;
    virtual std::optional<std::vector<char>> GetBlock() = 0;
    virtual std::vector<char> GetBuffer() = 0;
};

/*
 * Synchronous input reader
 */
class SyncInputReader : public InputReader {
public:
    SyncInputReader(std::istream * stream, std::size_t block_size_bytes);

    bool ReachedEOF() const override;
    std::optional<std::vector<char>> GetBlock() override;
    std::vector<char> GetBuffer() override;
};

/*
 * Asynchronous input reader
 */
class AsyncInputReader : public InputReader {
private:
    /* buffer where we read up until one block */
    volatile char *buffer_;
    volatile std::size_t bytes_in_buffer_;

    /* mutex for accessing buffer */
    std::mutex mutex_;

    /* thread for reading from input stream */
    std::thread reader_thread_;
    volatile bool running_;

    void Read();

public:
    AsyncInputReader(std::istream * stream, std::size_t block_size_bytes);
    ~AsyncInputReader() override;

    bool ReachedEOF() const override;
    std::optional<std::vector<char>> GetBlock() override;
    std::vector<char> GetBuffer() override;
};

#endif