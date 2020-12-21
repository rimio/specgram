/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "colormap.hpp"

#include <cmath>
#include <cassert>
#include <spdlog/spdlog.h>

std::unique_ptr<ColorMap>
ColorMap::FromType(ColorMapType type)
{
    if (type == ColorMapType::kGray) {
        return std::make_unique<GrayColorMap>();
    } else if (type == ColorMapType::kJet) {
        return std::make_unique<JetColorMap>();
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

std::vector<uint8_t>
GrayColorMap::Map(const std::vector<double>& input) const
{
    std::vector<uint8_t> output;
    output.resize(input.size() * 4);
    for (std::size_t i = 0; i < input.size(); i ++) {
        output[i * 4 + 0] = std::floor(input[i] * 255);
        output[i * 4 + 1] = std::floor(input[i] * 255);
        output[i * 4 + 2] = std::floor(input[i] * 255);
        output[i * 4 + 3] = 255;
    }

    return output;
}

InterpolationColorMap::InterpolationColorMap(const std::vector<std::vector<uint8_t>>& colors,
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
        static_cast<uint8_t>(std::round(fl * this->colors_[k][0] + fu * this->colors_[k+1][0])),
        static_cast<uint8_t>(std::round(fl * this->colors_[k][1] + fu * this->colors_[k+1][1])),
        static_cast<uint8_t>(std::round(fl * this->colors_[k][2] + fu * this->colors_[k+1][2])),
        static_cast<uint8_t>(std::round(fl * this->colors_[k][3] + fu * this->colors_[k+1][3])),
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

JetColorMap::JetColorMap() : InterpolationColorMap(
            { /* MATLAB jet colormap */
                { 0, 0, 143, 255 },
                { 0, 0, 255, 255 },
                { 0, 255, 255, 255 },
                { 255, 255, 0, 255 },
                { 255, 127, 0, 255 },
                { 255, 0, 0, 255 },
                { 127, 0, 0, 255 },
            },
            { 0.0f, 1.0f / 9.0f, 23.0f / 63.0f, 13.0f / 21.0f, 47.0f / 63.0f, 55.0 / 63.0, 1.0f }
        )
{
}