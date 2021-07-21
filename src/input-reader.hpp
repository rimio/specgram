/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _INPUT_READER_HPP_
#define _INPUT_READER_HPP_

#include <istream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

/*
 * Input reader base class
 */
class InputReader {
protected:
    std::istream * const stream_;
    const std::size_t block_size_bytes_;

    virtual std::vector<char> GetBuffer() = 0;

public:
    InputReader() = delete;
    InputReader(const InputReader&) = delete;
    InputReader(InputReader&&) = delete;
    InputReader & operator=(const InputReader&) = delete;

    InputReader(std::istream * stream, std::size_t block_size_bytes);
    virtual ~InputReader() = default;

    virtual bool ReachedEOF() const = 0;
    virtual std::optional<std::vector<char>> GetBlock() = 0;
};

/*
 * Synchronous input reader
 */
class SyncInputReader : public InputReader {
protected:
    std::vector<char> GetBuffer() override;

public:
    SyncInputReader(std::istream * stream, std::size_t block_size_bytes);

    bool ReachedEOF() const override;
    std::optional<std::vector<char>> GetBlock() override;
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

protected:
    std::vector<char> GetBuffer() override;

public:
    AsyncInputReader(std::istream * stream, std::size_t block_size_bytes);
    ~AsyncInputReader() override;

    bool ReachedEOF() const override;
    std::optional<std::vector<char>> GetBlock() override;
};

#endif
