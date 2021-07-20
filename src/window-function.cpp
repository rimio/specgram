/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "window-function.hpp"

#include <cassert>

WindowFunction::WindowFunction(std::size_t window_size) : window_size_(window_size)
{
    cached_factors_.resize(window_size);
}

std::unique_ptr<WindowFunction>
WindowFunction::FromType(WindowFunctionType type, std::size_t window_size)
{
    switch (type) {
        case WindowFunctionType::kNone:
            return nullptr; /* no window needed */

        case WindowFunctionType::kHann:
            return std::make_unique<HannWindowFunction>(window_size);

        case WindowFunctionType::kHamming:
            return std::make_unique<HammingWindowFunction>(window_size);

        case WindowFunctionType::kBlackman:
            return std::make_unique<BlackmanWindowFunction>(window_size);

        case WindowFunctionType::kNuttall:
            return std::make_unique<NuttallWindowFunction>(window_size);

        default:
            throw std::runtime_error("unknown window function");
    }
}

ComplexWindow
WindowFunction::Apply(const ComplexWindow& window) const
{
    /* only matching windows */
    if (window.size() != this->window_size_) {
        throw std::runtime_error("incorrect window size for window function application");
    }
    assert(this->cached_factors_.size() == this->window_size_);

    ComplexWindow output;
    output.resize(this->window_size_);
    for (std::size_t i = 0; i < this->window_size_; i++) {
        output[i] = window[i] * this->cached_factors_[i];
    }
    return output;
}

GeneralizedCosineWindowFunction::GeneralizedCosineWindowFunction(std::size_t window_size,
                                                                 const std::vector<double>& a)
    : WindowFunction(window_size)
{
    if (this->window_size_ == 1) {
        /* noop case */
        this->cached_factors_[0] = 1.0;
        return;
    }

    double N = (double)this->window_size_ - 1.0;
    for (std::size_t n = 0; n < this->window_size_; n++) {
        this->cached_factors_[n] = 0;
        for (unsigned int k = 0; k < a.size(); k++) {
            this->cached_factors_[n] +=
                std::pow<double>(-1.0f, k) * a[k] * std::cos(2.0 * (double)M_PI * k * (double)n / N);
        }
    }
}

HannWindowFunction::HannWindowFunction(std::size_t window_size)
    : GeneralizedCosineWindowFunction(window_size, { 0.5, 0.5 })
{
}

HammingWindowFunction::HammingWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 0.54, 0.46 })
{
}

BlackmanWindowFunction::BlackmanWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 0.42, 0.5, 0.08 })
{
}

NuttallWindowFunction::NuttallWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 0.3635819, 0.4891775, 0.1365995, 0.0106411 })
{
}
