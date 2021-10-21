/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _CONFIGURATION_HPP_
#define _CONFIGURATION_HPP_

#include "color-map.hpp"
#include "value-map.hpp"
#include "window-function.hpp"

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <optional>
#include <tuple>

/**
 * Configuration God object. This parses program arguments into usable,
 * structured configuration options.
 */
class Configuration {
private:
    std::optional<std::string> input_filename_;
    std::optional<std::string> output_filename_;
    bool dump_to_stdout_;                   /* true if output PNG image must go to stdout */

    std::size_t block_size_;                /* group read values in blocks of block_size_ items */
    double rate_;                           /* sampling rate of signal, in Hz */
    DataType datatype_;                     /* input data type (does not cover complex/real discrimination) */
    bool has_complex_input_;                /* true if input is complex */
    double prescale_factor_;                /* value to scale input with before applying other transformations */

    std::size_t fft_width_;                 /* size of FFT window, in values */
    std::size_t fft_stride_;                /* stride of FFT window, in values */
    WindowFunctionType window_function_;    /* window function to apply before FFT */
    std::size_t average_count_;             /* number of windows to average for each displayed window */
    bool alias_negative_;                   /* alias negative frequencies to positive */

    bool no_resampling_;                    /* do not perform resampling; if true, width_ is meaningless */
    std::size_t width_;                     /* width of resampled output window, in values or pixels */
    double min_freq_;                       /* lower bound of displayed frewquency band */
    double max_freq_;                       /* upper bound of displayed frewquency band */
    ValueMapType scale_type_;               /* type of scale used on FFT output */
    std::string scale_unit_;                /* unit of the scale (just the text) */
    double scale_lower_bound_;
    double scale_upper_bound_;
    ColorMapType color_map_;                /* color map to apply to scaled output */
    sf::Color color_map_custom_color_;      /* custom color for color map (where applicable) */
    sf::Color background_color_;            /* background color of rendered output */
    sf::Color foreground_color_;            /* foreground color of rendered output (text, axes etc) */
    bool has_axes_;                         /* render axes */
    bool has_legend_;                       /* render legend */
    bool is_horizontal_;                    /* flows from left to right, instead of top to bottom */
    bool print_input_;                      /* debug printing of input */
    bool print_fft_;                        /* debug printing of FFT values */
    bool print_output_;                     /* debug printing of output */

    bool live_;                             /* whether we have live output or not */
    std::size_t count_;                     /* number of output windows to display in spectrogoram */
    std::string title_;                     /* window title */

    bool has_live_window_;                  /* display a live plot of the current FFT window */

    std::size_t margin_size_;
    std::size_t live_margin_size_;
    std::size_t minimum_margin_size_;
    std::size_t legend_height_;
    std::size_t live_fft_height_;
    std::size_t axis_font_size_;

    Configuration();

    /**
     * Translate a hex color to a sf::Color.
     * @param str RGB/RGBA hex color code.
     * @return Color.
     */
    static sf::Color StringToColor(const std::string& str);

    /**
     * Parse a scale string.
     * @param str Scale string.
     * @return Optional bounds and unit string.
     */
    using OptionalBound = std::optional<double>;
    using ScaleProperties = std::tuple<OptionalBound, OptionalBound, std::string>;
    static ScaleProperties StringToScale(const std::string& str);

public:
    /**
     * Parse command line arguments and return a configuration object.
     * @param argc Argument strings.
     * @param argv Argument count.
     * @return New Configuration object instance.
     */
    static std::tuple<Configuration, int, bool> Build(int argc, const char **argv);

    /* generic getters */
    Configuration GetForLive() const;
    const auto & GetInputFilename() const { return input_filename_; }
    const auto & GetOutputFilename() const { return output_filename_; }
    auto MustDumpToStdout() const { return dump_to_stdout_; }

    /* input getters */
    auto GetBlockSize() const { return block_size_; }
    auto GetRate() const { return rate_; }
    auto GetDataType() const { return datatype_; }
    auto HasComplexInput() const { return has_complex_input_; }
    auto GetPrescaleFactor() const { return prescale_factor_; }

    /* FFT getters */
    auto GetFFTWidth() const { return fft_width_; }
    auto GetFFTStride() const { return fft_stride_; }
    auto GetWindowFunction() const { return window_function_; }
    auto IsAliasingNegativeFrequencies() const { return alias_negative_; }
    auto GetAverageCount() const { return average_count_; }

    /* display getters */
    auto CanResample() const { return !no_resampling_; }
    auto GetWidth() const { return width_; }
    auto GetMinFreq() const { return min_freq_; }
    auto GetMaxFreq() const { return max_freq_; }
    auto GetScaleType() const { return scale_type_; }
    auto GetScaleUnit() const { return scale_unit_; }
    auto GetScaleLowerBound() const { return scale_lower_bound_; }
    auto GetScaleUpperBound() const { return scale_upper_bound_; }
    auto GetColorMap() const { return color_map_; }
    auto GetColorMapCustomColor() const { return color_map_custom_color_; }
    auto GetBackgroundColor() const { return background_color_; }
    auto GetForegroundColor() const { return foreground_color_; }
    auto HasAxes() const { return has_axes_ || has_legend_; }
    auto HasLegend() const { return has_legend_; }
    auto IsHorizontal() const { return is_horizontal_; }
    auto MustPrintInput() const { return print_input_; }
    auto MustPrintFFT() const { return print_fft_; }
    auto MustPrintOutput() const { return print_output_; }

    /* live options */
    auto IsLive() const { return live_; }
    auto GetCount() const { return count_; }
    auto GetTitle() const { return title_; }

    /* internal options */
    auto HasLiveWindow() const { return has_live_window_; }

    auto GetMarginSize() const { return margin_size_; }
    auto GetLiveMarginSize() const { return live_margin_size_; }
    auto GetMinimumMarginSize() const { return minimum_margin_size_; }
    auto GetLegendHeight() const { return legend_height_; }
    auto GetLiveFFTHeight() const { return live_fft_height_; }

    auto GetAxisFontSize() const { return axis_font_size_; }

    sf::Color GetLiveGuidelinesColor() const;
};

#endif
