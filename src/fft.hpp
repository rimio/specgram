/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _FFT_HPP_
#define _FFT_HPP_

#include "window-function.hpp"

#include <fftw3.h>

/**
 * Computes the fast Fourier transform of an input window.
 */
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
    /* plan and buffers are not copiable */
    FFT() = delete;
    FFT(const FFT &c) = delete;
    FFT(FFT &&) = delete;
    FFT & operator=(const FFT&) = delete;

    /**
     * @param win_width Width of window this object must support.
     * NOTE: Only supports this window size!
     */
    explicit FFT(std::size_t win_width);

    /**
     * @param win_width Width of window this object must support.
     * @param win_func Window function to apply before FFT computations.
     * NOTE: Only supports this window size!
     */
    FFT(std::size_t win_width, std::unique_ptr<WindowFunction>& win_func);

    virtual ~FFT();

    /**
     * Compute the fast Fourier transform.
     * @param input Array of complex input values.
     * @return Complex values corresponding to the FFT terms, equal in size to input.
     *
     * NOTE: This functions normalizes the output by 1/N.
     * NOTE: This function provides the negative frequencies in the first half
     *       and the positive frequencies in the second half.
     * NOTE: For even-sized inputs, the Nyquist frequency term is the last in
     *       the output vector.
     */
    ComplexWindow Compute(const ComplexWindow& input);

    /**
     * Retrieve the (real) magnitudes of a complex input vector (window).
     * @param input Array of complex values.
     * @param alias If true, will alias negative and positive frequencies.
     * @return Array of real values, containing amplitudes.
     *
     * NOTE: When aliasing, each output value will be the sum of that term's
     *       magnitude as well as the corresponding negative or positive
     *       frequency term's magnitude.
     */
    static RealWindow GetMagnitude(const ComplexWindow& input, bool alias);

    /**
     * Compute the frequency bounds of a specific FFT window.
     * @param rate Sampling rate of the input signal.
     * @param width Width of the FFT window.
     * @return Lower and upper bounds, in Hz.
     */
    static std::tuple<double, double> GetFrequencyLimits(double rate, std::size_t width);

    /**
     * Compute the index in the FFT window of a given frequency.
     * @param rate Sampling rate of input.
     * @param width Width of the FFT window.
     * @param f Frequency, in Hz.
     * @return Index (computed exactly) of the frequency.
     */
    static double GetFrequencyIndex(double rate, std::size_t width, double f);

    /**
     * Resample a real, scaled output of the FFT.
     * @param input Real window, values between [0..1].
     * @param rate Sampling rate of input.
     * @param width Desired output width.
     * @param fmin Frequency lower bound (from input).
     * @param fmax Frequency upper bound (from input).
     * @return Resampled values, clamped to [0..1].
     *
     * NOTE: Uses Lanczos resampling algorithm.
     * NOTE: Will resize the [fmin..fmax] band from input (computed as if
     *       input is a FFT output) to a width-sized output window.
     */
    static RealWindow Resample(const RealWindow& input, double rate, std::size_t width, double fmin, double fmax);

    /**
     * Crop a real, scaled output of the FFT.
     * @param input Real window, values between [0..1].
     * @param rate Sampling rate of input.
     * @param fmin fmin Frequency lower bound (from input).
     * @param fmax fmax Frequency upper bound (from input).
     * @return Cropped window.
     *
     * NOTE: The [fmin..fmax] band from input is computed as if input is a FFT
     *       output, which under normal circumstances it should be.
     */
    static RealWindow Crop(const RealWindow& input, double rate, double fmin, double fmax);
};

#endif
