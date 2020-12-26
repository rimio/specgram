/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
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

FFT::FFT(std::size_t win_width, FFTWindowFunction wf) : window_width_(win_width)
{
    /* allocate buffers */
    this->in_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->in_ != nullptr);
    this->out_ = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * win_width);
    assert(this->out_ != nullptr);

    /* compute plan */
    this->plan_ = fftw_plan_dft_1d(win_width, this->in_, this->out_, FFTW_FORWARD, FFTW_ESTIMATE);

    /* compute window function */
    this->apply_window_function_ = (wf != FFTWindowFunction::kNone);
    if (this->apply_window_function_) {
        this->window_.resize(win_width);
    }

    switch (wf) {
        case FFTWindowFunction::kHann: {
                auto n = win_width;
                for (std::size_t i = 0; i < win_width; i++) {
                    double sv = std::sin((double) M_PI * (double) i / (double) n);
                    this->window_[i] = sv * sv;
                }
            }
            break;

        case FFTWindowFunction::kHamming: {
                auto n = win_width;
                for (std::size_t i = 0; i < win_width; i++) {
                    double cv = std::cos(2.0f * (double) M_PI * (double) i / (double) n);
                    double a0 = 25.0f/46.0f;
                    this->window_[i] = a0 - (1.0f - a0) * cv;
                }
            }
            break;

        case FFTWindowFunction::kBlackman: {
                auto n = win_width;
                for (std::size_t i = 0; i < win_width; i++) {
                    double c1 = std::cos(2.0f * (double) M_PI * (double) i / (double) n);
                    double c2 = std::cos(4.0f * (double) M_PI * (double) i / (double) n);
                    double a0 = 7938.0f / 18608.0f;
                    double a1 = 9240.0f / 18608.0f;
                    double a2 = 1430.0f / 18608.0f;
                    this->window_[i] = a0 - a1 * c1 + a2 * c2;
                }
            }
            break;

        case FFTWindowFunction::kNuttall: {
                auto n = win_width;
                for (std::size_t i = 0; i < win_width; i++) {
                    double c1 = std::cos(2.0f * (double) M_PI * (double) i / (double) n);
                    double c2 = std::cos(4.0f * (double) M_PI * (double) i / (double) n);
                    double c3 = std::cos(6.0f * (double) M_PI * (double) i / (double) n);
                    double a0 = 0.355768f;
                    double a1 = 0.487396f;
                    double a2 = 0.144232f;
                    double a3 = 0.012604f;
                    this->window_[i] = a0 - a1 * c1 + a2 * c2 - a3 * c3;
                }
            }
            break;

        case FFTWindowFunction::kNone:
            /* nop */
            break;

        default:
            assert(false);
            spdlog::error("Internal error: unknown window function");
    }
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
FFT::Compute(const std::vector<std::complex<double>>& input, bool alias)
{
    /* assume the same memory representation */
    assert(sizeof(fftw_complex) == sizeof(std::complex<double>));

    /* assume we received exactly one window */
    assert(input.size() == this->window_width_);

    /* make a copy of the input and apply window function */
    auto input_ref = &input;
    std::vector<std::complex<double>> input_copy;
    if (this->apply_window_function_) {
        input_copy.resize(this->window_width_);
        for (std::size_t i = 0; i < this->window_width_; i ++) {
            input_copy[i] = input[i] * this->window_[i];
        }
        input_ref = &input_copy;
    }

    /* copy to input buffer */
    std::memcpy((void *)this->in_, (void *)input_ref->data(), this->window_width_ * sizeof(fftw_complex));

    /* execute plan */
    fftw_execute(this->plan_);

    /* build output vector from output buffer */
    /* fftw maps frequencies to k/T for k=0..window_width; we want negative frequencies at the beginning of
     * the output (i.e. first half) so we switch the upper and lower halves, i.e.: */
    /*    even width: out_: [0 1 2 3 4 5 6 7] -> output: [5 6 7 0 1 2 3 4] */
    /*     odd width: out_: [0 1 2 3 4 5 6]   -> output: [4 5 6 0 1 2 3] */
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

    /* alias negative/positive frequencies (e.g. if input is not true complex) */
    if (alias) {
        /* TODO: nicer way? */
        if (window_width_ % 2) {
            for (std::size_t i = 0; i < uhl; i++) {
                output[i] += output[this->window_width_ - i - 1];
            }
        } else {
            for (std::size_t i = 0; i < uhl; i++) {
                output[i] += output[this->window_width_ - i - 2];
            }
        }
    }
    return output;
}

double
FFT::sinc(double x)
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

std::tuple<double, double>
FFT::GetFrequencyLimits(double rate, std::size_t width)
{
    assert(rate > 0);
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
    return (f - in_fmin) / (in_fmax - in_fmin) * (width - 1);
}

std::vector<double>
FFT::Resample(const std::vector<double>& input, double rate,
              std::size_t width, double fmin, double fmax)
{
    assert(rate > 0);
    assert(fmin < fmax);

    /* find corresponding indices for fmin/fmax */
    /* [0..input.size()-1] -> [in_fmin, in_fmax] */
    /*   [i_fmin..i_fmax]  ->    [fmin, fmax]    */
    double i_fmin = FFT::GetFrequencyIndex(rate, input.size(), fmin);
    double i_fmax = FFT::GetFrequencyIndex(rate, input.size(), fmax);

    /* prepare output */
    std::vector<double> output;
    output.resize(width);

    /* [0..width] -> [i_fmin..i_fmax] */
    /* Lanczos resampling */
    static constexpr std::size_t lanc_a = 3;
    for (std::size_t j = 0; j < width; j++) {
        double x = (double)j / (double)(width - 1) * (i_fmax - i_fmin) + i_fmin;
        double sum = 0.0f;
        double lsum = 0.0f;

        /* convolve */
        for (int i = std::floor(x) - lanc_a + 1; i <= std::floor(x) + lanc_a; i ++) {
            if (i >= 0 && i < input.size()) {
                double lanc_xi = FFT::sinc(x - i) * FFT::sinc((x - i) / lanc_a);
                sum += input[i] * lanc_xi;
                lsum += lanc_xi;
            }
        }

        output[j] = std::clamp<double>(sum / lsum, 0.0f, 1.0f);
    }

    return output;
}

std::vector<double>
FFT::Crop(const std::vector<double>& input, double rate, double fmin, double fmax)
{
    assert(fmin < fmax);

    /* find corresponding indices for fmin/fmax */
    /* [0..input.size()-1] -> [in_fmin, in_fmax] */
    /*   [i_fmin..i_fmax]  ->    [fmin, fmax]    */
    double di_fmin = std::round(FFT::GetFrequencyIndex(rate, input.size(), fmin));
    double di_fmax = std::round(FFT::GetFrequencyIndex(rate, input.size(), fmax));
    assert(di_fmin >= 0);
    assert(di_fmax < input.size());
    assert(di_fmin < di_fmax);

    /* we're cropping, so no interpolation allowed */
    auto i_fmin = static_cast<std::size_t>(di_fmin);
    auto i_fmax = static_cast<std::size_t>(di_fmax);
    assert(i_fmax - i_fmin > 0);

    /* return corresponding subvector */
    return std::vector<double>(input.begin() + i_fmin, input.begin() + i_fmax);
}