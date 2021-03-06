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

/* Axis tick */
typedef std::tuple<double, std::string> AxisTick;

/* Orientation */
enum class Orientation {
    k90CCW,
    kNormal,
    k90CW,
    k180
};

/*
 * Spectrogram rendering class
 */
class Renderer {
private:
    const Configuration configuration_;
    const std::size_t fft_count_;

    sf::Font font_;

    sf::RenderTexture canvas_;
    sf::Texture fft_area_texture_;

    std::size_t width_;
    std::size_t height_;

    sf::Transform legend_transform_;
    sf::Transform fft_live_transform_;
    sf::Transform fft_area_transform_;

    std::list<AxisTick> frequency_ticks_;
    std::list<AxisTick> live_ticks_;

    static std::list<AxisTick> GetLinearTicks(double v_min, double v_max, const std::string& v_unit,
                                              unsigned int num_ticks);
    static std::list<AxisTick> GetNiceTicks(double v_min, double v_max, const std::string& v_unit,
                                            unsigned int length_px, unsigned int est_tick_length_px);

    void RenderAxis(sf::RenderTexture& texture,
                    const sf::Transform& t, bool lhs, Orientation orientation, double length,
                    const std::list<AxisTick>& ticks);

public:
    Renderer() = delete;
    Renderer(const Configuration& conf, const ColorMap& cmap, const ValueMap& vmap, std::size_t fft_count);

    /* render commands */
    void RenderFFTArea(const std::vector<uint8_t>& memory);
    void RenderFFTArea(const std::list<std::vector<uint8_t>>& history);
    void RenderLiveFFT(const RealWindow& window, const std::vector<uint8_t>& colors);

    /* canvas builder */
    sf::Texture GetCanvas();

    /* size getters */
    auto GetWidth() const { return width_; }
    auto GetHeight() const { return height_; }
};

#endif