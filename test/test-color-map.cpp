/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/color-map.hpp"
#include <vector>

std::vector<ColorMapType> ALL_COLOR_MAP_TYPES {
    ColorMapType::kJet,
    ColorMapType::kGray,
    ColorMapType::kPurple,
    ColorMapType::kBlue,
    ColorMapType::kGreen,
    ColorMapType::kOrange,
    ColorMapType::kRed,
    ColorMapType::kCustom
};

const sf::Color BLACK(  0,   0,   0);
const sf::Color GRAY (127, 127, 127);
const sf::Color RED  (255,   0,   0,   0);
const sf::Color GREEN(  0, 255,   0,   0);
const sf::Color BLUE (  0,   0, 255,   0);
const sf::Color WHITE(255, 255, 255);

TEST(TestColorMap, FactoryWrongType)
{
    EXPECT_THROW_MATCH(auto ret = ColorMap::Build((ColorMapType)999, sf::Color(0, 0, 0), sf::Color(0, 0, 0)),
                       std::runtime_error, "unknown color map");
}

TEST(TestColorMap, InterpolationColorMap)
{
    /* various errors in input */
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK }, { 1.0 }),
                       std::runtime_error, "at least two colors needed in a colormap");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK }, { 1.0, 2.0 }),
                       std::runtime_error, "number of boundaries and number of colors differ");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, WHITE }, { 1.0, 2.0 }),
                       std::runtime_error, "first colormap boundary must be 0.0");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, WHITE }, { 0.0, 2.0 }),
                       std::runtime_error, "last colormap boundary must be 1.0");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, GRAY, GRAY, WHITE }, { 0.0, 0.7, 0.3, 1.0 }),
                       std::runtime_error, "boundaries must be ascending");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, GRAY, GRAY, WHITE }, { 0.0, 0.5, 0.5, 1.0 }),
                       std::runtime_error, "boundaries must be ascending");

    /* domain bounds */
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, WHITE }, { 0.0, 1.0 }).Map( { -1e-9 }),
                       std::runtime_error, "input value outside of colormap domain");
    EXPECT_THROW_MATCH(InterpolationColorMap({ BLACK, WHITE }, { 0.0, 1.0 }).Map( { 1.0+1e-9 }),
                       std::runtime_error, "input value outside of colormap domain");
    EXPECT_NO_THROW(InterpolationColorMap({ BLACK, WHITE }, { 0.0, 1.0 }).Map( { 0.0 }));
    EXPECT_NO_THROW(InterpolationColorMap({ BLACK, WHITE }, { 0.0, 1.0 }).Map( { 1.0 }));

    { /* 2-color interpolation */
        InterpolationColorMap map({BLACK, WHITE}, {0.0, 1.0});

        EXPECT_EQ(map.Map({}).size(), 0); /* zero input, zero output, no errors */
        EXPECT_EQ(map.Gradient(0).size(), 0);

        { /* 1-length inputs, test simple interpolation */
            constexpr std::size_t steps = 256;
            for (std::size_t i = 0; i < steps; i++) {
                double input = (double) i / (double) (steps - 1);
                auto out = map.Map({input});
                EXPECT_EQ(out.size(), 4);
                EXPECT_EQ(out[0], i);
                EXPECT_EQ(out[1], i);
                EXPECT_EQ(out[2], i);
                EXPECT_EQ(out[3], 255);
            }
        }

        { /* n-length inputs */
            constexpr std::size_t steps = 256;
            std::vector<double> input;
            for (std::size_t i = 0; i < steps; i++) {
                input.push_back((double) i / (double) (steps - 1));
            }
            auto out = map.Map(input);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], i);
                EXPECT_EQ(out[i * 4 + 1], i);
                EXPECT_EQ(out[i * 4 + 2], i);
                EXPECT_EQ(out[i * 4 + 3], 255);
            }
        }

        { /* gradient */
            constexpr std::size_t steps = 100;
            auto out = map.Gradient(steps);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                double ev = (double)i / (double)(steps-1);
                uint8_t color = (uint8_t)std::round(ev * 255.0);
                EXPECT_EQ(out[i * 4 + 0], color);
                EXPECT_EQ(out[i * 4 + 1], color);
                EXPECT_EQ(out[i * 4 + 2], color);
                EXPECT_EQ(out[i * 4 + 3], 255);
            }
        }
    }

    { /* n-color interpolation */
        InterpolationColorMap map({BLACK, RED, GREEN, BLUE, BLACK}, {0.0, 0.25, 0.5, 0.75, 1.0});

        EXPECT_EQ(map.Map({}).size(), 0); /* zero input, zero output, no errors */
        EXPECT_EQ(map.Gradient(0).size(), 0);

        constexpr std::size_t steps = 9;
        std::vector<uint8_t> exp_r { 0, 128, 255, 128, 0, 0, 0, 0, 0 };
        std::vector<uint8_t> exp_g { 0, 0, 0, 128, 255, 128, 0, 0, 0 };
        std::vector<uint8_t> exp_b { 0, 0, 0, 0, 0, 128, 255, 128, 0 };
        std::vector<uint8_t> exp_a { 255, 128, 0, 0, 0, 0, 0, 128, 255 };

        { /* 1-length inputs, test simple interpolation */
            for (std::size_t i = 0; i < steps; i++) {
                double input = (double) i / (double) (steps - 1);
                auto out = map.Map({input});
                EXPECT_EQ(out.size(), 4);
                EXPECT_EQ(out[0], exp_r[i]);
                EXPECT_EQ(out[1], exp_g[i]);
                EXPECT_EQ(out[2], exp_b[i]);
                EXPECT_EQ(out[3], exp_a[i]);
            }
        }

        { /* n-length inputs */
            std::vector<double> input;
            for (std::size_t i = 0; i < steps; i++) {
                input.push_back((double) i / (double) (steps - 1));
            }
            auto out = map.Map(input);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], exp_r[i]);
                EXPECT_EQ(out[i * 4 + 1], exp_g[i]);
                EXPECT_EQ(out[i * 4 + 2], exp_b[i]);
                EXPECT_EQ(out[i * 4 + 3], exp_a[i]);
            }
        }

        { /* gradient */
            auto out = map.Gradient(steps);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], exp_r[i]);
                EXPECT_EQ(out[i * 4 + 1], exp_g[i]);
                EXPECT_EQ(out[i * 4 + 2], exp_b[i]);
                EXPECT_EQ(out[i * 4 + 3], exp_a[i]);
            }
        }
    }
}

TEST(TestColorMap, TwoColorMap)
{
    { /* 2-color interpolation */
        TwoColorMap map(BLACK, WHITE);

        EXPECT_EQ(map.Map({}).size(), 0); /* zero input, zero output, no errors */
        EXPECT_EQ(map.Gradient(0).size(), 0);

        { /* 1-length inputs, test simple interpolation */
            constexpr std::size_t steps = 256;
            for (std::size_t i = 0; i < steps; i++) {
                double input = (double) i / (double) (steps - 1);
                auto out = map.Map({input});
                EXPECT_EQ(out.size(), 4);
                EXPECT_EQ(out[0], i);
                EXPECT_EQ(out[1], i);
                EXPECT_EQ(out[2], i);
                EXPECT_EQ(out[3], 255);
            }
        }

        { /* n-length inputs */
            constexpr std::size_t steps = 256;
            std::vector<double> input;
            for (std::size_t i = 0; i < steps; i++) {
                input.push_back((double) i / (double) (steps - 1));
            }
            auto out = map.Map(input);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], i);
                EXPECT_EQ(out[i * 4 + 1], i);
                EXPECT_EQ(out[i * 4 + 2], i);
                EXPECT_EQ(out[i * 4 + 3], 255);
            }
        }

        { /* gradient */
            constexpr std::size_t steps = 100;
            auto out = map.Gradient(steps);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                double ev = (double)i / (double)(steps-1);
                uint8_t color = (uint8_t)std::round(ev * 255.0);
                EXPECT_EQ(out[i * 4 + 0], color);
                EXPECT_EQ(out[i * 4 + 1], color);
                EXPECT_EQ(out[i * 4 + 2], color);
                EXPECT_EQ(out[i * 4 + 3], 255);
            }
        }
    }
}

TEST(TestColorMap, ThreeColorMap)
{
    { /* 3-color interpolation */
        ThreeColorMap map(RED, GREEN, BLUE);

        EXPECT_EQ(map.Map({}).size(), 0); /* zero input, zero output, no errors */
        EXPECT_EQ(map.Gradient(0).size(), 0);

        constexpr std::size_t steps = 5;
        std::vector<uint8_t> exp_r { 255, 128, 0, 0, 0,  };
        std::vector<uint8_t> exp_g { 0, 128, 255, 128, 0 };
        std::vector<uint8_t> exp_b { 0, 0, 0, 128, 255 };
        std::vector<uint8_t> exp_a { 0, 0, 0, 0, 0 };

        { /* 1-length inputs, test simple interpolation */
            for (std::size_t i = 0; i < steps; i++) {
                double input = (double) i / (double) (steps - 1);
                auto out = map.Map({input});
                EXPECT_EQ(out.size(), 4);
                EXPECT_EQ(out[0], exp_r[i]);
                EXPECT_EQ(out[1], exp_g[i]);
                EXPECT_EQ(out[2], exp_b[i]);
                EXPECT_EQ(out[3], exp_a[i]);
            }
        }

        { /* n-length inputs */
            std::vector<double> input;
            for (std::size_t i = 0; i < steps; i++) {
                input.push_back((double) i / (double) (steps - 1));
            }
            auto out = map.Map(input);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], exp_r[i]);
                EXPECT_EQ(out[i * 4 + 1], exp_g[i]);
                EXPECT_EQ(out[i * 4 + 2], exp_b[i]);
                EXPECT_EQ(out[i * 4 + 3], exp_a[i]);
            }
        }

        { /* gradient */
            auto out = map.Gradient(steps);
            EXPECT_EQ(out.size(), 4 * steps);
            for (std::size_t i = 0; i < steps; i++) {
                EXPECT_EQ(out[i * 4 + 0], exp_r[i]);
                EXPECT_EQ(out[i * 4 + 1], exp_g[i]);
                EXPECT_EQ(out[i * 4 + 2], exp_b[i]);
                EXPECT_EQ(out[i * 4 + 3], exp_a[i]);
            }
        }
    }
}

TEST(TestColorMap, CopyColorMap)
{
    { /* 3-color interpolation */
        auto map = ThreeColorMap(RED, GREEN, BLUE).Copy();

        EXPECT_EQ(map->Map({}).size(), 0); /* zero input, zero output, no errors */
        EXPECT_EQ(map->Gradient(0).size(), 0);

        constexpr std::size_t steps = 5;
        std::vector<uint8_t> exp_r { 255, 128, 0, 0, 0,  };
        std::vector<uint8_t> exp_g { 0, 128, 255, 128, 0 };
        std::vector<uint8_t> exp_b { 0, 0, 0, 128, 255 };
        std::vector<uint8_t> exp_a { 0, 0, 0, 0, 0 };

        auto out = map->Gradient(steps);
        EXPECT_EQ(out.size(), 4 * steps);
        for (std::size_t i = 0; i < steps; i++) {
            EXPECT_EQ(out[i * 4 + 0], exp_r[i]);
            EXPECT_EQ(out[i * 4 + 1], exp_g[i]);
            EXPECT_EQ(out[i * 4 + 2], exp_b[i]);
            EXPECT_EQ(out[i * 4 + 3], exp_a[i]);
        }
    }
}
