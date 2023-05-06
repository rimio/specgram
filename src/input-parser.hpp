/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _INPUT_PARSER_HPP_
#define _INPUT_PARSER_HPP_

#include <vector>
#include <complex>
#include <memory>

/**
 * Input data type
 */
enum class DataType {
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
    kFloat64
};

/* Complex type that we normalize everything to */
typedef std::complex<double> Complex;

/* Window of real numbers */
typedef std::vector<double> RealWindow;

/* Window of complex numbers */
typedef std::vector<Complex> ComplexWindow;

/**
 * Input parser base class
 */
class InputParser {
protected:
    double prescale_factor_;        /* factor that is applied before further processing */
    bool is_complex_;               /* input is complex? */
    std::vector<Complex> values_;   /* parsed values */

    InputParser() = delete;

    /**
     * @param prescale Prescale factor, applied after parsing.
     * @param is_complex If true, input is treated as complex-typed. Two input
     *                   values will be read for each output value.
     */
    explicit InputParser(double prescale, bool is_complex);

public:
    InputParser(const InputParser &c) = delete;
    InputParser(InputParser &&) = delete;
    InputParser & operator=(const InputParser&) = delete;
    virtual ~InputParser() = default;

    /**
     * Factory method for retrieving a suitable parser.
     * @param dtype Input data type.
     * @param prescale Prescale factor to apply after parsing.
     * @param is_complex If true, input is treated as complex-typed. Two input
     *                   values will be read for each output value.
     * @return New Parser object.
     */
    static std::unique_ptr<InputParser> Build(DataType dtype, double prescale, bool is_complex);

    /**
     * @return The number of values that have been parsed, but not yet retrieved.
     */
    std::size_t GetBufferedValueCount() const;

    /**
     * Retrieves, without removing, from the parsed (buffered) value array.
     * @param count Number of values to retrieve.
     * @return
     */
    std::vector<Complex> PeekValues(std::size_t count) const;

    /**
     * Removes values from the parsed (buffered) values array.
     * @param count Number of values to remove.
     */
    void RemoveValues(std::size_t count);

    /**
     * Parses a block of bytes.
     * @param block Block of bytes that must have a size that is a multiple of
     *              the underlying data type size (or twice that for complex).
     * @return Number of parsed values.
     */
    virtual std::size_t ParseBlock(const std::vector<char> &block) = 0;

    /**
     * @return Size of the underlying data type (or twice for complex).
     */
    virtual std::size_t GetDataTypeSize() const = 0;

    /**
     * @return True if underlying data type is signed.
     */
    virtual bool IsSigned() const = 0;

    /**
     * @return True if underlying data type is floating point.
     */
    virtual bool IsFloatingPoint() const = 0;

    /**
     * @return True if parsing complex input.
     */
    virtual bool IsComplex() const { return is_complex_; };
};

/**
 * Specialized parser for integer input.
 */
template <class T>
class IntegerInputParser : public InputParser {
public:
    IntegerInputParser() = delete;
    explicit IntegerInputParser(double prescale, bool is_complex);

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override;
    bool IsSigned() const override { return std::numeric_limits<T>::is_signed; };
    bool IsFloatingPoint() const override { return false; };
};

/**
 * Specialized parser for floating point input.
 */
template <class T>
class FloatInputParser : public InputParser {
public:
    FloatInputParser() = delete;
    explicit FloatInputParser(double prescale, bool is_complex);

    std::size_t ParseBlock(const std::vector<char> &block) override;

    std::size_t GetDataTypeSize() const override;
    bool IsSigned() const override { return true; };
    bool IsFloatingPoint() const override { return true; };
};

#endif
