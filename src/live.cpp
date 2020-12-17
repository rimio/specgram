/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "live.hpp"

#include <cassert>
#include <cstring>

LiveOutput::LiveOutput(const Configuration& conf, const ColorMap& cmap)
    : configuration_(conf), renderer_(conf.GetForLive(), conf.GetCount())
{
    auto width = conf.IsHorizontal() ? renderer_.GetHeight() : renderer_.GetWidth();
    auto height = conf.IsHorizontal() ? renderer_.GetWidth() : renderer_.GetHeight();

    /* create non-resizable window */
    this->window_.create(sf::VideoMode(width, height),
                         conf.GetTitle(),
                         sf::Style::Close);
    this->window_.setFramerateLimit(0);

    /* FFT area raw memory used to update the area texture */
    this->fft_area_.resize(conf.GetWidth() * conf.GetCount() * 4); /* RGBA pixel array */

    /* render UI */
    this->renderer_.RenderUserInterface();

    /* render legend */
    std::vector<double> legend_values;
    for (std::size_t i = 0; i < conf.GetWidth(); i++) {
        legend_values.push_back((double) i / (double) (conf.GetWidth()-1));
    }
    auto legend_colors = cmap.Map(legend_values);
    this->renderer_.RenderLegend(legend_colors);
}

void
LiveOutput::AddWindow(const std::vector<uint8_t>& window, const std::vector<double>& win_values)
{
    std::size_t wlen_bytes = this->configuration_.GetWidth() * 4;
    assert(window.size() == wlen_bytes);
    /* scroll down one window */
    std::memmove(reinterpret_cast<void *>(this->fft_area_.data() + wlen_bytes),
                 reinterpret_cast<const void *>(this->fft_area_.data()),
                 fft_area_.size() - wlen_bytes);
    /* copy new window */
    std::memcpy(reinterpret_cast<void *>(this->fft_area_.data()),
                reinterpret_cast<const void *>(window.data()),
                wlen_bytes);

    /* update renderer */
    renderer_.RenderFFTArea(this->fft_area_);
    renderer_.RenderLiveFFT(win_values);

    /* draw window */
    this->Render();
}

bool
LiveOutput::HandleEvents()
{
    sf::Event event;
    while (this->window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            this->window_.close();
    }
    return this->window_.isOpen();
}

void
LiveOutput::Render()
{
    sf::Texture canvas_texture = this->renderer_.GetCanvas();
    sf::Sprite canvas_sprite(canvas_texture);
    if (this->configuration_.IsHorizontal()) {
        canvas_sprite.setRotation(-90.0f);
        canvas_sprite.setPosition(0.0f, canvas_texture.getSize().y);
    }

    this->window_.clear();
    this->window_.draw(canvas_sprite);
    this->window_.display();
}