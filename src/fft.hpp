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

enum FFTScale {
    kdBFS
};

class ValueMap {
protected:
    const int lower_;
    const int upper_;

    ValueMap(int lower_, int upper);
public:
    ValueMap() = delete;

    auto GetLowerBound() const { return lower_; }
    auto GetUpperBound() const { return upper_; }

    virtual std::vector<double> Normalize(const std::vector<std::complex<double>>& input) = 0;
    virtual std::string GetUnit() const = 0;
    virtual std::string GetName() const = 0;
};

class dBFSValueMap : public ValueMap {
private:
public:
    explicit dBFSValueMap(const int mindb);

    std::vector<double> Normalize(const std::vector<std::complex<double>>& input) override;
    std::string GetUnit() const override { return "dB"; }
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

public:
    FFT() = delete;
    FFT(const FFT &c) = delete;
    FFT(FFT &&) = delete;
    FFT & operator=(const FFT&) = delete;

    explicit FFT(std::size_t win_width);
    virtual ~FFT();

    std::vector<std::complex<double>> Compute(const std::vector<std::complex<double>>& input);

    static std::vector<std::complex<double>> Resample(const std::vector<std::complex<double>>& input,
                                                      unsigned int rate, std::size_t width,
                                                      unsigned int fmin, unsigned int fmax);
};

#endif