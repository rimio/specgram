/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
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

/**
 * Types of supported window functions
 */
enum class WindowFunctionType {
    kNone,
    kHann,
    kHamming,
    kBlackman,
    kNuttall
};

/**
 * Base window function class.
 */
class WindowFunction {
protected:
    const std::size_t window_size_;         /* size of window */
    std::vector<double> cached_factors_;    /* precomputed factors for each element in window */

    /**
     * @param window_size Window size.
     */
    explicit WindowFunction(std::size_t window_size);

public:
    WindowFunction() = delete;

    /**
     * Apply function to a window.
     * @param window Array of complex numbers.
     * @return Element-wise multiplication between window and precomputed factors.
     */
    ComplexWindow Apply(const ComplexWindow& window) const;

    /**
     * Build a fitting window function.
     * @param type One of WindowFunctionType
     * @param window_size Size of window.
     * @return New WindowFunction instance.
     */
    static std::unique_ptr<WindowFunction> Build(WindowFunctionType type, std::size_t window_size);
};

/**
 * Specialization for generalized cosine window functions.
 */
class GeneralizedCosineWindowFunction : public WindowFunction {
protected:
public:
    /**
     * @param window_size Window size.
     * @param a List of coefficients.
     */
    GeneralizedCosineWindowFunction(std::size_t window_size, const std::vector<double>& a);
};

/**
 * Specialization for Hann windows.
 */
class HannWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit HannWindowFunction(std::size_t window_size);
};

/**
 * Specialization for Hamming windows.
 */
class HammingWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit HammingWindowFunction(std::size_t window_size);
};

/**
 * Specialization for Blackman windows.
 */
class BlackmanWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit BlackmanWindowFunction(std::size_t window_size);
};

/**
 * Specialization for Nuttall windows.
 */
class NuttallWindowFunction : public GeneralizedCosineWindowFunction {
public:
    explicit NuttallWindowFunction(std::size_t window_size);
};

#endif
