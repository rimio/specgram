/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef _LIVE_HPP_
#define _LIVE_HPP_

#include <SFML/Graphics.hpp>

class LiveOutput {
private:
    /* dimensions and options */
    const std::size_t display_width_;
    const std::size_t history_size_;

    const std::string title_;

    /* SFML stuff */
    sf::RenderWindow window_;
    sf::RenderTexture window_texture_;
    sf::Texture fft_area_texture_;

    /* raw fft history */
    std::vector<uint8_t> fft_area_;

public:
    LiveOutput() = delete;
    LiveOutput(const LiveOutput &c) = delete;
    LiveOutput(LiveOutput &&) = delete;
    LiveOutput & operator=(const LiveOutput&) = delete;

    LiveOutput(std::size_t disp_width, std::size_t hist_size, const std::string& title);

    void AddWindow(const std::vector<uint8_t>& window);
    bool HandleEvents();
    void Render();
};

#endif