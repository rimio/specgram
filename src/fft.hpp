/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _FFT_HPP_
#define _FFT_HPP_

#include "window-function.hpp"

#include <fftw3.h>

class FFT {
private:
    /* FFT window width */
    const std::size_t window_width_;

    /* fftw buffers */
    fftw_complex *in_;
    fftw_complex *out_;

    /* fftw plan */
    fftw_plan plan_;

    /* window function */
    std::unique_ptr<WindowFunction> window_function_;

public:
    FFT() = delete;
    FFT(const FFT &c) = delete;
    FFT(FFT &&) = delete;
    FFT & operator=(const FFT&) = delete;

    FFT(std::size_t win_width, std::unique_ptr<WindowFunction>& win_func);
    virtual ~FFT();

    ComplexWindow Compute(const ComplexWindow& input);

    static RealWindow GetMagnitude(const ComplexWindow& input, bool alias);
    static std::tuple<double, double> GetFrequencyLimits(double rate, std::size_t width);
    static double GetFrequencyIndex(double rate, std::size_t width, double f);
    static RealWindow Resample(const RealWindow& input, double rate, std::size_t width, double fmin, double fmax);
    static RealWindow Crop(const RealWindow& input, double rate, double fmin, double fmax);
};

#endif