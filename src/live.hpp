/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _LIVE_HPP_
#define _LIVE_HPP_

#include "configuration.hpp"
#include "renderer.hpp"
#include "color-map.hpp"
#include <SFML/Graphics.hpp>

class LiveOutput {
private:
    /* configuration */
    const Configuration configuration_;
    Renderer renderer_;

    /* live window */
    sf::RenderWindow window_;

    /* raw FFT history */
    std::vector<uint8_t> fft_area_;

public:
    LiveOutput() = delete;
    LiveOutput(const LiveOutput &c) = delete;
    LiveOutput(LiveOutput &&) = delete;
    LiveOutput & operator=(const LiveOutput&) = delete;

    LiveOutput(const Configuration& conf, const ColorMap& cmap, const ValueMap& vmap);

    void AddWindow(const std::vector<uint8_t>& window, const RealWindow& win_values);
    bool HandleEvents();
    void Render();
};

#endif