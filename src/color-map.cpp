/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "color-map.hpp"

#include <cmath>
#include <cassert>
#include <spdlog/spdlog.h>

std::unique_ptr<ColorMap>
ColorMap::FromType(ColorMapType type,
                   const sf::Color& bg_color,
                   const sf::Color& custom_color)
{
    /* black and white */
    sf::Color black(0, 0, 0);
    sf::Color white(255, 255, 255);

    /* colormaps are not allowed to be translucent */
    sf::Color opaque_bg_color(bg_color.r, bg_color.g, bg_color.b, 255);
    sf::Color opaque_custom_color(custom_color.r, custom_color.g, custom_color.b, 255);

    switch (type) {
        case ColorMapType::kJet:
            return std::make_unique<JetColorMap>();

        case ColorMapType::kGray:
            return std::make_unique<TwoColorMap>(black, white);

        case ColorMapType::kPurple:
            return std::make_unique<ThreeColorMap>(black, sf::Color(204, 51, 255, 255), white);

        case ColorMapType::kBlue:
            return std::make_unique<ThreeColorMap>(black, sf::Color(51, 51, 255, 255), white);

        case ColorMapType::kGreen:
            return std::make_unique<ThreeColorMap>(black, sf::Color(0, 150, 0, 255), white);

        case ColorMapType::kOrange:
            return std::make_unique<ThreeColorMap>(black, sf::Color(255, 102, 0, 255), white);

        case ColorMapType::kRed:
            return std::make_unique<ThreeColorMap>(black, sf::Color(230, 0, 0, 255), white);

        case ColorMapType::kCustom:
            return std::make_unique<TwoColorMap>(opaque_bg_color, opaque_custom_color);

        default:
            assert(false);
            spdlog::error("Internal error: unknown color map");
            return nullptr;
    }
}

std::vector<uint8_t>
ColorMap::Gradient(std::size_t width) const
{
    RealWindow values;
    values.resize(width);
    for (std::size_t i = 0; i < width; i++) {
        values[i] = (double) i / (double) (width-1);
    }
    return this->Map(values);
}

InterpolationColorMap::InterpolationColorMap(const std::vector<sf::Color>& colors,
                                             const std::vector<double>& vals) : colors_(colors), values_(vals)
{
    /* respect boundaries */
    assert(vals.size() == colors.size());
    assert(vals.size() >= 2);
    assert(vals[0] == 0.0f);
    assert(vals[vals.size()-1] == 1.0f);
}

std::vector<uint8_t>
InterpolationColorMap::GetColor(double value) const
{
    assert(value >= 0.0f);
    assert(value <= 1.0f);

    std::size_t k = 0;
    while (value > this->values_[k+1]) {
        k++;
    }

    float fu = (value - this->values_[k]) / (this->values_[k+1] - this->values_[k]);
    float fl = 1.0f - fu;

    return {
        static_cast<uint8_t>(std::round(fl * this->colors_[k].r + fu * this->colors_[k+1].r)),
        static_cast<uint8_t>(std::round(fl * this->colors_[k].g + fu * this->colors_[k+1].g)),
        static_cast<uint8_t>(std::round(fl * this->colors_[k].b + fu * this->colors_[k+1].b)),
        static_cast<uint8_t>(std::round(fl * this->colors_[k].a + fu * this->colors_[k+1].a)),
    };
}

std::vector<uint8_t>
InterpolationColorMap::Map(const RealWindow& input) const
{
    std::vector<uint8_t> output;
    output.resize(input.size() * 4);

    for (std::size_t i = 0; i < input.size(); i++) {
        auto color = this->GetColor(input[i]);
        assert(color.size() == 4);
        output[i * 4 + 0] = color[0];
        output[i * 4 + 1] = color[1];
        output[i * 4 + 2] = color[2];
        output[i * 4 + 3] = color[3];
    }

    return output;
}

TwoColorMap::TwoColorMap(const sf::Color& c1, const sf::Color& c2)
    : InterpolationColorMap({ c1, c2 }, { 0.0f, 1.0f })
{
}

ThreeColorMap::ThreeColorMap(const sf::Color& c1, const sf::Color& c2, const sf::Color& c3)
    : InterpolationColorMap({ c1, c2, c3 }, { 0.0f, 0.5f, 1.0f })
{
}

JetColorMap::JetColorMap() : InterpolationColorMap(
            { /* MATLAB jet colormap */
                sf::Color(0, 0, 143, 255),
                sf::Color(0, 0, 255, 255),
                sf::Color(0, 255, 255, 255),
                sf::Color(255, 255, 0, 255),
                sf::Color(255, 127, 0, 255),
                sf::Color(255, 0, 0, 255),
                sf::Color(127, 0, 0, 255),
            },
            { 0.0f, 1.0f / 9.0f, 23.0f / 63.0f, 13.0f / 21.0f, 47.0f / 63.0f, 55.0 / 63.0, 1.0f }
        )
{
}