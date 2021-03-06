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

    virtual RealWindow Map(const RealWindow& input) = 0;
    virtual std::string GetUnit() const = 0;
    virtual std::string GetName() const = 0;
};

class dBFSValueMap : public ValueMap {
private:
public:
    explicit dBFSValueMap(double mindb);

    RealWindow Map(const RealWindow& input) override;
    std::string GetUnit() const override { return "dBFS"; }
    std::string GetName() const override { return "dBFS"; }
};

#endif