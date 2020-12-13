/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "configuration.hpp"
#include "specgram.hpp"

#include <args.hxx>
#include <tuple>


Configuration::Configuration()
{
    this->filename_ = {};

    this->block_size_ = 256;
    this->rate_ = 44100;
    this->datatype_ = Input::DataType::kSignedInt16;

    this->fft_width_ = 512;
    this->fft_stride_ = 512;
}

std::tuple<Configuration, int, bool>
Configuration::FromArgs(int argc, char **argv)
{
    Configuration conf;

    /* build parser */
    args::ArgumentParser parser("Generate spectrogram from stdin.", "For more info see https://github.com/rimio/specgram");

    args::Positional<std::string> file(parser, "file", "Output PNG file");

    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag version(parser, "version", "Display version", {'v', "version"});

    args::Group input_opts(parser, "Input options:", args::Group::Validators::DontCare);
    args::ValueFlag<int>
        rate(input_opts, "integer", "Sampling rate of input in Hz (default: 44100)", {'r', "rate"});
    args::ValueFlag<std::string>
        datatype(input_opts, "string", "Data type of input (default: s16)", {'d', "datatype"});
    args::ValueFlag<int>
        block_size(input_opts, "integer", "Block size when reading input, in data types (default: 256)", {'b', "block_size"});

    args::Group fft_opts(parser, "FFT options:", args::Group::Validators::DontCare);
    args::ValueFlag<int>
        fft_width(fft_opts, "integer", "FFT window width (default: 512)", {'f', "fft_width"});
    args::ValueFlag<int>
        fft_stride(fft_opts, "integer", "FFT window stride (default: 512)", {'g', "fft_stride"});

    args::Group display_opts(parser, "Display options:", args::Group::Validators::DontCare);
    args::ValueFlag<int>
        width(display_opts, "integer", "Display width (default: 512)", {'w', "width"});
    args::ValueFlag<int>
        fmin(display_opts, "integer", "Minimum frequency in Hz (default: -0.5 * rate for complex data types, 0 otherwise)", {'x', "fmin"});
    args::ValueFlag<int>
        fmax(display_opts, "integer", "Maximum frequency in Hz (default: 0.5 * rate)", {'y', "fmax"});
    args::ValueFlag<std::string>
        scale(display_opts, "string", "Display scale (default: dBFS)", {'s', "scale"});
    args::ValueFlag<std::string>
        colormap(display_opts, "string", "Colormap (default: gray)", {'c', "colormap"});
    args::Flag
        axes(display_opts, "axes", "Display axes", {'a', "axes"});
    args::Flag
        legend(display_opts, "legend", "Display legend", {'e', "legend"});
    args::Flag
        horizontal(display_opts, "horisontal", "Display horizontally", {'z', "horizontal"});

    args::Group live_opts(parser, "Display options:", args::Group::Validators::DontCare);
    args::Flag
        live(live_opts, "live", "Display live spectrogram", {'l', "live"});
    args::ValueFlag<int>
        window_count(live_opts, "integer", "Number of FFT windows in displayed history (default: 512)", {'k', "count"});
    args::ValueFlag<std::string>
        title(live_opts, "string", "Window title", {'t', "title"});

    /* parse arguments */
    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << parser;
        return std::make_tuple(conf, 0, true);
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return std::make_tuple(conf, 1, true);
    } catch (args::ValidationError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return std::make_tuple(conf, 1, true);
    }

    /* version mode */
    if (version) {
        std::cout << "specgram version " << SPECGRAM_VERSION << std::endl;
        return std::make_tuple(conf, 0, true);
    }

    /* check and store command line arguments */
    if (file) {
        conf.filename_ = args::get(file);
    } else if (!live) {
        std::cerr << "Either specify file or '--live', otherwise nothing to do." << std::endl;
        return std::make_tuple(conf, 1, true);
    }

    if (block_size) {
        if (args::get(block_size) <= 0) {
            std::cerr << "'block_size' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.block_size_ = args::get(block_size);
        }
    }
    if (rate) {
        if (args::get(rate) <= 0) {
            std::cerr << "'rate' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.rate_ = args::get(rate);
        }
    }
    if (datatype) {
        auto& dtype = args::get(datatype);
        if (dtype == "s16") {
            conf.datatype_ = Input::DataType::kSignedInt16;
        } else {
            std::cerr << "Unknown data type '" << dtype << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        }
    }

    if (fft_width) {
        if (args::get(fft_width) <= 0) {
            std::cerr << "'fft_width' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.fft_width_ = args::get(fft_width);
        }
    }
    if (fft_stride) {
        if (args::get(fft_stride) <= 0) {
            std::cerr << "'fft_width' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.fft_stride_ = args::get(fft_stride);
        }
    }

    /* normal usage mode, don't exit */
    return std::make_tuple(conf, 0, false);
}