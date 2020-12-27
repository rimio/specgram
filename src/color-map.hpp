/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _COLOR_MAP_HPP_
#define _COLOR_MAP_HPP_

#include "input-parser.hpp"

#include <vector>
#include <memory>
#include <cstdint>
#include <SFML/Graphics.hpp>

enum ColorMapType {
    /* MATLAB jet map */
    kJet,

    /* bicolor maps */
    kGray,
    kPurple,
    kBlue,
    kGreen,
    kOrange,
    kRed,

    /* custom; bg->color */
    kCustom
};

class ColorMap {
protected:
    ColorMap() = default;

public:
    ColorMap(const ColorMap&) = default;

    static std::unique_ptr<ColorMap> FromType(ColorMapType type,
                                              const sf::Color& bg_color,
                                              const sf::Color& custom_color);

    virtual std::vector<uint8_t> Map(const RealWindow& input) const = 0;
    std::vector<uint8_t> Gradient(std::size_t width) const;
};

class InterpolationColorMap : public ColorMap {
protected:
    const std::vector<sf::Color> colors_;
    const std::vector<double> values_;

    InterpolationColorMap(const std::vector<sf::Color>& colors, const std::vector<double>& vals);
    std::vector<uint8_t> GetColor(double value) const;

public:
    InterpolationColorMap() = delete;

    std::vector<uint8_t> Map(const std::vector<double>& input) const override;
};

class TwoColorMap : public InterpolationColorMap {
public:
    TwoColorMap(const sf::Color& c1, const sf::Color& c2);
};

class ThreeColorMap : public InterpolationColorMap {
public:
    ThreeColorMap(const sf::Color& c1, const sf::Color& c2, const sf::Color& c3);
};

class JetColorMap : public InterpolationColorMap {
public:
    JetColorMap();
};

#endif