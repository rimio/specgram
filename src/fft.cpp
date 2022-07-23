/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "fft.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>
#include <complex>

static double
sinc(double x)
{
    /* this function is straight up lifted from boost, as I have absolutely no inclination of linking to it */
    static const double taylor_0_bound = std::numeric_limits<double>::epsilon();
    static const double taylor_2_bound = sqrt(taylor_0_bound);
    static const double taylor_n_bound = sqrt(taylor_2_bound);

    if (std::abs(x) >= taylor_n_bound) {
        return std::sin(x) / x;
    } else {
        double result = 1.0f;

        if (abs(x) >= taylor_0_bound) {
            double x2 = x * x;
            result -= x2 / 6.0f;

            if (abs(x) >= taylor_2_bound) {
                result += (x2 * x2) / 120.0f;
            }
        }

        return result;
    }
}

FFT::FFT(std::size_t win_width) : window_width_(win_width)
{
    /* no zero-width fft */
    if (win_width == 0) {
        throw std::runtime_error("cannot compute zero-width fft");
    }

    /* allocate buffers */
    this->in_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->in_ != nullptr);
    this->out_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->out_ != nullptr);

    /* compute plan */
    this->plan_ = fftw_plan_dft_1d(win_width, this->in_, this->out_, FFTW_FORWARD, FFTW_ESTIMATE);
}

FFT::FFT(std::size_t win_width, std::unique_ptr<WindowFunction>& win_func) : FFT(win_width)
{
    this->window_function_ = std::move(win_func);
}

FFT::~FFT()
{
    fftw_destroy_plan(this->plan_);

    fftw_free(this->in_);
    this->in_ = nullptr;
    fftw_free(this->out_);
    this->out_ = nullptr;
}

ComplexWindow
FFT::Compute(const ComplexWindow& input)
{
    /* assume the same memory representation */
    assert(sizeof(fftw_complex) == sizeof(Complex));

    /* assume we received exactly one window */
    if (input.size() != this->window_width_) {
        throw std::runtime_error("input window size must match FFTW plan size");
    }

    /* make a copy of the input and apply window function */
    auto input_ref = &input;
    ComplexWindow processed_input;
    if (this->window_function_ != nullptr) {
        processed_input = this->window_function_->Apply(input);
        input_ref = &processed_input;
    }

    /* copy to input buffer */
    assert(this->in_ != nullptr);
    std::memcpy((void *)this->in_, (void *)input_ref->data(), this->window_width_ * sizeof(fftw_complex));

    /* execute plan */
    fftw_execute(this->plan_);

    /* build output vector from output buffer */
    /* fftw maps frequencies to k/T for k=0..window_width; we want negative frequencies at the beginning of
     * the output (i.e. first half) so we switch the upper and lower halves, i.e.: */
    /*    even width: out_: [0 1 2 3 4 5 6 7] -> output: [5 6 7 0 1 2 3 4] */
    /*     odd width: out_: [0 1 2 3 4 5 6]   -> output: [4 5 6 0 1 2 3] */
    ComplexWindow output;
    output.resize(this->window_width_);

    assert(this->out_ != nullptr);
    auto uhl = (this->window_width_ - 1) / 2; /* upper half length */
    auto lhl = this->window_width_ - uhl; /* lower half length */
    std::memcpy((void *) output.data(),
                (void *) (this->out_ + lhl),
                uhl * sizeof(fftw_complex));
    std::memcpy((void *) (output.data() + uhl),
                (void *) this->out_,
                lhl * sizeof(fftw_complex));

    /* fftw does not normalize; divide by the window size */
    for (auto& v : output) {
        v /= (double)output.size();
    }

    return output;
}

RealWindow
FFT::GetMagnitude(const ComplexWindow& input, bool alias)
{
    auto n = input.size();

    RealWindow output;
    output.resize(n);
    for (std::size_t i = 0; i < n; i++) {
        output[i] = std::abs<double>(input[i]);
    }

    /* alias negative/positive frequencies (e.g. if input is not true complex) */
    if (alias) {
        std::size_t offset = 2 - n % 2;
        for (std::size_t i = 0; i < (n - 1) / 2; i++) {
            output[i] += output[n - i - offset];
            output[n - i - offset] = output[i];
        }
    }

    return output;
}

std::tuple<double, double>
FFT::GetFrequencyLimits(double rate, std::size_t width)
{
    if (rate <= 0) {
        throw std::runtime_error("rate must be positive in order to compute frequency limits");
    }
    if (width == 0) {
        throw std::runtime_error("FFT width must be positive in order to compute frequency limits");
    }

    if (width % 2 == 0) {
        std::size_t half = width / 2;
        return std::make_tuple(-(double)(half-1) / (double)width * (double)rate,
                               +(double)half / (double)width * (double)rate);
    } else {
        std::size_t half = width / 2;
        return std::make_tuple(-(double)half / (double)width * (double)rate,
                               +(double)half / (double)width * (double)rate);
    }
}

double
FFT::GetFrequencyIndex(double rate, std::size_t width, double f)
{
    auto [in_fmin, in_fmax] = FFT::GetFrequencyLimits(rate, width);
    assert(in_fmin < in_fmax);
    return (f - in_fmin) / (in_fmax - in_fmin) * (width - 1);
}

RealWindow
FFT::Resample(const RealWindow& input, double rate, std::size_t width, double fmin, double fmax)
{
    if (rate <= 0.0f) {
        throw std::runtime_error("rate must be positive for resampling");
    }
    if (fmin >= fmax) {
        throw std::runtime_error("resampling frequency bounds either not distinct or not in order");
    }
    if (width == 0) {
        throw std::runtime_error("resampling requires positive width");
    }

    /* find corresponding indices for fmin/fmax */
    /* [0..input.size()-1] -> [in_fmin, in_fmax] */
    /*   [i_fmin..i_fmax]  ->    [fmin, fmax]    */
    double i_fmin = FFT::GetFrequencyIndex(rate, input.size(), fmin);
    double i_fmax = FFT::GetFrequencyIndex(rate, input.size(), fmax);

    /* prepare output */
    RealWindow output;
    output.resize(width);

    /* [0..width] -> [i_fmin..i_fmax] */
    /* Lanczos resampling */
    static constexpr std::size_t lanc_a = 3;
    for (std::size_t j = 0; j < width; j++) {
        double x = (double)j / (double)(width - 1) * (i_fmax - i_fmin) + i_fmin;
        double sum = 0.0f;
        double lsum = 0.0f;

        /* convolve */
        for (int i = static_cast<int>(std::floor(x)) - static_cast<int>(lanc_a) + 1; i <= std::floor(x) + lanc_a; i ++) {
            if (i >= 0 && i < static_cast<int>(input.size())) {
                double lanc_xi = ::sinc(x - i) * ::sinc((x - i) / lanc_a);
                sum += input[i] * lanc_xi;
                lsum += lanc_xi;
            }
        }

        double value = sum / lsum;
        value = std::isnan(value) ? 0.0 : value;
        output[j] = std::clamp<double>(value, 0.0f, 1.0f);
    }

    return output;
}

RealWindow
FFT::Crop(const RealWindow& input, double rate, double fmin, double fmax)
{
    if (rate <= 0.0f) {
        throw std::runtime_error("rate must be positive for cropping");
    }
    if (fmin >= fmax) {
        throw std::runtime_error("cropping frequency bounds either not distinct or not in order");
    }

    /* find corresponding indices for fmin/fmax */
    /* [0..input.size()-1] -> [in_fmin, in_fmax] */
    /*   [i_fmin..i_fmax]  ->    [fmin, fmax]    */
    double di_fmin = std::round(FFT::GetFrequencyIndex(rate, input.size(), fmin));
    double di_fmax = std::round(FFT::GetFrequencyIndex(rate, input.size(), fmax));

    /* we're cropping, so no interpolation allowed */
    auto i_fmin = static_cast<std::int64_t>(di_fmin);
    auto i_fmax = static_cast<std::int64_t>(di_fmax);

    if (i_fmin < 0) {
        throw std::runtime_error("fmin outside of window");
    }
    if (i_fmax >= (int64_t)input.size()) {
        throw std::runtime_error("fmax outside of window");
    }

    /* return corresponding subvector */
    return RealWindow(input.begin() + i_fmin, input.begin() + i_fmax + 1);
}
