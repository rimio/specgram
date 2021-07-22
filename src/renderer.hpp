/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "configuration.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <list>

/* Orientation */
enum class Orientation {
    k90CCW,
    kNormal,
    k90CW,
    k180
};

/**
 * Spectrogram rendering class
 */
class Renderer {
protected: /* for all intents and purposes this should be private, but we want to unit test a few of the methods here */
    const Configuration configuration_; /* configuration is cached as it contains multiple settings regarding spacing and sizing */
    const std::size_t fft_count_;       /* number of windows to render */
    const std::unique_ptr<const ColorMap> color_map_; /* color map used for rendering */

    sf::Font font_;

    sf::RenderTexture canvas_;
    sf::Texture fft_area_texture_;      /* actual spectrogram area */

    std::size_t width_;
    std::size_t height_;

    sf::Transform legend_transform_;
    sf::Transform fft_live_transform_;
    sf::Transform fft_area_transform_;

    /**
     * First value specifies the position of the tick in the domain [0..1].
     * Second value is the text of the tick.
     */
    using AxisTick = std::tuple<double, std::string>;

    std::list<AxisTick> frequency_ticks_;
    std::list<AxisTick> live_ticks_;

    /**
     * Return a short representation of the value (using unit prefixes like, m, k, M ...).
     * @param value The value to encode.
     * @param scale Scale of the value (in the numeric sense).
     * @param unit Unit of the value.
     * @return Short representation string.
     *
     * NOTE: The unit prefix is computed just from the scale, not the value itself.
     *       For example, ValueToShortString(0.00004, 5, "V") will yield "0.04mV",
     *       not "40uV" (which would otherwise be the more natural representation).
     *       This behaviour is purposeful, as it allows more flexibility (e.g. if
     *       we want the same prefix and number of decimal places for all values
     *       of an axis).
     */
    static std::string ValueToShortString(double value, int scale, const std::string& unit);

    /**
     * Build an array of ticks with linear spacing.
     * @param v_min Lowest value on the axis.
     * @param v_max Highest value on the axis.
     * @param v_unit Unit of the axis.
     * @param num_ticks Number of ticks to generate.
     * @return Array of ticks.
     */
    [[maybe_unused]] /* need for this method disappeared when fixing #9, might be useful in the future */
    static std::list<AxisTick> GetLinearTicks(double v_min, double v_max, const std::string& v_unit,
                                              unsigned int num_ticks);

    /**
     * Build an array of nicely spaced ticks.
     * @param v_min Lowest value on the axis.
     * @param v_max Highest value on the axis.
     * @param v_unit Unit of the axis.
     * @param length_px Length of the scale, in pixels (used for spacing estimation).
     * @param min_tick_length_px Minimum tick spacing.
     * @param rotated If true then vertical. Otherwise horizontal.
     * @return Array of ticks.
     *
     * NOTE: This method attempts to find some nice values for the ticks that
     *       also have sufficient spacing for the text to fit properly.
     * NOTE: This method enforces an error of <1% between tick text and tick value.
     */
    std::list<AxisTick> GetNiceTicks(double v_min, double v_max, const std::string& v_unit,
                                     unsigned int length_px, unsigned int min_tick_length_px, bool rotated);

    /**
     * Render an axis upon a texture
     * @param texture Texture to render to.
     * @param t Transform to use.
     * @param lhs True if has left-hand side text.
     * @param orientation One of Orientation.
     * @param length Length in pixels.
     * @param ticks Ticks.
     */
    void RenderAxis(sf::RenderTexture& texture,
                    const sf::Transform& t, bool lhs, Orientation orientation, double length,
                    const std::list<AxisTick>& ticks);

public:
    Renderer() = delete;
    Renderer(const Configuration& conf, const ColorMap& cmap, const ValueMap& vmap, std::size_t fft_count);

    /**
     * Render the spectrogram area.
     * @param memory RGBA memory of the colorized spectrogram.
     */
    void RenderFFTArea(const std::vector<uint8_t>& memory);

    /**
     * Render the spectrogram area.
     * @param history List of RGBA colorized windows.
     */
    void RenderFFTArea(const std::list<std::vector<uint8_t>>& history);
    std::vector<uint8_t> RenderLiveFFT(const RealWindow& window);

    /**
     * @return The rendered canvas texture.
     */
    sf::Texture GetCanvas();

    /* size getters */
    auto GetWidth() const { return width_; }
    auto GetHeight() const { return height_; }
};

#endif