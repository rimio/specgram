/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "configuration.hpp"
#include "input-parser.hpp"
#include "input-reader.hpp"
#include "color-map.hpp"
#include "value-map.hpp"
#include "window-function.hpp"
#include "fft.hpp"
#include "live.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <list>
#include <random>
#include <cstdio>

/* main loop exit condition */
volatile bool main_loop_running = true;

/* temporary output file name */
std::string temp_file_name = "";

/*
 * SIGINT handler
 */
void
sigint_handler(int)
{
    /* no longer loop */
    main_loop_running = false;

    /* uninstall handler in case user REALLY does not want to wait for wrap-up */
    std::signal(SIGINT, nullptr);
}

/*
 * printing functions
 */
void
print_complex_window(const std::string& name, const ComplexWindow& window)
{
    std::cout << name << ": [";
    for (const auto& v : window) {
        std::cout << " " << std::setprecision(3) << std::fixed << v.real()
                  << std::showpos << v.imag() << "j";
    }
    std::cout << "]" << std::endl;
}

void
print_real_window(const std::string& name, const RealWindow& window)
{
    std::cout << name << ": [";
    for (const auto& v : window) {
        std::cout << " " << std::setprecision(3) << std::fixed << v;
    }
    std::cout << "]" << std::endl;
}

/*
 * generate a random string (see https://stackoverflow.com/a/50556436)
 */
std::string
generate_random_string(std::size_t length)
{
    std::mt19937 generator { std::random_device{}() };
    std::uniform_int_distribution<int> distribution { 'a', 'z' };

    std::string output(length, '\0');
    for(auto& ch : output) {
        ch = static_cast<char>(distribution(generator) & 0xff);
    }

    return output;
}

/*
 * dump image to stdout (in PNG format). This writes a temp file in /dev/shm since SFML cannot save to memory.
 */
void
dump_to_stdout(const sf::Image& image)
{
    static constexpr size_t TEMP_BUFFER_SIZE = 1024;
    static constexpr size_t TEMP_FILENAME_LENGTH = 32;

    /* save */
    temp_file_name = "/dev/shm/" + generate_random_string(TEMP_FILENAME_LENGTH) + ".png";
    spdlog::info("Temporary file: {}", temp_file_name);
    image.saveToFile(temp_file_name);

    /* from now on we have a leakable resource (the file); if using STDIN for input, we're here from a SIGINT,
     * and we expect a SIGPIPE soon; install a handler that will clean up */
    std::signal(SIGPIPE, [](int) { std::remove(temp_file_name.c_str()); /* no logger, no checks */ std::exit(0); });

    /* dump */
    std::ifstream file(temp_file_name, std::ios::in | std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("cannot read temp file " + temp_file_name);
    }
    while (!file.eof()) {
        char buffer[TEMP_BUFFER_SIZE];
        file.read(buffer, TEMP_BUFFER_SIZE);
        std::size_t read_count = file.gcount();
        std::cout.write(buffer, read_count);
    }
    file.close();

    /* clean up */
    if (std::remove(temp_file_name.c_str()) != 0) {
        spdlog::warn("Failed to delete temp file {}", temp_file_name);
    }
}

/*
 * entry point
 */
int
main(int argc, char** argv)
{
    /* set spdlog to STDERR and make it multithreaded */
    spdlog::set_default_logger(spdlog::stderr_color_mt("stderr"));

    /* parse command line arguments into global settings */
    auto [conf, conf_rc, conf_must_exit] = Configuration::FromArgs(argc, argv);
    if (conf_must_exit) {
        return conf_rc;
    }

    /* decide whether we have output or not */
    bool have_output = conf.GetOutputFilename().has_value() || conf.MustDumpToStdout();

    /* create window function */
    auto win_function = WindowFunction::FromType(conf.GetWindowFunction(), conf.GetFFTWidth());

    /* create FFT */
    spdlog::info("Creating {}-wide FFTW plan", conf.GetFFTWidth());
    FFT fft(conf.GetFFTWidth(), win_function);

    /* create value map */
    spdlog::info("Scale {}, unit {}, bounds [{}, {}]",
                 conf.GetScaleType() == ValueMapType::kLinear ? "linear" : "decibel",
                 conf.GetScaleUnit(),
                 conf.GetScaleLowerBound(),
                 conf.GetScaleUpperBound());
    std::unique_ptr<ValueMap> value_map = ValueMap::Build(conf.GetScaleType(),
                                                          conf.GetScaleLowerBound(),
                                                          conf.GetScaleUpperBound(),
                                                          conf.GetScaleUnit());

    /* create color map */
    auto color_map = ColorMap::FromType(conf.GetColorMap(),
                                        conf.GetBackgroundColor(),
                                        conf.GetColorMapCustomColor());

    /* create live window */
    std::unique_ptr<LiveOutput> live = nullptr;
    if (conf.IsLive()) {
        live = std::make_unique<LiveOutput>(conf, *color_map, *value_map);
    }

    /* create input parser */
    auto input = InputParser::FromDataType(conf.GetDataType(), conf.GetPrescaleFactor(), conf.HasComplexInput());
    if (input == nullptr) {
        return 1;
    }

    /* create input reader */
    std::istream *input_stream = nullptr;
    std::unique_ptr<InputReader> reader = nullptr;
    if (conf.GetInputFilename().has_value()) {
        spdlog::info("Input: {}", *conf.GetInputFilename());
        input_stream = new std::ifstream(*conf.GetInputFilename(), std::ios::in | std::ios::binary);
        assert(input_stream != nullptr);
        if (!input_stream->good()) {
            spdlog::error("Failed to open input file '{}'", *conf.GetInputFilename());
            return 1;
        }
        reader = std::make_unique<SyncInputReader>(input_stream,
                                                   input->GetDataTypeSize() * conf.GetBlockSize());
    } else {
        spdlog::info("Input: STDIN");
        input_stream = &std::cin;
        reader = std::make_unique<AsyncInputReader>(input_stream,
                                                    input->GetDataTypeSize() * conf.GetBlockSize());
    }

    /* display initialization info */
    if (input->IsFloatingPoint()) {
        spdlog::info("Input stream: {}{}bit floating point at {}Hz",
                     input->IsComplex() ? "complex " : "",
                     input->GetDataTypeSize() * 8, conf.GetRate());
    } else {
        spdlog::info("Input stream: {}{} {}bit integer at {}Hz",
                     input->IsComplex() ? "complex " : "",
                     input->IsSigned() ? "signed" : "unsigned",
                     input->GetDataTypeSize() * 8, conf.GetRate());
    }

    /* install SIGINT handler for CTRL+C */
    std::signal(SIGINT, sigint_handler);

    /* FFT window history */
    std::list<std::vector<uint8_t>> history;

    /* window average */
    RealWindow window_sum;
    window_sum.resize(conf.GetWidth());
    std::size_t window_sum_count = 0;

    /* main loop */
    while (main_loop_running && !reader->ReachedEOF()) {
        /* check for window events (if necessary) */
        if (live != nullptr) {
            if (!live->HandleEvents()) {
                /* exited by closing window */
                main_loop_running = false;
                /* uninstall signal so that reader thread can exit successfully */
                std::signal(SIGINT, nullptr);
            }
        }

        /* check for a complete block */
        auto block = reader->GetBlock();
        if (!block) {
            /* block not finished yet */
            continue;
        }

        /* take whatever is available from input stream */
        auto pvc = input->ParseBlock(*block);
        assert(pvc == block->size() / input->GetDataTypeSize());

        /* check if we have enough for a new FFT window */
        if ((input->GetBufferedValueCount() < conf.GetFFTWidth())
            || (input->GetBufferedValueCount() < conf.GetFFTStride())) {
            /* wait until we get enough values for a window and the spacing between windows */
            continue;
        }

        /* retrieve window and remove values that won't be used further */
        auto window_values = input->PeekValues(conf.GetFFTWidth());
        input->RemoveValues(conf.GetFFTStride());
        if (conf.MustPrintInput()) {
            print_complex_window("input", window_values);
        }

        /* compute FFT on fetched window */
        auto fft_values = fft.Compute(window_values);
        if (conf.MustPrintFFT()) {
            print_complex_window("fft", fft_values);
        }

        /* compute magnitude */
        auto fft_magnitude = FFT::GetMagnitude(fft_values, conf.IsAliasingNegativeFrequencies());

        /* map magnitude to [0..1] domain */
        auto normalized_magnitude = value_map->Map(fft_magnitude);

        if (conf.CanResample()) {
            /* resample to display width */
            normalized_magnitude = FFT::Resample(normalized_magnitude, conf.GetRate(), conf.GetWidth(),
                                                 conf.GetMinFreq(), conf.GetMaxFreq());
        } else {
            /* crop to display width */
            normalized_magnitude = FFT::Crop(normalized_magnitude, conf.GetRate(),
                                             conf.GetMinFreq(), conf.GetMaxFreq());
        }
        if (conf.MustPrintOutput()) {
            print_real_window("output", normalized_magnitude);
        }

        /* add to running total */
        assert(window_sum.size() == normalized_magnitude.size());
        assert(conf.GetAverageCount() > 0);
        for (std::size_t i = 0; i < window_sum.size(); i++) {
            window_sum[i] += normalized_magnitude[i] / conf.GetAverageCount();
        }
        window_sum_count++;

        if (window_sum_count < conf.GetAverageCount()) {
            /* we still have to compute some more windows before we show */
            continue;
        }

        /* colorize FFT */
        auto colorized = color_map->Map(window_sum);

        /* add to live */
        if (live != nullptr) {
            live->AddWindow(colorized, window_sum);
        }

        /* add to history */
        if (have_output) {
            history.push_back(colorized);
        }

        /* reset */
        window_sum_count = 0;
        for (auto& v : window_sum) {
            v = 0.0f;
        }
    }
    spdlog::info("Terminating ...");

    /* close input file */
    if (conf.GetInputFilename().has_value()) {
        assert(input_stream != nullptr);
        assert(input_stream != &std::cin);
        delete input_stream;
        input_stream = nullptr;
    }

    /* save file */
    if (have_output) {
        Renderer file_renderer(conf, *color_map, *value_map, history.size());
        file_renderer.RenderFFTArea(history);
        auto image = file_renderer.GetCanvas().copyToImage();

        /* rotate, if needed */
        if (conf.IsHorizontal()) {
            std::size_t w = image.getSize().x;
            std::size_t h = image.getSize().y;

            /* allocate memory */
            auto iptr = reinterpret_cast<const uint32_t *>(image.getPixelsPtr());
            uint32_t *optr = new uint32_t[w * h];

            /* copy 4 bytes (one pixel) at a time */
            for (std::size_t l = 0; l < h; l ++) {
                auto in = iptr + l * w;
                auto out = optr + (w-1) * h + l;
                for (std::size_t c = 0; c < w; c++, in++, out -= h) {
                    *out = *in;
                }
            }

            /* create rotated image */
            sf::Image rimage;
            rimage.create(h, w, reinterpret_cast<const uint8_t *>(optr));
            delete[] optr;

            image = rimage;
        }

        /* dump to file or stdout */
        if (conf.GetOutputFilename().has_value()) {
            spdlog::info("Output: {}", *conf.GetOutputFilename());
            image.saveToFile(*conf.GetOutputFilename());
        } else if (conf.MustDumpToStdout()) {
            spdlog::info("Output: STDOUT");
            dump_to_stdout(image);
        } else {
            throw std::runtime_error("don't know what to do with output");
        }
    }

    /* all ok */
    return 0;
}