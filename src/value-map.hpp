/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _VALUE_MAP_HPP_
#define _VALUE_MAP_HPP_

#include <string>
#include <vector>
#include <complex>

enum ValueMapType {
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

#endif