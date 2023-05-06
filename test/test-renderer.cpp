/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/renderer.hpp"
#include <X11/Xlib.h>

class ExposedRenderer : public Renderer
{
public:
    using AxisTick = Renderer::AxisTick;

    ExposedRenderer(const Configuration& conf, std::size_t fft_count)
        : Renderer(conf, fft_count) { };

    static std::string EValueToShortString(double value, int scale, const std::string& unit)
    {
        return ValueToShortString(value, scale, unit);
    }

    static std::list<AxisTick> EGetLinearTicks(double v_min, double v_max, const std::string& v_unit,
                                               unsigned int num_ticks)
    {
        return Renderer::GetLinearTicks(v_min, v_max, v_unit, num_ticks);
    }

    std::list<Renderer::AxisTick> EGetNiceTicks(double v_min, double v_max, const std::string& v_unit,
                                      unsigned int length_px, unsigned int min_tick_length_px, bool rotated)
    {
        return GetNiceTicks(v_min, v_max, v_unit, length_px, min_tick_length_px, rotated);
    }
};

TEST(TestRenderer, ValueToShortString)
{
    /* unit prefix */
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e20, -20, ""), "100000000T");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e12, -12, ""), "1T");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e9, -9, ""), "1G");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e6, -6, ""), "1M");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e3, -3, ""), "1k");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1, 0, ""), "1");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-3, 3, ""), "1m");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-6, 6, ""), "1u");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-9, 9, ""), "1n");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-12, 12, ""), "1p");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-20, 20, ""), "0.00000001p");

    /* rounding */
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 9, "V"), "49000nV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 8, "V"), "49.00uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 7, "V"), "49.0uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 6, "V"), "49uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 5, "V"), "0.05mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 4, "V"), "0.0mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 3, "V"), "0mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 2, "V"), "0.00V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 1, "V"), "0.0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, 0, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, -1, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, -2, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000049, -3, "V"), "0kV");

    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 9, "V"), "44000nV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 8, "V"), "44.00uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 7, "V"), "44.0uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 6, "V"), "44uV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 5, "V"), "0.04mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 4, "V"), "0.0mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 3, "V"), "0mV");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 2, "V"), "0.00V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 1, "V"), "0.0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, 0, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, -1, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, -2, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(0.000044, -3, "V"), "0kV");

    /* round to zero */
    EXPECT_EQ(ExposedRenderer::EValueToShortString(1e-10, 0, "V"), "0V");
    EXPECT_EQ(ExposedRenderer::EValueToShortString(-1e-10, 0, "V"), "0V");
}

TEST(TestRenderer, GetLinearTicks)
{
    constexpr double epsilon = 1e-9;

    EXPECT_THROW_MATCH(ExposedRenderer::EGetLinearTicks(0, 0, "", 0),
                       std::runtime_error, "requires at least two ticks");
    EXPECT_THROW_MATCH(ExposedRenderer::EGetLinearTicks(0, 0, "", 1),
                       std::runtime_error, "requires at least two ticks");

    EXPECT_THROW_MATCH(ExposedRenderer::EGetLinearTicks(1.0, 1.0, "", 2),
                       std::runtime_error, "minimum and maximum values are not in order");
    EXPECT_THROW_MATCH(ExposedRenderer::EGetLinearTicks(1.0+1e-9, 1.0, "", 2),
                       std::runtime_error, "minimum and maximum values are not in order");
    EXPECT_NO_THROW(ExposedRenderer::EGetLinearTicks(1.0-1e-9, 1.0, "", 2));

    using Test = struct { double l; double h; std::string unit; std::size_t num; std::vector<ExposedRenderer::AxisTick> ticks; };
    std::vector<Test> tests {
            {0.0, 3000.0, "V", 5, { {0.0/4.0, "0V"}, {1.0/4.0, "750V"}, {2.0/4.0, "1500V"}, {3.0/4.0, "2250V"}, {4.0/4.0, "3000V"} } },
            {0.0, 3000.0, "V", 4, { {0.0/3.0, "0kV"}, {1.0/3.0, "1kV"}, {2.0/3.0, "2kV"}, {3.0/3.0, "3kV"} } },

            {-5.0, 5.0, "V", 11, { {0.0, "-5V"}, {0.1, "-4V"}, {0.2, "-3V"}, {0.3, "-2V"}, {0.4, "-1V"}, {0.5, "0V"}, {0.6, "1V"}, {0.7, "2V"}, {0.8, "3V"}, {0.9, "4V"}, {1.0, "5V"} } },
            {-50.0, 50.0, "V", 11, { {0.0, "-50V"}, {0.1, "-40V"}, {0.2, "-30V"}, {0.3, "-20V"}, {0.4, "-10V"}, {0.5, "0V"}, {0.6, "10V"}, {0.7, "20V"}, {0.8, "30V"}, {0.9, "40V"}, {1.0, "50V"} } },

            {0.0, 2000.0,   "V", 4, { {0.0/3.0, "0V"},     {1.0/3.0, "667V"},   {2.0/3.0, "1333V"},  {3.0/3.0, "2000V"} } },
            {0.0,  200.0,   "V", 4, { {0.0/3.0, "0V"},     {1.0/3.0, "67V"},    {2.0/3.0, "133V"},   {3.0/3.0, "200V"} } },
            {0.0,   20.0,   "V", 4, { {0.0/3.0, "0.0V"},   {1.0/3.0, "6.7V"},   {2.0/3.0, "13.3V"},  {3.0/3.0, "20.0V"} } },
            {0.0,    2.0,   "V", 4, { {0.0/3.0, "0.00V"},  {1.0/3.0, "0.67V"},  {2.0/3.0, "1.33V"},  {3.0/3.0, "2.00V"} } },
            {0.0,    0.2,   "V", 4, { {0.0/3.0, "0mV"},    {1.0/3.0, "67mV"},   {2.0/3.0, "133mV"},  {3.0/3.0, "200mV"} } },
            {0.0,    0.02,  "V", 4, { {0.0/3.0, "0.0mV"},  {1.0/3.0, "6.7mV"},  {2.0/3.0, "13.3mV"}, {3.0/3.0, "20.0mV"} } },
            {0.0,    0.002, "V", 4, { {0.0/3.0, "0.00mV"}, {1.0/3.0, "0.67mV"}, {2.0/3.0, "1.33mV"}, {3.0/3.0, "2.00mV"} } },
    };

    for (auto& test : tests) {
        auto out = ExposedRenderer::EGetLinearTicks(test.l, test.h, test.unit, test.num);
        EXPECT_EQ(out.size(), test.num);
        EXPECT_EQ(out.size(), test.ticks.size());

        auto it = out.begin();
        for (std::size_t i=0; i<test.num; i++, it++) {
            EXPECT_LE(std::get<0>(*it) - std::get<0>(test.ticks[i]), epsilon);
            EXPECT_EQ(std::get<1>(*it), std::get<1>(test.ticks[i]));
        }
    }
}

TEST(TestRenderer, GetNiceTicks)
{
    { /* skip if Xorg not started */
        auto display = XOpenDisplay(nullptr);
        if (display == nullptr) {
            GTEST_SKIP();
        } else {
            XCloseDisplay(display);
        }
    }

    constexpr double epsilon = 1e-9;

    /* configuration */
    const char *args[] { "program", "-l" };
    auto[conf, rc, exit] = Configuration::Build(2, args);
    EXPECT_EQ(rc, 0);
    EXPECT_FALSE(exit);

    /* renderer */
    EXPECT_THROW_MATCH(ExposedRenderer(conf.GetForLive(), 0),
                       std::runtime_error, "positive number of FFT windows required by renderer");
    ExposedRenderer renderer(conf.GetForLive(), 128);

    EXPECT_THROW_MATCH(renderer.EGetNiceTicks(1.0, 1.0, "", 100, 30, false),
                       std::runtime_error, "minimum and maximum values are not in order");
    EXPECT_THROW_MATCH(renderer.EGetNiceTicks(1.0+1e-9, 1.0, "", 100, 30, false),
                       std::runtime_error, "minimum and maximum values are not in order");

    EXPECT_THROW_MATCH(renderer.EGetNiceTicks(0.0, 1.0, "", 0, 30, false),
                       std::runtime_error, "length in pixels must be positive");
    EXPECT_THROW_MATCH(renderer.EGetNiceTicks(0.0, 1.0, "", 100, 0, false),
                       std::runtime_error, "minimum tick length in pixels must be positive");

    using Test = struct { double l; double h; std::string unit; std::size_t len; std::size_t min_size; bool rot; std::vector<ExposedRenderer::AxisTick> ticks; };
    std::vector<Test> tests {
            /* very small minimum tick length, limited by text width */
            {0.0, 3000.0, "", 200, 1, false, { {0.0/7.0, "0"}, {1.0/6.0, "500"}, {2.0/6.0, "1000"}, {3.0/6.0, "1500"}, {4.0/6.0, "2000"}, {5.0/6.0, "2500"}, {6.0/6.0, "3000"} } },
            {0.0, 3000.0, "V", 200, 1, false, { {0.0/3.0, "0kV"}, {1.0/3.0, "1kV"}, {2.0/3.0, "2kV"}, {3.0/3.0, "3kV"} } },

            /* increase tick limit */
            {0.0, 3000.0, "V", 200, 30, false, { {0.0/3.0, "0kV"}, {1.0/3.0, "1kV"}, {2.0/3.0, "2kV"}, {3.0/3.0, "3kV"} } },
            {0.0, 3000.0, "V", 200, 90, false, { {0.0/2.0, "0V"}, {1.0/2.0, "1500V"}, {2.0/2.0, "3000V"} } },
            {0.0, 3000.0, "V", 200, 120, false, { {0.0, "0kV"}, {2.0/3.0, "2kV"} } },

            /* first nice value */
            {-300.0, 3000.0, "V", 200, 120, false, { {1.0/10.0, "0kV"}, {7.0/10.0, "2kV"} } },

            /* Issue #23 */
            {1.0, 100.0, "", 3, 50, false, { {0.02, "1"} } },

            /* Issue #26 */
            {-70.0, -20.0, "dBFS", 907, 50, false, { {0.02, "-69dBFS"}, {0.08, "-66dBFS"}, {0.14, "-63dBFS"}, {0.2, "-60dBFS"}, {0.26, "-57dBFS"}, {0.32, "-54dBFS"}, {0.38, "-51dBFS"}, {0.44, "-48dBFS"}, {0.50, "-45dBFS"}, {0.56, "-42dBFS"}, {0.62, "-39dBFS"}, {0.68, "-36dBFS"}, {0.74, "-33dBFS"}, {0.80, "-30dBFS"}, {0.86, "-27dBFS"}, {0.92, "-24dBFS"}, {0.98, "-21dBFS"} } },
            {-70.0, 0.0, "dBFS", 907, 50, false, { {0.0/14.0, "-70dBFS"}, {1.0/14.0, "-65dBFS"}, {2.0/14.0, "-60dBFS"}, {3.0/14.0, "-55dBFS"}, {4.0/14.0, "-50dBFS"}, {5.0/14.0, "-45dBFS"}, {6.0/14.0, "-40dBFS"}, {7.0/14.0, "-35dBFS"}, {8.0/14.0, "-30dBFS"}, {9.0/14.0, "-25dBFS"}, {10.0/14.0, "-20dBFS"}, {11.0/14.0, "-15dBFS"}, {12.0/14.0, "-10dBFS"}, {13.0/14.0, "-5dBFS"}, {14.0/14.0, "0dBFS"} } },
    };

    for (auto& test : tests) {
        auto out = renderer.EGetNiceTicks(test.l, test.h, test.unit, test.len, test.min_size, test.rot);
        EXPECT_EQ(out.size(), test.ticks.size());

        auto it = out.begin();
        for (std::size_t i=0; i<test.ticks.size(); i++, it++) {
            EXPECT_LE(std::get<0>(*it) - std::get<0>(test.ticks[i]), epsilon);
            EXPECT_EQ(std::get<1>(*it), std::get<1>(test.ticks[i]));
        }
    }
}
