/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _VALUE_MAP_HPP_
#define _VALUE_MAP_HPP_

#include "input-parser.hpp"

#include <string>
#include <vector>
#include <complex>

/**
 * Type of supported value maps
 */
enum class ValueMapType {
    kLinear,
    kDecibel
};

/**
 * Base value map class
 */
class ValueMap {
protected:
    const double lower_;        /* lower bound, in whatever unit */
    const double upper_;        /* upper bound, in whatever unit */
    const std::string unit_;    /* unit */

    /**
     * @param lower_ Lower bound of scale.
     * @param upper Upper bound of scale.
     * @param unit Unit.
     */
    ValueMap(double lower_, double upper, const std::string& unit);

public:
    ValueMap() = delete;

    auto GetLowerBound() const { return lower_; }
    auto GetUpperBound() const { return upper_; }
    virtual std::string GetUnit() const;

    /**
     * Maps all values to [0..1], based on bounds.
     * @param input Input values in whatever unit.
     * @return Output corresponding values, in the [0..1] domain.
     */
    virtual RealWindow Map(const RealWindow& input) = 0;

    /**
     * Build a fitting value map.
     * @param type One of ValueMapType.
     * @param lower Lower bound.
     * @param upper Upper bound.
     * @param unit Unit.
     * @return New ValueMap instance.
     */
    static std::unique_ptr<ValueMap> Build(ValueMapType type, double lower, double upper, std::string unit);
};

/**
 * Specialization for linear maps.
 */
class LinearValueMap : public ValueMap {
public:
    LinearValueMap(double lower, double upper, const std::string& unit);

    /**
     * Linearly map input to output.
     * @param input Input values in whatever unit.
     * @return Output corresponding values, in the [0..1] domain.
     *
     * NOTE: Transformation is
     *       x : [lower_ .. upper_] ---> [0 .. 1].
     */
    RealWindow Map(const RealWindow& input) override;
};

/**
 * Specialization for logarithmic maps in some dB unit
 */
class DecibelValueMap : public ValueMap {
public:
    /**
     * @param lower Lower bound.
     * @param upper Upper bound.
     * @param unit Unit without dB prefix.
     */
    DecibelValueMap(double lower, double upper, const std::string& unit);

    /**
     * Logarithmically map input to output.
     * @param input Input values in whatever unit.
     * @return Output corresponding values, in the [0..1] domain.
     *
     * NOTE: Transformation is
     *       20*log10(x) : [lower_ .. upper_] ---> [0 .. 1].
     */
    RealWindow Map(const RealWindow& input) override;

    /**
     * @return Unit with dB prefix.
     */
    std::string GetUnit() const override;
};

#endif
