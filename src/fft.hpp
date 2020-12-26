/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _FFT_HPP_
#define _FFT_HPP_

#include <cstddef>
#include <fftw3.h>
#include <vector>
#include <complex>

enum FFTWindowFunction {
    kNone,
    kHann,
    kHamming,
    kBlackman
};

enum FFTScale {
    kdBFS
};

class ValueMap {
protected:
    const double lower_;
    const double upper_;

    ValueMap(double lower_, double upper);
public:
    ValueMap() = delete;

    auto GetLowerBound() const { return lower_; }
    auto GetUpperBound() const { return upper_; }

    virtual std::vector<double> Map(const std::vector<std::complex<double>>& input) = 0;
    virtual std::string GetUnit() const = 0;
    virtual std::string GetName() const = 0;
};

class dBFSValueMap : public ValueMap {
private:
public:
    explicit dBFSValueMap(double mindb);

    std::vector<double> Map(const std::vector<std::complex<double>>& input) override;
    std::string GetUnit() const override { return "dBFS"; }
    std::string GetName() const override { return "dBFS"; }
};

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
    bool apply_window_function_;
    std::vector<double> window_;

    static double sinc(double x);

public:
    FFT() = delete;
    FFT(const FFT &c) = delete;
    FFT(FFT &&) = delete;
    FFT & operator=(const FFT&) = delete;

    FFT(std::size_t win_width, FFTWindowFunction wf);
    virtual ~FFT();

    std::vector<std::complex<double>> Compute(const std::vector<std::complex<double>>& input, bool alias);

    static std::tuple<double, double> GetFrequencyLimits(double rate, std::size_t width);
    static double GetFrequencyIndex(double rate, std::size_t width, double f);
    static std::vector<double> Resample(const std::vector<double>& input,
                                        double rate, std::size_t width,
                                        double fmin, double fmax);
    static std::vector<double> Crop(const std::vector<double>& input,
                                    double rate, double fmin, double fmax);
};

#endif