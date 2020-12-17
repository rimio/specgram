/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "configuration.hpp"
#include <SFML/Graphics.hpp>

class Renderer {
private:
    const Configuration configuration_;
    const std::size_t fft_count_;

    sf::Font font_;

    sf::RenderTexture canvas_;
    sf::Texture fft_area_texture_;

    std::size_t width_;
    std::size_t height_;

    sf::Vector2f legend_origin_;
    sf::Vector2f fft_live_origin_;
    sf::Vector2f fft_area_origin_;

    void RenderAxis(sf::RenderTexture& texture,
                    const sf::Vector2f& origin, float rot, float length,
                    float v_min, float v_max, std::string v_unit,
                    float est_tick_dist = 50.0f);

public:
    Renderer() = delete;
    Renderer(const Configuration& conf, std::size_t fft_count);

    /* render commands */
    void RenderUserInterface();
    void RenderFFTArea(const std::vector<uint8_t>& memory);
    void RenderLiveFFT(const std::vector<double>& window);
    void RenderLegend(const std::vector<uint8_t>& memory);

    /* canvas builder */
    sf::Texture GetCanvas() const;

    /* size getters */
    auto GetWidth() const { return width_; }
    auto GetHeight() const { return height_; }
};

#endif