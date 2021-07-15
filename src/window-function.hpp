/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _WINDOW_FUNCTION_HPP_
#define _WINDOW_FUNCTION_HPP_

#include "input-parser.hpp"

#include <vector>
#include <complex>
#include <memory>

enum class WindowFunctionType {
    kNone,
    kHann,
    kHamming,
    kBlackman,
    kNuttall
};

class WindowFunction {
protected:
    const std::size_t window_size_;
    std::vector<double> cached_factors_;

    explicit WindowFunction(std::size_t window_size);

public:
    WindowFunction() = delete;

    ComplexWindow Apply(const ComplexWindow& window) const;

    static std::unique_ptr<WindowFunction> FromType(WindowFunctionType type, std::size_t window_size);
};

class GeneralizedCosineWindowFunction : public WindowFunction {
protected:
public:
    GeneralizedCosineWindowFunction(std::size_t window_size, const std::vector<double>& a);
};

class HannWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit HannWindowFunction(std::size_t window_size);
};

class HammingWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit HammingWindowFunction(std::size_t window_size);
};

class BlackmanWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit BlackmanWindowFunction(std::size_t window_size);
};

class NuttallWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit NuttallWindowFunction(std::size_t window_size);
};

#endif