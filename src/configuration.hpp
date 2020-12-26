/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _CONFIGURATION_HPP_
#define _CONFIGURATION_HPP_

#include "input.hpp"
#include "fft.hpp"
#include "colormap.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <tuple>

class Configuration {
private:
    std::optional<std::string> input_filename_;
    std::optional<std::string> output_filename_;

    std::size_t block_size_;
    double rate_;
    DataType datatype_;

    std::size_t fft_width_;
    std::size_t fft_stride_;
    FFTWindowFunction window_function_;
    bool alias_negative_;

    bool no_resampling_;
    std::size_t width_;
    double min_freq_;
    double max_freq_;
    FFTScale scale_;
    ColorMapType color_map_;
    sf::Color color_map_custom_color_;
    bool has_axes_;
    bool has_legend_;
    bool is_horizontal_;

    bool live_;
    std::size_t count_;
    std::string title_;

    bool has_live_window_;

    std::size_t margin_size_;
    std::size_t minimum_margin_size_;
    std::size_t legend_height_;
    std::size_t live_fft_height_;
    std::size_t axis_font_size_;
    sf::Color background_color_;
    sf::Color axes_color_;

    Configuration();

public:
    /* parse command line arguments and return a configuration object */
    static std::tuple<Configuration, int, bool> FromArgs(int argc, char **argv);

    /* generic getters */
    Configuration GetForLive() const;
    const auto & GetInputFilename() const { return input_filename_; }
    const auto & GetOutputFilename() const { return output_filename_; }

    /* input getters */
    auto GetBlockSize() const { return block_size_; }
    auto GetRate() const { return rate_; }
    auto GetDataType() const { return datatype_; }

    /* FFT getters */
    auto GetFFTWidth() const { return fft_width_; }
    auto GetFFTStride() const { return fft_stride_; }
    auto GetWindowFunction() const { return window_function_; }
    auto IsAliasingNegativeFrequencies() const { return alias_negative_; }

    /* display getters */
    auto CanResample() const { return !no_resampling_; }
    auto GetWidth() const { return width_; }
    auto GetMinFreq() const { return min_freq_; }
    auto GetMaxFreq() const { return max_freq_; }
    auto GetScale() const { return scale_; }
    auto GetColorMap() const { return color_map_; }
    auto GetColorMapCustomColor() const { return color_map_custom_color_; }
    auto HasAxes() const { return has_axes_ || has_legend_; }
    auto HasLegend() const { return has_legend_; }
    auto IsHorizontal() const { return is_horizontal_; }

    /* live options */
    auto IsLive() const { return live_; }
    auto GetCount() const { return count_; }
    auto GetTitle() const { return title_; }

    /* internal options */
    auto HasLiveWindow() const { return has_live_window_; }

    auto GetMarginSize() const { return margin_size_; }
    auto GetMinimumMarginSize() const { return minimum_margin_size_; }
    auto GetLegendHeight() const { return legend_height_; }
    auto GetLiveFFTHeight() const { return live_fft_height_; }

    auto GetAxisFontSize() const { return axis_font_size_; }

    auto GetBackgroundColor() const { return background_color_; }
    auto GetAxesColor() const { return axes_color_; }
};

#endif