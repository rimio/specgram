/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "test.hpp"
#include "../src/input-reader.hpp"
#include <fstream>
#include <random>
#include <string>
#include <thread>
#include <csignal>

std::vector<char> random_data(std::size_t size)
{
    std::vector<char> data(size);

    std::random_device rd;
    std::default_random_engine re(rd());
    std::uniform_int_distribution<std::int8_t> ud;

    for (std::size_t i = 0; i < size; i ++) {
        data[i] = ud(re);
    }

    return data;
}

void generate_file(const std::string& temp_file_name, const std::vector<char>& buffer)
{
    /* dump */
    std::ofstream file(temp_file_name, std::ios::out | std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("cannot open temp file " + temp_file_name);
    }
    file.write(buffer.data(), buffer.size());
    file.close();
}

void check_same(const std::vector<char>& expected, const std::vector<char>& actual, std::size_t max_len_diff)
{
    EXPECT_GE(expected.size(), actual.size());
    EXPECT_LE(expected.size() - actual.size(), max_len_diff);
    for (std::size_t i = 0; i<actual.size(); i++) {
        EXPECT_EQ(expected[i], actual[i]);
    }
}

TEST(TestInputReader, BadParameters)
{
    EXPECT_THROW_MATCH(SyncInputReader(nullptr, 100),
                       std::runtime_error, "valid stream is required");
    EXPECT_THROW_MATCH(AsyncInputReader(nullptr, 100),
                       std::runtime_error, "valid stream is required");

    EXPECT_THROW_MATCH(SyncInputReader((std::istream *)1, 0),
                       std::runtime_error, "block size in bytes must be positive");
    EXPECT_THROW_MATCH(AsyncInputReader((std::istream *)1, 0),
                       std::runtime_error, "block size in bytes must be positive");
}

TEST(TestInputReader, SyncInputReader)
{
    constexpr std::size_t max_block_size = 4096;
    constexpr std::size_t memory = 4096;
    const std::string file_name = "/dev/shm/TestInputReader_SyncInputReader.data";

    auto expected = random_data(memory);
    generate_file(file_name, expected);

    for (std::size_t block_size = 1; block_size < max_block_size; block_size++) {
        std::ifstream file(file_name, std::ios::in | std::ios::binary);
        EXPECT_FALSE(file.fail());
        SyncInputReader reader(&file, block_size);

        std::vector<char> output;
        output.reserve(memory);
        while (true) {
            auto block = reader.GetBlock();
            if (!block.has_value()) {
                break;
            }
            EXPECT_EQ((*block).size(), block_size);
            output.insert(output.end(), (*block).begin(), (*block).end());
        }
        EXPECT_TRUE(reader.ReachedEOF());
        file.close();

        check_same(expected, output, block_size);
    }
}

TEST(TestInputReader, AsyncInputReader)
{
    constexpr std::size_t max_block_size = 4096;
    constexpr std::size_t memory = 4096;
    const std::string file_name = "/dev/shm/TestInputReader_AsyncInputReader.data";

    auto expected = random_data(memory);
    generate_file(file_name, expected);

    for (std::size_t block_size = 1; block_size < max_block_size; block_size++) {
        std::ifstream file(file_name, std::ios::in | std::ios::binary);
        EXPECT_FALSE(file.fail());

        std::vector<char> output;
        output.reserve(memory);

        std::signal(SIGINT, [](int) {  }); /* SIGINT is sent to the reader thread upon destruction of AsyncInputReader */
        { /* scope out the reader so it does not die when we close the file */
            AsyncInputReader reader(&file, block_size);
            while (output.size() < expected.size() - block_size) {
                auto block = reader.GetBlock();
                if (block.has_value()) {
                    EXPECT_EQ((*block).size(), block_size);
                    output.insert(output.end(), (*block).begin(), (*block).end());
                }
            }
            EXPECT_FALSE(reader.ReachedEOF());
        }
        std::signal(SIGINT, nullptr);

        file.close();

        check_same(expected, output, block_size);
    }
}