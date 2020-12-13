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
    std::vector<std::complex<double>> output;
    output.reserve(window_width_);
    std::memcpy((void *)output.data(), (void *)this->out_, this->window_width_ * sizeof(fftw_complex));
    return output;
}