/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _LIVE_HPP_
#define _LIVE_HPP_

#include "configuration.hpp"
#include "renderer.hpp"
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

    LiveOutput(const Configuration& conf);

    void AddWindow(const std::vector<uint8_t>& window);
    bool HandleEvents();
    void Render();
};

#endif