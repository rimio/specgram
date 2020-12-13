/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "input.hpp"

#include <spdlog/spdlog.h>
#include <memory>
#include <sys/select.h>

Input::Input(std::size_t block_size)
{
    this->buffer_ = nullptr;
    this->block_size_ = block_size;
    this->leftover_ = 0;
}

Input::~Input()
{
    assert(this->buffer_ != nullptr);
    delete[] this->buffer_;
    this->buffer_ = nullptr;
}

std::streamsize
Input::ConsumeStream(std::istream &stream)
{
    /* read whatever is available */
    auto bytes_read = stream.readsome(this->buffer_ + this->leftover_, block_size_ * this->GetDataTypeSize());
    spdlog::info("Consumed {} of {}", bytes_read, block_size_ * this->GetDataTypeSize());
    assert(bytes_read >= 0);

    if (bytes_read > 0) {
        /* leftover initialized to whole buffer; Parse() will consume */
        this->leftover_ += bytes_read;
        this->Parse();
    }

    return bytes_read;
}

std::size_t
Input::GetBufferedValueCount() const
{
    return values_.size();
}

std::unique_ptr<Input>
Input::FromDataType(DataType dtype, size_t block_size)
{
    if (dtype == DataType::kSignedInt8) {
        return std::make_unique<IntegerInput<char>>(block_size);
    } else if (dtype == DataType::kSignedInt16) {
        return std::make_unique<IntegerInput<short>>(block_size);
    } else if (dtype == DataType::kSignedInt32) {
        return std::make_unique<IntegerInput<int>>(block_size);
    } else if (dtype == DataType::kUnsignedInt8) {
        return std::make_unique<IntegerInput<unsigned char>>(block_size);
    } else if (dtype == DataType::kUnsignedInt16) {
        return std::make_unique<IntegerInput<unsigned short>>(block_size);
    } else if (dtype == DataType::kUnsignedInt32) {
        return std::make_unique<IntegerInput<unsigned int>>(block_size);
    } else {
        assert(false);
        spdlog::error("Unknown datatype!");
        return nullptr;
    }
}

template <class T>
IntegerInput<T>::IntegerInput(std::size_t block_size) : Input(block_size)
{
    this->buffer_ = new char[(block_size + 1) * sizeof(T)];
}

template <class T>
void
IntegerInput<T>::Parse()
{
    T *nd = reinterpret_cast<T *>(this->buffer_);

    /* parse one value at a time */
    while (this->leftover_ >= sizeof(T)) {
        /* TODO: figure out correct normalization for signed types? */
        double real = (*nd) / std::numeric_limits<T>::max();
        this->values_.emplace_back(std::complex<double>(real, 0.0f));

        this->leftover_ -= sizeof(T);
        nd++;
    }

    /* move leftover to beginning of buffer */
    T *start = reinterpret_cast<T *>(this->buffer_);
    for (unsigned int i = this->leftover_; i > 0; i--, *start++ = *nd++) /* nop */ ;
}

template class IntegerInput<char>;
template class IntegerInput<short>;
template class IntegerInput<int>;
template class IntegerInput<unsigned char>;
template class IntegerInput<unsigned short>;
template class IntegerInput<unsigned int>;