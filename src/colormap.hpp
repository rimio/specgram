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
#include <SFML/Graphics.hpp>

enum ColorMapType {
    /* luma gray */
    kGray,

    /* MATLAB jet map */
    kJet,

    /* bicolor maps */
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

    virtual std::vector<uint8_t> Map(const std::vector<double>& input) const = 0;
    std::vector<uint8_t> Gradient(std::size_t width) const;
};

class GrayColorMap : public ColorMap {
public:
    GrayColorMap() = default;

    std::vector<uint8_t> Map(const std::vector<double>& input) const override;
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

class JetColorMap : public InterpolationColorMap {
public:
    JetColorMap();
};

#endif