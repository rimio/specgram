/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/value-map.hpp"
#include <vector>
#include <cmath>

static std::vector<ValueMapType> ALL_VALUE_MAP_TYPES { ValueMapType::kLinear, ValueMapType::kDecibel };

TEST(TestValueMap, FactoryWrongType)
{
    EXPECT_THROW_MATCH(auto ret = ValueMap::Build((ValueMapType)999, 0.0, 1.0, "V"),
                       std::runtime_error, "unknown value map type");
}

TEST(TestValueMap, FactoryBoundsOrder)
{
    for (auto type : ALL_VALUE_MAP_TYPES) {
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, 1.0, 0.0, ""),
                           std::runtime_error, "lower bound cannot exceed upper bound");
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, std::numeric_limits<double>::quiet_NaN(), 0.0, ""),
                           std::runtime_error, "bounds cannot be nan or inf");
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, std::numeric_limits<double>::infinity(), 0.0, ""),
                           std::runtime_error, "bounds cannot be nan or inf");
        EXPECT_NO_THROW(auto ret = ValueMap::Build(type, 1.0, 1.0, ""));
    }
}

TEST(TestValueMap, FactoryBoundsNanInf)
{
    for (auto type : ALL_VALUE_MAP_TYPES) {
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, std::numeric_limits<double>::quiet_NaN(), 0.0, ""),
                           std::runtime_error, "bounds cannot be nan or inf");
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, std::numeric_limits<double>::infinity(), 0.0, ""),
                           std::runtime_error, "bounds cannot be nan or inf");
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, 0.0, std::numeric_limits<double>::quiet_NaN(), ""),
                           std::runtime_error, "bounds cannot be nan or inf");
        EXPECT_THROW_MATCH(auto ret = ValueMap::Build(type, 0.0, std::numeric_limits<double>::infinity(), ""),
                           std::runtime_error, "bounds cannot be nan or inf");
    }
}

TEST(TestValueMap, GetUnit)
{
    EXPECT_STREQ(ValueMap::Build(ValueMapType::kLinear, 0.0, 1.0, "")->GetUnit().c_str(), "");
    EXPECT_STREQ(ValueMap::Build(ValueMapType::kDecibel, 0.0, 1.0, "")->GetUnit().c_str(), "dB");
    EXPECT_STREQ(ValueMap::Build(ValueMapType::kLinear, 0.0, 1.0, "V")->GetUnit().c_str(), "V");
    EXPECT_STREQ(ValueMap::Build(ValueMapType::kDecibel, 0.0, 1.0, "V")->GetUnit().c_str(), "dBV");
}

TEST(TestValueMap, LinearMapDomain)
{
    constexpr double lower_bound = -33.0;
    constexpr double upper_bound = 57.0;
    constexpr double epsilon = 1e-9;
    constexpr std::size_t steps = 100;

    std::unique_ptr<ValueMap> map;
    EXPECT_NO_THROW(map = ValueMap::Build(ValueMapType::kLinear, lower_bound, upper_bound, ""));

    for (std::size_t i=0; i<=steps; i++) {
        double expect = (double)i / (double)steps;
        double input = lower_bound + (upper_bound - lower_bound) * expect;
        auto out = map->Map(RealWindow { input });

        EXPECT_EQ(out.size(), 1);
        EXPECT_LE(std::abs(out[0] - expect), epsilon);
    }

    {
        RealWindow expect(steps+1);
        RealWindow win(steps+1);
        for (std::size_t i = 0; i <= steps; i++) {
            expect[i] = (double) i / (double) steps;
            win[i] = lower_bound + (upper_bound - lower_bound) * expect[i];
        }
        RealWindow out;
        EXPECT_NO_THROW(out = map->Map(win));
        for (std::size_t i = 0; i <= steps; i++) {
            EXPECT_LE(std::abs(out[i] - expect[i]), epsilon);
        }
    }

    EXPECT_LE(std::abs(map->Map(RealWindow { -40.0 })[0]), epsilon);
    EXPECT_LE(std::abs(map->Map(RealWindow { 70.0 })[0] - 1.0), epsilon);
}

TEST(TestValueMap, LogarithmicMapDomain)
{
    constexpr double lower_bound = -120.;
    constexpr double upper_bound = 20.0;
    constexpr double epsilon = 1e-9;
    constexpr std::size_t steps = 100;

    std::unique_ptr<ValueMap> map;
    EXPECT_NO_THROW(map = ValueMap::Build(ValueMapType::kDecibel, lower_bound, upper_bound, ""));

    for (std::size_t i=0; i<=steps; i++) {
        double expect = (double)i / (double)steps;
        /* y = 20log10(x) */
        /* x = 10^(y/20) */
        double input = std::pow(10.0, (lower_bound + (upper_bound - lower_bound) * expect) / 20.0);
        auto out = map->Map(RealWindow { input });

        EXPECT_EQ(out.size(), 1);
        EXPECT_LE(std::abs(out[0] - expect), epsilon);
    }

    {
        RealWindow expect(steps+1);
        RealWindow win(steps+1);
        for (std::size_t i = 0; i <= steps; i++) {
            expect[i] = (double) i / (double) steps;
            win[i] = std::pow(10.0, (lower_bound + (upper_bound - lower_bound) * expect[i]) / 20.0);
        }
        RealWindow out;
        EXPECT_NO_THROW(out = map->Map(win));
        for (std::size_t i = 0; i <= steps; i++) {
            EXPECT_LE(std::abs(out[i] - expect[i]), epsilon);
        }
    }

    EXPECT_LE(std::abs(map->Map(RealWindow { 0.0 })[0]), epsilon);
    EXPECT_LE(std::abs(map->Map(RealWindow { 100.0 })[0] - 1.0), epsilon);
}