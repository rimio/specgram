/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/input-parser.hpp"
#include <random>

const std::vector<DataType> ALL_DATA_TYPES {
        DataType::kSignedInt8,
        DataType::kSignedInt16,
        DataType::kSignedInt32,
        DataType::kSignedInt64,

        DataType::kUnsignedInt8,
        DataType::kUnsignedInt16,
        DataType::kUnsignedInt32,
        DataType::kUnsignedInt64,

        DataType::kFloat32,
        DataType::kFloat64
};

TEST(TestInputParser, IsSigned)
{
    for (bool is_complex : { false, true }) {
        /* direct instantiate */
        EXPECT_FALSE(IntegerInputParser<std::uint8_t>(1.0, is_complex).IsSigned());
        EXPECT_FALSE(IntegerInputParser<std::uint16_t>(1.0, is_complex).IsSigned());
        EXPECT_FALSE(IntegerInputParser<std::uint32_t>(1.0, is_complex).IsSigned());
        EXPECT_FALSE(IntegerInputParser<std::uint64_t>(1.0, is_complex).IsSigned());

        EXPECT_TRUE(IntegerInputParser<std::int8_t>(1.0, is_complex).IsSigned());
        EXPECT_TRUE(IntegerInputParser<std::int16_t>(1.0, is_complex).IsSigned());
        EXPECT_TRUE(IntegerInputParser<std::int32_t>(1.0, is_complex).IsSigned());
        EXPECT_TRUE(IntegerInputParser<std::int64_t>(1.0, is_complex).IsSigned());

        EXPECT_TRUE(FloatInputParser<float>(1.0, is_complex).IsSigned());
        EXPECT_TRUE(FloatInputParser<double>(1.0, is_complex).IsSigned());

        /* factory */
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt8, 1.0, is_complex)->IsSigned());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt16, 1.0, is_complex)->IsSigned());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt32, 1.0, is_complex)->IsSigned());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt64, 1.0, is_complex)->IsSigned());

        EXPECT_TRUE(InputParser::FromDataType(DataType::kSignedInt8, 1.0, is_complex)->IsSigned());
        EXPECT_TRUE(InputParser::FromDataType(DataType::kSignedInt16, 1.0, is_complex)->IsSigned());
        EXPECT_TRUE(InputParser::FromDataType(DataType::kSignedInt32, 1.0, is_complex)->IsSigned());
        EXPECT_TRUE(InputParser::FromDataType(DataType::kSignedInt64, 1.0, is_complex)->IsSigned());

        EXPECT_TRUE(InputParser::FromDataType(DataType::kFloat32, 1.0, is_complex)->IsSigned());
        EXPECT_TRUE(InputParser::FromDataType(DataType::kFloat64, 1.0, is_complex)->IsSigned());
    }
}

TEST(TestInputParser, IsFloatingPoint)
{
    for (bool is_complex : { false, true }) {
        /* direct instantiate */
        EXPECT_FALSE(IntegerInputParser<std::uint8_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::uint16_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::uint32_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::uint64_t>(1.0, is_complex).IsFloatingPoint());

        EXPECT_FALSE(IntegerInputParser<std::int8_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::int16_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::int32_t>(1.0, is_complex).IsFloatingPoint());
        EXPECT_FALSE(IntegerInputParser<std::int64_t>(1.0, is_complex).IsFloatingPoint());

        EXPECT_TRUE(FloatInputParser<float>(1.0, is_complex).IsFloatingPoint());
        EXPECT_TRUE(FloatInputParser<double>(1.0, is_complex).IsFloatingPoint());

        /* factory */
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt8, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt16, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt32, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kUnsignedInt64, 1.0, is_complex)->IsFloatingPoint());

        EXPECT_FALSE(InputParser::FromDataType(DataType::kSignedInt8, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kSignedInt16, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kSignedInt32, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_FALSE(InputParser::FromDataType(DataType::kSignedInt64, 1.0, is_complex)->IsFloatingPoint());

        EXPECT_TRUE(InputParser::FromDataType(DataType::kFloat32, 1.0, is_complex)->IsFloatingPoint());
        EXPECT_TRUE(InputParser::FromDataType(DataType::kFloat64, 1.0, is_complex)->IsFloatingPoint());
    }
}

TEST(TestInputParser, IsComplex)
{
    for (bool is_complex : { false, true }) {
        /* direct instantiate */
        EXPECT_EQ(is_complex, IntegerInputParser<std::uint8_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::uint16_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::uint32_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::uint64_t>(1.0, is_complex).IsComplex());

        EXPECT_EQ(is_complex, IntegerInputParser<std::int8_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::int16_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::int32_t>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, IntegerInputParser<std::int64_t>(1.0, is_complex).IsComplex());

        EXPECT_EQ(is_complex, FloatInputParser<float>(1.0, is_complex).IsComplex());
        EXPECT_EQ(is_complex, FloatInputParser<double>(1.0, is_complex).IsComplex());

        /* factory */
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kUnsignedInt8, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kUnsignedInt16, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kUnsignedInt32, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kUnsignedInt64, 1.0, is_complex)->IsComplex());

        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kSignedInt8, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kSignedInt16, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kSignedInt32, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kSignedInt64, 1.0, is_complex)->IsComplex());

        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kFloat32, 1.0, is_complex)->IsComplex());
        EXPECT_EQ(is_complex, InputParser::FromDataType(DataType::kFloat64, 1.0, is_complex)->IsComplex());
    }
}

TEST(TestInputParser, GetDataTypeSize)
{
    for (bool is_complex : { false, true }) {
        /* direct instantiate */
        EXPECT_EQ(1 * (is_complex ? 2 : 1), IntegerInputParser<std::uint8_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(2 * (is_complex ? 2 : 1), IntegerInputParser<std::uint16_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(4 * (is_complex ? 2 : 1), IntegerInputParser<std::uint32_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), IntegerInputParser<std::uint64_t>(1.0, is_complex).GetDataTypeSize());

        EXPECT_EQ(1 * (is_complex ? 2 : 1), IntegerInputParser<std::int8_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(2 * (is_complex ? 2 : 1), IntegerInputParser<std::int16_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(4 * (is_complex ? 2 : 1), IntegerInputParser<std::int32_t>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), IntegerInputParser<std::int64_t>(1.0, is_complex).GetDataTypeSize());

        EXPECT_EQ(4 * (is_complex ? 2 : 1), FloatInputParser<float>(1.0, is_complex).GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), FloatInputParser<double>(1.0, is_complex).GetDataTypeSize());

        /* factory */
        EXPECT_EQ(1 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kUnsignedInt8, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(2 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kUnsignedInt16, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(4 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kUnsignedInt32, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kUnsignedInt64, 1.0, is_complex)->GetDataTypeSize());

        EXPECT_EQ(1 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kSignedInt8, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(2 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kSignedInt16, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(4 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kSignedInt32, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kSignedInt64, 1.0, is_complex)->GetDataTypeSize());

        EXPECT_EQ(4 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kFloat32, 1.0, is_complex)->GetDataTypeSize());
        EXPECT_EQ(8 * (is_complex ? 2 : 1), InputParser::FromDataType(DataType::kFloat64, 1.0, is_complex)->GetDataTypeSize());
    }
}

TEST(TestInputParser, ParseBlockWrongSize)
{
    for (bool is_complex : { false, true }) {
        for (auto dt : ALL_DATA_TYPES) {
            auto parser = InputParser::FromDataType(dt, 1.0, is_complex);
            for (std::size_t bs = 0; bs < 512; bs++) {
                if ((bs % parser->GetDataTypeSize()) == 0) {
                    EXPECT_NO_THROW(parser->ParseBlock(std::vector<char>(bs)));
                } else {
                    EXPECT_THROW_MATCH(parser->ParseBlock(std::vector<char>(bs)),
                                       std::runtime_error, "block size must be a multiple of sizeof(datatype)");
                }
            }
        }
    }
}

using Buffer = std::vector<char>;
using Result = std::vector<Complex>;

std::tuple<Buffer, Result> make_test(DataType dt, bool is_complex, std::size_t length = 20480)
{
    Buffer buf;
    Result res;

    res.resize(length);
    std::size_t factor = length * (is_complex ? 2 : 1);

    std::random_device rd;
    std::default_random_engine re(rd());

    switch (dt) {
        case DataType::kSignedInt8:
            {
                buf.resize(factor * 1);
                std::uniform_int_distribution<std::int8_t> ud;
                auto mem = (std::int8_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::int8_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::int8_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::int8_t>::max());
                    }
                }
            }
            break;

        case DataType::kUnsignedInt8:
            {
                buf.resize(factor * 1);
                std::uniform_int_distribution<std::uint8_t> ud;
                auto mem = (std::uint8_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::uint8_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::uint8_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::uint8_t>::max());
                    }
                }
            }
            break;

        case DataType::kSignedInt16:
            {
                buf.resize(factor * 2);
                std::uniform_int_distribution<std::int16_t> ud;
                auto mem = (std::int16_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::int16_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::int16_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::int16_t>::max());
                    }
                }
            }
            break;

        case DataType::kUnsignedInt16:
            {
                buf.resize(factor * 2);
                std::uniform_int_distribution<std::uint16_t> ud;
                auto mem = (std::uint16_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::uint16_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::uint16_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::uint16_t>::max());
                    }
                }
            }
            break;

        case DataType::kSignedInt32:
            {
                buf.resize(factor * 4);
                std::uniform_int_distribution<std::int32_t> ud;
                auto mem = (std::int32_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::int32_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::int32_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::int32_t>::max());
                    }
                }
            }
            break;

        case DataType::kUnsignedInt32:
            {
                buf.resize(factor * 4);
                std::uniform_int_distribution<std::uint32_t> ud;
                auto mem = (std::uint32_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::uint32_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::uint32_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::uint32_t>::max());
                    }
                }
            }
            break;

        case DataType::kSignedInt64:
            {
                buf.resize(factor * 8);
                std::uniform_int_distribution<std::int64_t> ud;
                auto mem = (std::int64_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::int64_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::int64_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::int64_t>::max());
                    }
                }
            }
            break;

        case DataType::kUnsignedInt64:
            {
                buf.resize(factor * 8);
                std::uniform_int_distribution<std::uint64_t> ud;
                auto mem = (std::uint64_t *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = ud(re);
                        mem[i*2+1] = ud(re);
                        res[i] = std::complex<double>((double)mem[i*2+0] / (double)std::numeric_limits<std::uint64_t>::max(),
                                                      (double)mem[i*2+1] / (double)std::numeric_limits<std::uint64_t>::max());
                    } else {
                        mem[i] = ud(re);
                        res[i] = std::complex<double>((double)mem[i] / (double)std::numeric_limits<std::uint64_t>::max());
                    }
                }
            }
            break;

        case DataType::kFloat32:
            {
                buf.resize(factor * 4);
                std::uniform_int_distribution<std::int64_t> ud;
                auto mem = (float *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = (float)ud(re) / (float)std::numeric_limits<std::uint64_t>::max();
                        mem[i*2+1] = (float)ud(re) / (float)std::numeric_limits<std::uint64_t>::max();
                        res[i] = std::complex<double>(mem[i*2+0], mem[i*2+1]);
                    } else {
                        mem[i] = (float)ud(re) / (float)std::numeric_limits<std::uint64_t>::max();
                        res[i] = std::complex<double>(mem[i]);
                    }
                }
            }
            break;

        case DataType::kFloat64:
            {
                buf.resize(factor * 8);
                std::uniform_int_distribution<std::int64_t> ud;
                auto mem = (double *) buf.data();
                for (std::size_t i = 0; i < length; i++) {
                    if (is_complex) {
                        mem[i*2+0] = (double)ud(re) / (double)std::numeric_limits<std::uint64_t>::max();
                        mem[i*2+1] = (double)ud(re) / (double)std::numeric_limits<std::uint64_t>::max();
                        res[i] = std::complex<double>(mem[i*2+0], mem[i*2+1]);
                    } else {
                        mem[i] = (double)ud(re) / (double)std::numeric_limits<std::uint64_t>::max();
                        res[i] = std::complex<double>(mem[i]);
                    }
                }
            }
            break;

        default:
            throw std::runtime_error("unknown data type");
    }

    return std::make_tuple(buf, res);
}

TEST(TestInputParser, ParseBlock)
{
    constexpr double epsilon = 1e-9;

    std::vector<std::size_t> block_sizes { 312, 123, 874, 2048, 1024,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                           4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                           16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                                           921, 1, 432, 1, 225, 1, 231, 1 };

    for (bool is_complex : { false, true }) {
        for (auto dt : ALL_DATA_TYPES) {
            for (double prescale : {1.0, 33.49}) {
                auto parser = InputParser::FromDataType(dt, prescale, is_complex);

                auto[buf, res] = make_test(dt, is_complex);
                std::size_t bs_idx = 0;
                std::size_t buf_idx = 0;
                std::size_t res_idx = 0;

                while (buf_idx < buf.size()) {
                    /* get next block size in round robin fashion */
                    auto bs = block_sizes[bs_idx % block_sizes.size()] * parser->GetDataTypeSize();
                    bs_idx++;

                    /* build block */
                    bs = std::min(bs, buf.size() - buf_idx);
                    std::vector<char> block(bs);
                    ::memcpy(block.data(), buf.data() + buf_idx, bs);
                    buf_idx += bs;

                    /* parse */
                    std::size_t parsed_count = 0;
                    EXPECT_NO_THROW(parsed_count = parser->ParseBlock(block));

                    /* retrieve parsed */
                    if (parsed_count > 0) {
                        std::vector<Complex> values;
                        EXPECT_NO_THROW(values = parser->PeekValues(parsed_count));
                        EXPECT_EQ(values.size(), parsed_count);
                        EXPECT_NO_THROW(parser->RemoveValues(parsed_count));

                        for (std::size_t i = 0; i < parsed_count; i++, res_idx++) {
                            EXPECT_TRUE(res_idx < res.size());
                            EXPECT_LE(std::abs(res[res_idx] * prescale - values[i]), epsilon);
                        }
                    }
                }
            }
        }
    }
}

TEST(TestInputParser, PeekValues)
{
    constexpr std::size_t memory = 1024;
    std::vector<char> block(memory);
    for (bool is_complex : { false, true }) {
        for (auto dt : ALL_DATA_TYPES) {
            auto parser = InputParser::FromDataType(dt, 1.0, is_complex);
            std::size_t parsed_count = 0;
            EXPECT_NO_THROW(parsed_count = parser->ParseBlock(block));
            for (std::size_t i = 0; i < parsed_count + 100; i ++) {
                std::vector<Complex> result;
                EXPECT_NO_THROW(result = parser->PeekValues(i));
                EXPECT_TRUE((result.size() == i) || (result.size() == memory / parser->GetDataTypeSize()));
            }
        }
    }
}

TEST(TestInputParser, RemoveValues)
{
    constexpr std::size_t memory = 1024;
    std::vector<char> block(memory);
    for (bool is_complex : { false, true }) {
        for (auto dt : ALL_DATA_TYPES) {
            for (std::size_t i = 0; i < (memory / InputParser::FromDataType(dt, 1.0, is_complex)->GetDataTypeSize()) + 100; i++) {
                auto parser = InputParser::FromDataType(dt, 1.0, is_complex);
                std::size_t parsed_count = 0;
                EXPECT_NO_THROW(parsed_count = parser->ParseBlock(block));

                std::vector<Complex> result;
                EXPECT_NO_THROW(parser->RemoveValues(i));
                EXPECT_NO_THROW(result = parser->PeekValues(1000000));
                EXPECT_EQ(result.size(), std::max<std::int64_t>(parsed_count - i, 0));
            }
        }
    }
}