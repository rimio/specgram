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
    } else {
        assert(false);
        spdlog::error("Internal error: unknown color map");
        return nullptr;
    }
}

std::vector<uint8_t>
GrayColorMap::Map(const std::vector<double>& input)
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