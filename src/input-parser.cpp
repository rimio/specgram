/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "input-parser.hpp"

#include <cassert>

InputParser::InputParser(double prescale, bool is_complex) : prescale_factor_(prescale), is_complex_(is_complex)
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
InputParser::Build(DataType dtype, double prescale, bool is_complex)
{
    if (dtype == DataType::kSignedInt8) {
        return std::make_unique<IntegerInputParser<std::int8_t>>(prescale, is_complex);
    } else if (dtype == DataType::kSignedInt16) {
        return std::make_unique<IntegerInputParser<std::int16_t>>(prescale, is_complex);
    } else if (dtype == DataType::kSignedInt32) {
        return std::make_unique<IntegerInputParser<std::int32_t>>(prescale, is_complex);
    } else if (dtype == DataType::kSignedInt64) {
        return std::make_unique<IntegerInputParser<std::int64_t>>(prescale, is_complex);
    } else if (dtype == DataType::kUnsignedInt8) {
        return std::make_unique<IntegerInputParser<std::uint8_t>>(prescale, is_complex);
    } else if (dtype == DataType::kUnsignedInt16) {
        return std::make_unique<IntegerInputParser<std::uint16_t>>(prescale, is_complex);
    } else if (dtype == DataType::kUnsignedInt32) {
        return std::make_unique<IntegerInputParser<std::uint32_t>>(prescale, is_complex);
    } else if (dtype == DataType::kUnsignedInt64) {
        return std::make_unique<IntegerInputParser<std::uint64_t>>(prescale, is_complex);
    } else if (dtype == DataType::kFloat32) {
        return std::make_unique<FloatInputParser<float>>(prescale, is_complex);
    } else if (dtype == DataType::kFloat64) {
        return std::make_unique<FloatInputParser<double>>(prescale, is_complex);
    } else {
        throw std::runtime_error("unknown datatype");
    }
}

template <class T>
IntegerInputParser<T>::IntegerInputParser(double prescale, bool is_complex) : InputParser(prescale, is_complex)
{
}

template <class T>
std::size_t
IntegerInputParser<T>::GetDataTypeSize() const
{
    return sizeof(T) * (this->is_complex_ ? 2 : 1);
}

template <class T>
std::size_t
IntegerInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    std::size_t item_size = (this->is_complex_ ? 2 : 1) * sizeof(T);
    if (block.size() % item_size != 0) {
        throw std::runtime_error("block size must be a multiple of sizeof(datatype)");
    }

    std::size_t count = block.size() / item_size;
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        Complex value;
        if (this->is_complex_) {
            value = Complex(start[i * 2], start[i * 2 + 1]);
        } else {
            value = Complex(start[i], 0.0f);
        }

        /* normalize to domain limit */
        value /= (double)std::numeric_limits<T>::max();
        value *= this->prescale_factor_;
        this->values_.emplace_back(value);
    }

    return count;
}

template <class T>
FloatInputParser<T>::FloatInputParser(double prescale, bool is_complex) : InputParser(prescale, is_complex)
{
}

template <class T>
std::size_t
FloatInputParser<T>::GetDataTypeSize() const
{
    return sizeof(T) * (this->is_complex_ ? 2 : 1);
}

template <class T>
std::size_t
FloatInputParser<T>::ParseBlock(const std::vector<char> &block)
{
    /* this function assumes well structured blocks */
    std::size_t item_size = (this->is_complex_ ? 2 : 1) * sizeof(T);
    if (block.size() % item_size != 0) {
        throw std::runtime_error("block size must be a multiple of sizeof(datatype)");
    }

    std::size_t count = block.size() / item_size;
    const T *start = reinterpret_cast<const T *>(block.data());

    /* parse one value at a time into complex target */
    for (std::size_t i = 0; i < count; i ++) {
        Complex value;
        /* remove NaNs */
        if (this->is_complex_) {
            value = Complex(std::isnan(start[i * 2]) ? 0.0f : start[i * 2],
                            std::isnan(start[i * 2 + 1]) ? 0.0f : start[i * 2 + 1]);
        } else {
            value = Complex(std::isnan(start[i]) ? 0.0f : start[i], 0.0f);
        }

        /* prescale */
        value *= this->prescale_factor_;
        this->values_.emplace_back(value);
    }

    return count;
}

template class IntegerInputParser<std::uint8_t>;
template class IntegerInputParser<std::uint16_t>;
template class IntegerInputParser<std::uint32_t>;
template class IntegerInputParser<std::uint64_t>;

template class IntegerInputParser<std::int8_t>;
template class IntegerInputParser<std::int16_t>;
template class IntegerInputParser<std::int32_t>;
template class IntegerInputParser<std::int64_t>;

template class FloatInputParser<float>;
template class FloatInputParser<double>;
