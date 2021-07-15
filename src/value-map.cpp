/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "value-map.hpp"

ValueMap::ValueMap(double lower, double upper) : lower_(lower), upper_(upper)
{
}

dBFSValueMap::dBFSValueMap(double mindb) : ValueMap(mindb, 0)
{
}

RealWindow
dBFSValueMap::Map(const RealWindow& input)
{
    auto n = input.size();
    RealWindow output;
    output.resize(n);

    for (unsigned int i = 0; i < n; i ++) {
        output[i] = 20.0 * std::log10(input[i] / n);
        output[i] = std::clamp<double>(output[i], this->lower_, 0.0f);
        output[i] = 1.0f - output[i] / this->lower_;
    }

    return output;
}