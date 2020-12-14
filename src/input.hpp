/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include <memory>
#include <vector>
#include <complex>
#include <mutex>
#include <thread>

/*
 * Input data type
 */
enum DataType {
    kSignedInt8,
    kSignedInt16,
    kSignedInt32,
    kUnsignedInt8,
    kUnsignedInt16,
    kUnsignedInt32,
};

/*
 * Input reader
 */
class InputReader {
private:
    std::istream &stream_;
    const std::size_t block_size_bytes_;

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
    InputReader() = delete;
    InputReader(const InputReader &c) = delete;
    InputReader(InputReader &&) = delete;
    InputReader & operator=(const InputReader&) = delete;

    InputReader(std::istream &stream, std::size_t block_size_bytes);
    virtual ~InputReader();

    bool HasBlock();
    std::optional<std::vector<char>> GetBlock();
    std::vector<char> GetBuffer();
};

/*
 * Input parser base class
 */
class InputParser {
protected:
    std::vector<std::complex<double>> values_;

    InputParser() = default;

public:
    InputParser(const InputParser &c) = delete;
    InputParser(InputParser &&) = delete;
    InputParser & operator=(const InputParser&) = delete;
    virtual ~InputParser() = default;

    static std::unique_ptr<InputParser> FromDataType(DataType dtype);

    std::size_t GetBufferedValueCount() const;

    std::vector<std::complex<double>> PeekValues(std::size_t count) const;
    void RemoveValues(std::size_t count);

    virtual std::size_t ParseBlock(const std::vector<char> &block) = 0;
    virtual std::size_t GetDataTypeSize() const = 0;
    virtual bool IsSigned() const = 0;
    virtual bool IsFloatingPoint() const = 0;
    virtual bool IsComplex() const = 0;
};

/*
 * Integer input parser
 */
template <class T>
class IntegerInputParser : public InputParser {
public:
    IntegerInputParser() = default;

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override { return sizeof(T); };
    bool IsSigned() const override { return std::numeric_limits<T>::is_signed; };
    bool IsFloatingPoint() const override { return false; };
    bool IsComplex() const override { return false; };
};

#endif