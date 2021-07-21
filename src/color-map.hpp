/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
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

enum class ColorMapType {
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

/**
 * Base color map class.
 *
 * This maps values in the domain [0..1] to 32bit RGBA colors, based on
 * various color schemes.
 */
class ColorMap {
protected:
    ColorMap() = default;

public:
    ColorMap(const ColorMap&) = default;

    /**
     * Factory method for colormaps.
     * @param type One of ColorMapType.
     * @param bg_color Background color (where applicable).
     * @param custom_color Custom color (where applicable).
     * @return A new ColorMap object.
     */
    static std::unique_ptr<ColorMap> Build(ColorMapType type,
                                           const sf::Color& bg_color,
                                           const sf::Color& custom_color);

    /**
     * Map a window of real values to RGBA colours.
     * @param input Array of floating point values in the domain [0..1].
     * @return Array of bytes, 4 for each input value, RGBA format.
     */
    virtual std::vector<uint8_t> Map(const RealWindow& input) const = 0;

    /**
     * Create a gradient of the colormap, displaying all possible colors.
     * @param width Width of the gradient (in pixels).
     * @return Array of bytes, one for each pixel, RGBA format.
     */
    std::vector<uint8_t> Gradient(std::size_t width) const;

    /**
     * @return Copy of this colormap.
     */
    virtual std::unique_ptr<ColorMap> Copy() const = 0;
};

/**
 * Color map that linearly interpolates between a number of specified colors,
 * based on value landmarks.
 */
class InterpolationColorMap : public ColorMap {
private:
    const std::vector<sf::Color> colors_;
    const std::vector<double> values_;

    std::vector<uint8_t> GetColor(double value) const;

public:
    /**
     * Create an interpolation-based colormap.
     * @param colors Vectors of colors used for interpolation.
     * @param vals The corresponding value for each color.
     *
     * NOTE: First value in "vals" must be 0.0 and last value must be "1.0".
     *       Thus, the whole [0..1] domain is covered.
     */
    InterpolationColorMap(const std::vector<sf::Color>& colors, const std::vector<double>& vals);
    InterpolationColorMap() = delete;

    std::vector<uint8_t> Map(const std::vector<double>& input) const override;
    std::unique_ptr<ColorMap> Copy() const override;
};

/**
 * Specialization for only two colors.
 */
class TwoColorMap : public InterpolationColorMap {
public:
    /**
     * @param c1 First color, corresponding to 0.0.
     * @param c2 Second color, corresponding to 1.0.
     */
    TwoColorMap(const sf::Color& c1, const sf::Color& c2);
};

/**
 * Specialization for only three colors.
 */
class ThreeColorMap : public InterpolationColorMap {
public:
    /**
     * @param c1 First color, corresponding to 0.0.
     * @param c2 Second color, corresponding to 0.5.
     * @param c3 Third color, corresponding to 1.0.
     */
    ThreeColorMap(const sf::Color& c1, const sf::Color& c2, const sf::Color& c3);
};

/**
 * Jet colormap, as seen in MATLAB, matplotlib and others.
 */
class JetColorMap : public InterpolationColorMap {
public:
    JetColorMap();
};

#endif