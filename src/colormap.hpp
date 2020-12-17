/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _COLORMAP_HPP_
#define _COLORMAP_HPP_

#include <vector>
#include <memory>
#include <cstdint>

enum ColorMapType {
    kGray,
    kJet
};

class ColorMap {
protected:
    ColorMap() = default;

public:
    ColorMap(const ColorMap&) = default;

    static std::unique_ptr<ColorMap> FromType(ColorMapType type);

    virtual std::vector<uint8_t> Map(const std::vector<double>& input) const = 0;
};

class GrayColorMap : public ColorMap {
public:
    GrayColorMap() = default;

    std::vector<uint8_t> Map(const std::vector<double>& input) const override;
};

class InterpolationColorMap : public ColorMap {
protected:
    const std::vector<std::vector<uint8_t>> colors_;
    const std::vector<double> values_;

    InterpolationColorMap(const std::vector<std::vector<uint8_t>>& colors, const std::vector<double>& vals);
    std::vector<uint8_t> GetColor(double value) const;

public:
    InterpolationColorMap() = delete;

    std::vector<uint8_t> Map(const std::vector<double>& input) const override;
};

class JetColorMap : public InterpolationColorMap {
public:
    JetColorMap();
};

#endif