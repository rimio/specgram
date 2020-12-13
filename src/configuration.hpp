/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _CONFIGURATION_HPP_
#define _CONFIGURATION_HPP_

#include "input.hpp"

#include <string>
#include <optional>
#include <tuple>

class Configuration {
private:
    std::optional<std::string> filename_;

    std::size_t block_size_;
    unsigned int rate_;
    DataType datatype_;

    std::size_t fft_width_;
    std::size_t fft_stride_;

    Configuration();

public:
    /* parse command line arguments and return a configuration object */
    static std::tuple<Configuration, int, bool> FromArgs(int argc, char **argv);

    /* generic getters */
    const auto & GetFilename() const { return filename_; }

    /* input getters */
    auto GetBlockSize() const { return block_size_; }
    auto GetRate() const { return rate_; }
    auto GetDataType() const { return datatype_; }

    /* FFT getters */
    auto GetFFTWidth() const { return fft_width_; }
    auto GetFFTStride() const { return fft_stride_; }
};

#endif