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
    kGray
};

class ColorMap {
protected:
    ColorMap() = default;

public:
    static std::unique_ptr<ColorMap> FromType(ColorMapType type);

    virtual std::vector<uint8_t> Map(const std::vector<double>& input) = 0;
};

class GrayColorMap : public ColorMap {
public:
    GrayColorMap() = default;

    std::vector<uint8_t> Map(const std::vector<double>& input) override;
};

#endif