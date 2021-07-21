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

/**
 * Live output God object, keeping track of rendering, window, history etc.
 */
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

    /**
     * @param conf Configuration to use.
     * @param cmap Color map instance to use.
     * @param vmap Value map instance to use.
     */
    LiveOutput(const Configuration& conf, const ColorMap& cmap, const ValueMap& vmap);

    /**
     * Add a FFT window to the history and render it.
     * @param win_values Window values, real, scaled.
     * @return A copy of the colorized window that is rendered.
     */
    std::vector<uint8_t> AddWindow(const RealWindow& win_values);

    /**
     * Handle window events.
     * @return True if window was closed.
     */
    bool HandleEvents();

    /**
     * Render live window.
     */
    void Render();
};

#endif