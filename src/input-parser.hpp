/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _INPUT_PARSER_HPP_
#define _INPUT_PARSER_HPP_

#include <vector>
#include <complex>
#include <memory>

/*
 * Input data type
 */
enum DataType {
    /* signed integer */
    kSignedInt8,
    kSignedInt16,
    kSignedInt32,
    kSignedInt64,

    /* unsigned integer */
    kUnsignedInt8,
    kUnsignedInt16,
    kUnsignedInt32,
    kUnsignedInt64,

    /* floating point */
    kFloat32,
    kFloat64,

    /* complex */
    kComplex64,
    kComplex128
};

/*
 * Input parser base class
 */
class InputParser {
protected:
    double prescale_factor_;
    std::vector<std::complex<double>> values_;

    InputParser() = delete;
    explicit InputParser(double prescale);

public:
    InputParser(const InputParser &c) = delete;
    InputParser(InputParser &&) = delete;
    InputParser & operator=(const InputParser&) = delete;
    virtual ~InputParser() = default;

    static std::unique_ptr<InputParser> FromDataType(DataType dtype, double prescale);

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
    IntegerInputParser() = delete;
    explicit IntegerInputParser(double prescale);

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override { return sizeof(T); };
    bool IsSigned() const override { return std::numeric_limits<T>::is_signed; };
    bool IsFloatingPoint() const override { return false; };
    bool IsComplex() const override { return false; };
};

/*
 * Floating point input parser
 */
template <class T>
class FloatInputParser : public InputParser {
public:
    FloatInputParser() = delete;
    explicit FloatInputParser(double prescale);

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override { return sizeof(T); };
    bool IsSigned() const override { return true; };
    bool IsFloatingPoint() const override { return true; };
    bool IsComplex() const override { return false; };
};

/*
 * Complex input parser
 */
template <class T>
class ComplexInputParser : public InputParser {
public:
    ComplexInputParser() = delete;
    explicit ComplexInputParser(double prescale);

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override { return 2 * sizeof(T); };
    bool IsSigned() const override { return true; };
    bool IsFloatingPoint() const override { return true; };
    bool IsComplex() const override { return true; };
};

#endif