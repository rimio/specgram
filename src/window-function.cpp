/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "window-function.hpp"

#include <spdlog/spdlog.h>
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
            assert(false);
            spdlog::error("Internal error: unknown window function");
            return nullptr;
    }
}

ComplexWindow
WindowFunction::Apply(const ComplexWindow& window) const
{
    /* only matching windows */
    assert(window.size() == this->window_size_);
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
    for (std::size_t n = 0; n < this->window_size_; n++) {
        this->cached_factors_[n] = 0;
        for (unsigned int k = 0; k < a.size(); k++) {
            this->cached_factors_[n] +=
                std::pow<double>(-1.0f, k) * a[k] * std::cos(2.0f * (double)M_PI * k * n / this->window_size_);
        }
    }
}

HannWindowFunction::HannWindowFunction(std::size_t window_size)
    : GeneralizedCosineWindowFunction(window_size, { 0.5f, 0.5f })
{
}

HammingWindowFunction::HammingWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 0.53836f, 0.46164f })
{
}

BlackmanWindowFunction::BlackmanWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 7938.0f / 18608.0f, 9240.0f / 18608.0f, 1430.0f / 18608.0f })
{
}

NuttallWindowFunction::NuttallWindowFunction(std::size_t window_size)
        : GeneralizedCosineWindowFunction(window_size, { 0.355768f, 0.487396f, 0.144232f, 0.012604f })
{
}