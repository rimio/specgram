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
    if (type == ColorMapType::kJet) {
        return std::make_unique<JetColorMap>();
    } else if (type == ColorMapType::kGray) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(255, 255, 255, 255));
    } else if (type == ColorMapType::kPurple) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(206, 153, 255, 255));
    } else if (type == ColorMapType::kBlue) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(102, 163, 255, 255));
    } else if (type == ColorMapType::kGreen) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(91, 215, 91, 255));
    } else if (type == ColorMapType::kOrange) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(255, 102, 0, 255));
    } else if (type == ColorMapType::kRed) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             sf::Color(230, 0, 0, 255));
    } else if (type == ColorMapType::kCustom) {
        return std::make_unique<TwoColorMap>(bg_color,
                                             custom_color);
    } else {
        assert(false);
        spdlog::error("Internal error: unknown color map");
        return nullptr;
    }
}

std::vector<uint8_t>
ColorMap::Gradient(std::size_t width) const
{
    std::vector<double> values;
    for (std::size_t i = 0; i < width; i++) {
        values.push_back((double) i / (double) (width-1));
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
InterpolationColorMap::Map(const std::vector<double>& input) const
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