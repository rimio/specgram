/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "fft.hpp"

#include <spdlog/spdlog.h>
#include <cassert>
#include <cstring>
#include <cmath>
#include <complex>

ValueMap::ValueMap(int lower, int upper) : lower_(lower), upper_(upper)
{
}

dBFSValueMap::dBFSValueMap(const int mindb) : ValueMap(mindb, 0)
{
}

FFT::FFT(std::size_t win_width) : window_width_(win_width)
{
    /* allocate buffers */
    this->in_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->in_ != nullptr);
    this->out_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->out_ != nullptr);

    /* compute plan */
    this->plan_ = fftw_plan_dft_1d(win_width, this->in_, this->out_, FFTW_FORWARD, FFTW_ESTIMATE);
}

FFT::~FFT()
{
    fftw_destroy_plan(this->plan_);

    fftw_free(this->in_);
    this->in_ = nullptr;
    fftw_free(this->out_);
    this->out_ = nullptr;
}

std::vector<std::complex<double>>
FFT::Compute(const std::vector<std::complex<double>>& input)
{
    /* assume the same memory representation */
    assert(sizeof(fftw_complex) == sizeof(std::complex<double>));

    /* assume we received exactly one window */
    assert(input.size() == this->window_width_);

    /* copy to input buffer */
    std::memcpy((void *)this->in_, (void *)input.data(), this->window_width_ * sizeof(fftw_complex));

    /* execute plan */
    fftw_execute(this->plan_);

    /* build output vector from output buffer */
    /* fftw maps frequencies to k/T for k=0..window_width; we want negative frequencies at the beginning of
     * the output (i.e. first half) so we switch the upper and lower halves, i.e.: */
    /*    even width: out_: [0 1 2 3 4 5 6 7] -> output: [5 6 7 0 1 2 3 4] */
    /*     odd width: out_: [0 1 2 3 4 5 6]   -> output: [5 6 7 0 1 2 3] */
    std::vector<std::complex<double>> output;
    output.resize(this->window_width_);

    auto uhl = (this->window_width_ - 1) / 2; /* upper half length */
    auto lhl = this->window_width_ - uhl; /* lower half length */
    std::memcpy((void *) output.data(),
                (void *) (this->out_ + lhl),
                uhl * sizeof(fftw_complex));
    std::memcpy((void *) (output.data() + uhl),
                (void *) this->out_,
                lhl * sizeof(fftw_complex));
    return output;
}

std::vector<std::complex<double>>
FFT::Resample(const std::vector<std::complex<double>>& input, unsigned int rate,
              std::size_t width, unsigned int fmin, unsigned int fmax)
{
    return input;
}

std::vector<double>
dBFSValueMap::Normalize(const std::vector<std::complex<double>>& input)
{
    auto n = input.size();

    std::vector<double> output;
    output.resize(n);

    for (unsigned int i = 0; i < n; i ++) {
        output[i] = 20.0 * std::log10(2.0f * std::abs<double>(input[i]) / n);
        assert(output[i] <= 0.0f); /* if failed then input normalization failed somehow */
        output[i] = std::clamp<double>(output[i], this->lower_, 0.0f);
        output[i] = output[i] / this->lower_ + 1.0f;
    }

    return output;
}