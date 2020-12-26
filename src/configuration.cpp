/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "configuration.hpp"
#include "specgram.hpp"

#include <spdlog/spdlog.h>
#include <args.hxx>
#include <tuple>
#include <regex>

Configuration::Configuration()
{
    this->input_filename_ = {};
    this->output_filename_ = {};

    this->block_size_ = 256;
    this->rate_ = 44100;
    this->datatype_ = DataType::kSignedInt16;
    this->prescale_factor_ = 1.0f;

    this->fft_width_ = 1024;
    this->fft_stride_ = 1024;
    this->alias_negative_ = true;
    this->window_function_ = FFTWindowFunction::kHann;

    this->no_resampling_ = false;
    this->width_ = 512;
    this->min_freq_ = 0;
    this->max_freq_ = this->rate_ / 2;
    this->scale_ = FFTScale::kdBFS;
    this->color_map_ = ColorMapType::kJet;
    this->background_color_ = sf::Color(0, 0, 0);
    this->foreground_color_ = sf::Color(255, 255, 255);
    this->has_axes_ = false;
    this->has_legend_ = false;
    this->is_horizontal_ = false;

    this->live_ = false;
    this->count_ = 512;
    this->title_ = "Spectrogram";


    this->has_live_window_ = false;
    this->margin_size_ = 30;
    this->minimum_margin_size_ = 15;
    this->legend_height_ = 20;
    this->live_fft_height_ = 100;
    this->axis_font_size_ = 12;
}

Configuration
Configuration::GetForLive() const
{
    Configuration c(*this);

    /* overridden configuration for live output */
    c.has_axes_ = true;
    c.has_legend_ = true;
    c.has_live_window_ = true;

    /* do not allow transparent background, will generate artifacts on live view */
    c.background_color_.a = 255;

    return c;
}

std::optional<sf::Color>
Configuration::StringToColor(const std::string& str)
{
    try {
        if (std::regex_match(str, std::regex("[0-9a-fA-F]{6}"))) {
            unsigned int color = std::strtoul(str.c_str(), 0, 16);
            return sf::Color(
                    ((color >> 16) & 0xff),
                    ((color >> 8) & 0xff),
                    (color & 0xff),
                    255);
        } else if (std::regex_match(str, std::regex("[0-9a-fA-F]{8}"))) {
            unsigned int color = std::strtoul(str.c_str(), 0, 16);
            return sf::Color(
                    ((color >> 24) & 0xff),
                    ((color >> 16) & 0xff),
                    ((color >> 8) & 0xff),
                    (color & 0xff));
        } else {
            return {};
        }
    } catch (const std::exception& e) {
        return {};
    }
}

std::tuple<Configuration, int, bool>
Configuration::FromArgs(int argc, char **argv)
{
    Configuration conf;

    /* build parser */
    args::ArgumentParser parser("Generate spectrogram from stdin.", "For more info see https://github.com/rimio/specgram");

    args::Positional<std::string> outfile(parser, "outfile", "Output PNG file");

    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag version(parser, "version", "Display version", {'v', "version"});

    args::Group input_opts(parser, "Input options:", args::Group::Validators::DontCare);
    args::ValueFlag<std::string>
        infile(input_opts, "string", "Input file name", {'i', "input"});
    args::ValueFlag<float>
        rate(input_opts, "float", "Sampling rate of input in Hz (default: 44100)", {'r', "rate"});
    args::ValueFlag<std::string>
        datatype(input_opts, "string", "Data type of input (default: s16)", {'d', "datatype"});
    args::ValueFlag<float>
        prescale(input_opts, "float", "Prescaling factor (default: 1.0)", {'p', "prescale"});
    args::ValueFlag<int>
        block_size(input_opts, "integer", "Block size when reading input, in data types (default: 256)", {'b', "block_size"});

    args::Group fft_opts(parser, "FFT options:", args::Group::Validators::DontCare);
    args::ValueFlag<int>
        fft_width(fft_opts, "integer", "FFT window width (default: 1024)", {'f', "fft_width"});
    args::ValueFlag<int>
        fft_stride(fft_opts, "integer", "FFT window stride (default: 1024)", {'g', "fft_stride"});
    args::ValueFlag<std::string>
        win_func(fft_opts, "string", "Window function (default: hann)", {'n', "window_function"});
    args::ValueFlag<bool>
        alias(fft_opts, "boolean", "Alias negative and positive frequencies (default: 0 (no) for complex data types, 1 (yes) otherwise)",
              {'s', "alias"});

    args::Group display_opts(parser, "Display options:", args::Group::Validators::DontCare);
    args::Flag
        no_resampling(display_opts, "no_resampling", "No resampling; width will be computed from FFT parameters and fmin/fmax",
                      {'q', "no_resampling"});
    args::ValueFlag<int>
        width(display_opts, "integer", "Display width (default: 512)", {'w', "width"});
    args::ValueFlag<float>
        fmin(display_opts, "float", "Minimum frequency in Hz (default: -0.5 * rate for complex data types, 0 otherwise)", {'x', "fmin"});
    args::ValueFlag<float>
        fmax(display_opts, "float", "Maximum frequency in Hz (default: 0.5 * rate)", {'y', "fmax"});
    args::ValueFlag<std::string>
        scale(display_opts, "string", "Display scale (default: dBFS)", {'s', "scale"});
    args::ValueFlag<std::string>
        colormap(display_opts, "string", "Colormap (default: jet)", {'c', "colormap"});
    args::ValueFlag<std::string>
        bgcolor(display_opts, "string", "Background color (default: 000000)", {"bg-color"});
    args::ValueFlag<std::string>
        fgcolor(display_opts, "string", "Foreground color (default: ffffff)", {"fg-color"});
    args::Flag
        axes(display_opts, "axes", "Display axes (inferred for -e, --legend)", {'a', "axes"});
    args::Flag
        legend(display_opts, "legend", "Display legend", {'e', "legend"});
    args::Flag
        horizontal(display_opts, "horizontal", "Display horizontally", {'z', "horizontal"});

    args::Group live_opts(parser, "Live options:", args::Group::Validators::DontCare);
    args::Flag
        live(live_opts, "live", "Display live spectrogram", {'l', "live"});
    args::ValueFlag<int>
        count(live_opts, "integer", "Number of FFT windows in displayed history (default: 512)", {'k', "count"});
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
    } catch (args::ValidationError& e) {
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
    if (outfile) {
        conf.output_filename_ = args::get(outfile);
    } else if (!live) {
        std::cerr << "Either specify file or '--live', otherwise nothing to do." << std::endl;
        return std::make_tuple(conf, 1, true);
    }

    if (infile) {
        conf.input_filename_ = args::get(infile);
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
        if (dtype == "s8") {
            conf.datatype_ = DataType::kSignedInt8;
            conf.alias_negative_ = true;
        } else if (dtype == "s16") {
            conf.datatype_ = DataType::kSignedInt16;
            conf.alias_negative_ = true;
        } else if (dtype == "s32") {
            conf.datatype_ = DataType::kSignedInt32;
            conf.alias_negative_ = true;
        } else if (dtype == "s64") {
            conf.datatype_ = DataType::kSignedInt64;
            conf.alias_negative_ = true;
        } else if (dtype == "u8") {
            conf.datatype_ = DataType::kUnsignedInt8;
            conf.alias_negative_ = true;
        } else if (dtype == "u16") {
            conf.datatype_ = DataType::kUnsignedInt16;
            conf.alias_negative_ = true;
        } else if (dtype == "u32") {
            conf.datatype_ = DataType::kUnsignedInt32;
            conf.alias_negative_ = true;
        } else if (dtype == "u64") {
            conf.datatype_ = DataType::kUnsignedInt64;
            conf.alias_negative_ = true;
        } else if (dtype == "f32") {
            conf.datatype_ = DataType::kFloat32;
            conf.alias_negative_ = true;
        } else if (dtype == "f64") {
            conf.datatype_ = DataType::kFloat64;
            conf.alias_negative_ = true;
        } else if (dtype == "c64") {
            conf.datatype_ = DataType::kComplex64;
            conf.alias_negative_ = false;
        } else if (dtype == "c128") {
            conf.datatype_ = DataType::kComplex128;
            conf.alias_negative_ = false;
        } else {
            std::cerr << "Unknown data type '" << dtype << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        }
    }
    if (prescale) {
        conf.prescale_factor_ = args::get(prescale);
    }

    if (fft_width) {
        if (args::get(fft_width) <= 0) {
            std::cerr << "'fft_width' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.fft_width_ = args::get(fft_width);
        }
    }
    if (conf.fft_width_ % 2 == 0) {
        double boundary = conf.rate_ * (conf.fft_width_ - 2.0f) / (2.0f * conf.fft_width_);
        conf.min_freq_ = conf.alias_negative_ ? 0.0f : -boundary;
        conf.max_freq_ = conf.rate_ / 2.0f;
    } else {
        double boundary = conf.rate_ * (conf.fft_width_ - 1.0f) / (2.0f * conf.fft_width_);
        conf.min_freq_ = conf.alias_negative_ ? 0.0f : -boundary;
        conf.max_freq_ = boundary;
    }
    if (fft_stride) {
        if (args::get(fft_stride) <= 0) {
            std::cerr << "'fft_width' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.fft_stride_ = args::get(fft_stride);
        }
    }
    if (win_func) {
        auto& wf_str = args::get(win_func);
        if (wf_str == "none") {
            conf.window_function_ = FFTWindowFunction::kNone;
        } else if (wf_str == "hann") {
            conf.window_function_ = FFTWindowFunction::kHann;
        } else if (wf_str == "hamming") {
            conf.window_function_ = FFTWindowFunction::kHamming;
        } else if (wf_str == "blackman") {
            conf.window_function_ = FFTWindowFunction::kBlackman;
        } else if (wf_str == "nuttall") {
            conf.window_function_ = FFTWindowFunction::kBlackman;
        } else {
            std::cerr << "Unknown window function '" << wf_str << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        }
    }
    if (alias) {
        conf.alias_negative_ = args::get(alias);
    }

    if (no_resampling) {
        conf.no_resampling_ = true;
    }
    if (width) {
        if (args::get(width) <= 0) {
            std::cerr << "'width' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else if (conf.no_resampling_) {
            std::cerr << "'width' cannot be specified when not resampling (-q, --no_resampling)." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.width_ = args::get(width);
        }
    }
    if (fmin) {
        conf.min_freq_ = args::get(fmin);
    }
    if (fmax) {
        conf.max_freq_ = args::get(fmax);
    }
    if (scale) {
        auto& scale_str = args::get(scale);
        if (scale_str == "dbfs" || scale_str == "dBFS") {
            conf.scale_ = FFTScale::kdBFS;
        } else {
            std::cerr << "Unknown scale '" << scale_str << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        }
    }
    if (colormap) {
        auto& cmap_str = args::get(colormap);
        if (cmap_str == "gray") {
            conf.color_map_ = ColorMapType::kGray;
        } else if (cmap_str == "jet") {
            conf.color_map_ = ColorMapType::kJet;
        } else if (cmap_str == "purple") {
            conf.color_map_ = ColorMapType::kPurple;
        } else if (cmap_str == "blue") {
            conf.color_map_ = ColorMapType::kBlue;
        } else if (cmap_str == "green") {
            conf.color_map_ = ColorMapType::kGreen;
        } else if (cmap_str == "orange") {
            conf.color_map_ = ColorMapType::kOrange;
        } else if (cmap_str == "red") {
            conf.color_map_ = ColorMapType::kRed;
        } else {
            auto color = Configuration::StringToColor(cmap_str);
            if (!color.has_value()) {
                std::cerr << "Unknown colormap '" << cmap_str << "'" << std::endl;
                return std::make_tuple(conf, 1, true);
            } else {
                conf.color_map_custom_color_ = *color;
                conf.color_map_ = ColorMapType::kCustom;
            }
        }
    }
    if (bgcolor) {
        auto& str = args::get(bgcolor);
        auto color = Configuration::StringToColor(str);
        if (!color.has_value()) {
            std::cerr << "Invalid background color '" << str << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.background_color_ = *color;
        }
    }
    if (fgcolor) {
        auto& str = args::get(fgcolor);
        auto color = Configuration::StringToColor(str);
        if (!color.has_value()) {
            std::cerr << "Invalid foreground color '" << str << "'" << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.foreground_color_ = *color;
        }
    }
    if (axes) {
        conf.has_axes_ = true;
    }
    if (legend) {
        conf.has_legend_ = true;
    }
    if (horizontal) {
        conf.is_horizontal_ = true;
    }

    if (live) {
        conf.live_ = true;
        if (infile) {
            std::cerr << "live view not allowed on file input (-i, --input)" << std::endl;
            return std::make_tuple(conf, 1, true);
        }
    }
    if (count) {
        if (args::get(count) <= 0) {
            std::cerr << "'count' must be positive." << std::endl;
            return std::make_tuple(conf, 1, true);
        } else {
            conf.count_ = args::get(count);
        }
    }
    if (title) {
        conf.title_ = args::get(title);
    }

    /* compute width for --no_resampling case */
    if (conf.no_resampling_) {
        int mini = static_cast<int>(std::round(FFT::GetFrequencyIndex(conf.rate_, conf.fft_width_, conf.min_freq_)));
        int maxi = static_cast<int>(std::round(FFT::GetFrequencyIndex(conf.rate_, conf.fft_width_, conf.max_freq_)));

        if (mini < 0) {
            std::cerr
                    << "'fmin' is outside of FFT window, which is not allowed when not resampling (-q, --no_resampling)."
                    << std::endl;
            return std::make_tuple(conf, 1, true);
        }
        if (maxi >= static_cast<int>(conf.fft_width_)) {
            std::cerr
                    << "'fmax' is outside of FFT window, which is not allowed when not resampling (-q, --no_resampling)."
                    << std::endl;
            return std::make_tuple(conf, 1, true);
        }

        conf.width_ = maxi - mini;
        if (conf.width_ == 0) {
            std::cerr
                    << "'fmin' and 'fmax' are either equal or very close, which is not allowed when not resampling (-q, --no_resampling)."
                    << std::endl;
            return std::make_tuple(conf, 1, true);
        }
        /* (maxi-mini) < 0 should be caught lower */
    }

    /* fmin/fmax checks */
    if (conf.min_freq_ >= conf.max_freq_) {
        std::cerr << "'fmin' must be less than 'fmax'." << std::endl;
        return std::make_tuple(conf, 1, true);
    }

    /* normal usage mode, don't exit */
    return std::make_tuple(conf, 0, false);
}