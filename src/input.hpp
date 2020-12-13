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

/*
 * Input base class
 */
class Input {
protected:
    std::size_t block_size_;
    std::vector<std::complex<double>> values_;

    char *buffer_;
    unsigned int leftover_;

    Input(std::size_t block_size);

    virtual void Parse() = 0;

public:
    enum DataType {
        kSignedInt8,
        kSignedInt16,
        kSignedInt32,
        kUnsignedInt8,
        kUnsignedInt16,
        kUnsignedInt32,
    };

    Input() = delete;
    Input(const Input &c) = delete;
    Input(Input &&) = delete;
    Input & operator=(const Input&) = delete;
    virtual ~Input();

    static std::unique_ptr<Input> FromDataType(DataType dtype, std::size_t block_size);

    std::streamsize ConsumeStream(std::istream &stream);

    std::size_t GetBufferedValueCount() const;

    virtual std::size_t GetDataTypeSize() const = 0;
    virtual bool IsSigned() const = 0;
    virtual bool IsFloatingPoint() const = 0;
    virtual bool IsComplex() const = 0;
};

/*
 * Integer input
 */
template <class T>
class IntegerInput : public Input {
protected:
    virtual void Parse();

public:
    IntegerInput(std::size_t block_size);

    virtual std::size_t GetDataTypeSize() const { return sizeof(T); };
    virtual bool IsSigned() const { return std::numeric_limits<T>::is_signed; };
    virtual bool IsFloatingPoint() const { return !std::numeric_limits<T>::is_exact; };
    virtual bool IsComplex() const { return false; };
};

#endif