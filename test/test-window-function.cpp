/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/window-function.hpp"
#include <list>

static std::vector<WindowFunctionType> ALL_WF_TYPES {
    WindowFunctionType::kBlackman,
    WindowFunctionType::kHamming,
    WindowFunctionType::kHann,
    WindowFunctionType::kNuttall,
    WindowFunctionType::kNone
};

static constexpr double EPSILON = 1e-7;

void run_tests(const std::list<std::vector<double>>& tests, WindowFunctionType type)
{
    for (const auto& test : tests) {
        /* create window */
        auto win = WindowFunction::FromType(type, test.size());
        /* create identity input */
        ComplexWindow input(test.size());
        for (auto& i : input) { i = 1.0; }
        /* apply */
        auto out = win->Apply(input);
        EXPECT_EQ(out.size(), test.size());
        /* check */
        for (size_t i = 0; i < test.size(); i++) {
            EXPECT_LE(std::abs(out[i] - test[i]), EPSILON);
        }
    }
}

TEST(TestWindowFunction, FactoryWrongType)
{
    EXPECT_THROW_MATCH(auto ret = WindowFunction::FromType((WindowFunctionType)999, 10),
                       std::runtime_error, "unknown window function");
}

TEST(TestWindowFunction, TypeBlackman)
{
    /* np.blackman */
    std::list<std::vector<double>> tests;
    tests.push_back( { 1.0 });
    tests.push_back( { -1.38777878e-17, -1.38777878e-17 });
    tests.push_back( { -1.38777878e-17,  1.00000000e+00, -1.38777878e-17 });
    tests.push_back( { -1.38777878e-17,  3.40000000e-01,  1.00000000e+00,  3.40000000e-01,
                       -1.38777878e-17 });
    tests.push_back({ -1.38777878e-17,  5.08696327e-02,  2.58000502e-01,  6.30000000e-01,
                      9.51129866e-01,  9.51129866e-01,  6.30000000e-01,  2.58000502e-01,
                      5.08696327e-02, -1.38777878e-17 });
    tests.push_back({ -1.38777878e-17,  6.31911916e-03,  2.69872981e-02,  6.64466094e-02,
                      1.30000000e-01,  2.21308445e-01,  3.40000000e-01,  4.80127490e-01,
                      6.30000000e-01,  7.73553391e-01,  8.93012702e-01,  9.72244945e-01,
                      1.00000000e+00,  9.72244945e-01,  8.93012702e-01,  7.73553391e-01,
                      6.30000000e-01,  4.80127490e-01,  3.40000000e-01,  2.21308445e-01,
                      1.30000000e-01,  6.64466094e-02,  2.69872981e-02,  6.31911916e-03,
                      -1.38777878e-17 });

    run_tests(tests, WindowFunctionType::kBlackman);
}

TEST(TestWindowFunction, TypeHamming)
{
    /* np.hamming */
    std::list<std::vector<double>> tests;
    tests.push_back( { 1.0 });
    tests.push_back( { 0.08, 0.08 });
    tests.push_back( { 0.08, 1.  , 0.08 });
    tests.push_back( { 0.08, 0.54, 1.  , 0.54, 0.08 });
    tests.push_back( { 0.08      , 0.18761956, 0.46012184, 0.77      , 0.97225861,
                       0.97225861, 0.77      , 0.46012184, 0.18761956, 0.08 });
    tests.push_back( { 0.08      , 0.09567412, 0.14162831, 0.21473088, 0.31      ,
                       0.42094324, 0.54      , 0.65905676, 0.77      , 0.86526912,
                       0.93837169, 0.98432588, 1.        , 0.98432588, 0.93837169,
                       0.86526912, 0.77      , 0.65905676, 0.54      , 0.42094324,
                       0.31      , 0.21473088, 0.14162831, 0.09567412, 0.08 });

    run_tests(tests, WindowFunctionType::kHamming);
}

TEST(TestWindowFunction, TypeHann)
{
    /* np.hanning */
    std::list<std::vector<double>> tests;
    tests.push_back( { 1.0 });
    tests.push_back( { 0., 0. });
    tests.push_back( { 0., 1., 0. });
    tests.push_back( { 0. , 0.5, 1. , 0.5, 0. });
    tests.push_back( { 0.        , 0.11697778, 0.41317591, 0.75      , 0.96984631,
                       0.96984631, 0.75      , 0.41317591, 0.11697778, 0. });
    tests.push_back( { 0.        , 0.01703709, 0.0669873 , 0.14644661, 0.25      ,
                       0.37059048, 0.5       , 0.62940952, 0.75      , 0.85355339,
                       0.9330127 , 0.98296291, 1.        , 0.98296291, 0.9330127 ,
                       0.85355339, 0.75      , 0.62940952, 0.5       , 0.37059048,
                       0.25      , 0.14644661, 0.0669873 , 0.01703709, 0. });

    run_tests(tests, WindowFunctionType::kHann);
}

TEST(TestWindowFunction, TypeNuttall)
{
    /* scipy.signal.nuttall */
    /* NOTE: Nuttall4c from https://holometer.fnal.gov/GH_FFT.pdf */
    std::list<std::vector<double>> tests;
    tests.push_back( { 1. });
    tests.push_back( { 0.0003628, 0.0003628 });
    tests.push_back( { 3.628e-04, 1.000e+00, 3.628e-04 });
    tests.push_back( { 3.628000e-04, 2.269824e-01, 1.000000e+00, 2.269824e-01,
                       3.628000e-04 });
    tests.push_back( { 3.62800000e-04, 1.78909987e-02, 1.55596126e-01, 5.29229800e-01,
                       9.33220225e-01, 9.33220225e-01, 5.29229800e-01, 1.55596126e-01,
                       1.78909987e-02, 3.62800000e-04 });
    tests.push_back( { 3.62800000e-04, 1.84696229e-03, 8.24150804e-03, 2.52055665e-02,
                       6.13345000e-02, 1.26199203e-01, 2.26982400e-01, 3.64367322e-01,
                       5.29229800e-01, 7.01958233e-01, 8.55521792e-01, 9.61914112e-01,
                       1.00000000e+00, 9.61914112e-01, 8.55521792e-01, 7.01958233e-01,
                       5.29229800e-01, 3.64367322e-01, 2.26982400e-01, 1.26199203e-01,
                       6.13345000e-02, 2.52055665e-02, 8.24150804e-03, 1.84696229e-03,
                       3.62800000e-04 });

    run_tests(tests, WindowFunctionType::kNuttall);
}

TEST(TestWindowFunction, TypeNone)
{
    EXPECT_EQ(WindowFunction::FromType(WindowFunctionType::kNone, 1), nullptr);
    EXPECT_EQ(WindowFunction::FromType(WindowFunctionType::kNone, 10), nullptr);
}