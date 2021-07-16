/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
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

enum class ValueMapType {
    kLinear,
    kDecibel
};

class ValueMap {
protected:
    const double lower_;
    const double upper_;
    const std::string unit_;

    ValueMap(double lower_, double upper, const std::string& unit);

public:
    ValueMap() = delete;

    auto GetLowerBound() const { return lower_; }
    auto GetUpperBound() const { return upper_; }

    virtual RealWindow Map(const RealWindow& input) = 0;
    virtual std::string GetUnit() const;

    static std::unique_ptr<ValueMap> Build(ValueMapType type, double lower, double upper, std::string unit);
};

class LinearValueMap : public ValueMap {
public:
    LinearValueMap(double lower, double upper, const std::string& unit);

    RealWindow Map(const RealWindow& input) override;
};

class DecibelValueMap : public ValueMap {
public:
    /* unit parameter should NOT contain "dB" prefix */
    DecibelValueMap(double lower, double upper, const std::string& unit);

    RealWindow Map(const RealWindow& input) override;
    std::string GetUnit() const override;
};

#endif