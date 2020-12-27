/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "input-parser.hpp"

#include <cassert>

InputParser::InputParser(double prescale) : prescale_factor_(prescale)
{
}

std::size_t
InputParser::GetBufferedValueCount() const
{
    return values_.size();
}

std::vector<Complex>
InputParser::PeekValues(std::size_t count) const
{
    count = std::min<std::size_t>(count, this->values_.size());
    if (count == 0) {
        return std::vector<Complex>();
    }
    return std::vector<Complex> (this->values_.begin(), this->values_.begin() + count);
}

void
InputParser::RemoveValues(std::size_t count)
{
    count = std::min<std::size_t>(count, this->values_.size());
    if (count > 0) {
        this->values_.erase(this->values_.begin(), this->values_.begin() + count);
    }
}

std::unique_ptr<InputParser>
InputParser::FromDataType(DataType dtype, double prescale)
{
    if (dtype == DataType::kSignedInt8) {
        return std::make_unique<IntegerInputParser<char>>(prescale);
    } else if (dtype == DataType::kSignedInt16) {
        return std::make_unique<IntegerInputParser<short>>(prescale);
    } else if (dtype == DataType::kSignedInt32) {
        return std::make_unique<IntegerInputParser<int>>(prescale);
    } else if (dtype == DataType::kSignedInt64) {
        return std::make_unique<IntegerInputParser<long long>>(prescale);
    } else if (dtype == DataType::kUnsignedInt8) {
        return std::make_unique<IntegerInputParser<unsigned char>>(prescale);
    } else if (dtype == DataType::kUnsignedInt16) {
        return std::make_unique<IntegerInputParser<unsigned short>>(prescale);
    } else if (dtype == DataType::kUnsignedInt32) {
        return std::make_unique<IntegerInputParser<unsigned int>>(prescale);
    } else if (dtype == DataType::kUnsignedInt64) {
        return std::make_unique<IntegerInputParser<unsigned long long>>(prescale);
    } else if (dtype == DataType::kFloat32) {
        return std::make_unique<FloatInputParser<float>>(prescale);
    } else if (dtype == DataType::kFloat64) {
        return std::make_unique<FloatInputParser<double>>(prescale);
    } else if (dtype == DataType::kComplex64) {
        return std::make_unique<ComplexInputParser<float>>(prescale);
    } else if (dtype == DataType::kComplex128) {
        return std::make_unique<ComplexInputParser<double>>(prescale);
    } else {
        throw std::runtime_error("unknown datatype");
    }
}

template <class T>
IntegerInputParser<T>::IntegerInputParser(double prescale) : InputParser(prescale)
{
}

template <class T>
std::size_t
IntegerInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    if (block.size() % sizeof(T) != 0) {
        throw std::runtime_error("block size must be a multiple of sizeof(datatype)");
    }

    std::size_t count = block.size() / sizeof(T);
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        double real = (double) start[i] / std::numeric_limits<T>::max();
        /* make signed domain [-0.5..0.5] so that we can use a maximum amplitude of 1.0 */
        real *= std::numeric_limits<T>::is_signed ? 0.5f : 1.0f;
        real *= this->prescale_factor_;

        this->values_.emplace_back(Complex(real, 0.0f));
    }

    return count;
}

template <class T>
FloatInputParser<T>::FloatInputParser(double prescale) : InputParser(prescale)
{
}

template <class T>
std::size_t
FloatInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    if (block.size() % sizeof(T) != 0) {
        throw std::runtime_error("block size must be a multiple of sizeof(datatype)");
    }

    std::size_t count = block.size() / sizeof(T);
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        /* remove NaNs */
        double real = std::isnan(start[i]) ? 0.0f : start[i];
        /* prescale */
        real *= this->prescale_factor_;

        this->values_.emplace_back(Complex(real, 0.0f));
    }

    return count;
}

template <class T>
ComplexInputParser<T>::ComplexInputParser(double prescale) : InputParser(prescale)
{
}

template <class T>
std::size_t
ComplexInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    if (block.size() % (sizeof(T) * 2) != 0) {
        throw std::runtime_error("block size must be a multiple of 2*sizeof(datatype)");
    }

    std::size_t count = block.size() / sizeof(T) / 2;
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        /* remove NaNs */
        double real = std::isnan(start[i * 2 + 0]) ? 0.0f : start[i * 2 + 0];
        double imag = std::isnan(start[i * 2 + 1]) ? 0.0f : start[i * 2 + 1];
        /* prescale */
        real *= this->prescale_factor_;
        imag *= this->prescale_factor_;

        this->values_.emplace_back(Complex(real, imag));
    }

    return count;
}