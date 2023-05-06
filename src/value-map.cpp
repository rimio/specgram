/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "value-map.hpp"
#include <algorithm>
#include <cmath>

ValueMap::ValueMap(double lower, double upper, const std::string& unit) : lower_(lower), upper_(upper), unit_(unit)
{
    if (std::isnan(lower) || std::isnan(upper) || std::isinf(lower) || std::isinf(upper)) {
        throw std::runtime_error("bounds cannot be nan or inf");
    }
    if (lower > upper) {
        throw std::runtime_error("lower bound cannot exceed upper bound");
    }
}

std::string
ValueMap::GetUnit() const
{
    return unit_;
}

std::unique_ptr<ValueMap>
ValueMap::Build(ValueMapType type, double lower, double upper, std::string unit)
{
    switch (type) {
        case ValueMapType::kLinear:
            return std::make_unique<LinearValueMap>(lower, upper, unit);

        case ValueMapType::kDecibel:
            return std::make_unique<DecibelValueMap>(lower, upper, unit);

        default:
            throw std::runtime_error("unknown value map type");
    }
}

LinearValueMap::LinearValueMap(double lower, double upper, const std::string &unit)
    : ValueMap(lower, upper, unit)
{
}

RealWindow LinearValueMap::Map(const RealWindow &input)
{
    auto n = input.size();
    RealWindow output(n);

    for (unsigned int i = 0; i < n; i ++) {
        output[i] = std::clamp<double>(input[i], this->lower_, this->upper_);
        output[i] = (output[i] - this->lower_) / (this->upper_ - this->lower_);
    }

    return output;
}

DecibelValueMap::DecibelValueMap(double lower, double upper, const std::string &unit)
    : ValueMap(lower, upper, unit)
{
}

RealWindow DecibelValueMap::Map(const RealWindow &input)
{
    auto n = input.size();
    RealWindow output(n);

    for (unsigned int i = 0; i < n; i ++) {
        output[i] = 20.0 * std::log10(input[i]);
        output[i] = std::clamp<double>(output[i], this->lower_, this->upper_);
        output[i] = (output[i] - this->lower_) / (this->upper_ - this->lower_);
    }

    return output;
}

std::string DecibelValueMap::GetUnit() const
{
    return "dB" + unit_;
}
