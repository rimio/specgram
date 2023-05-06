/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "live.hpp"

#include <cassert>
#include <cstring>

LiveOutput::LiveOutput(const Configuration& conf)
    : width_(conf.GetWidth())
    , is_horizontal_(conf.IsHorizontal())
    , renderer_(conf.GetForLive(), conf.GetCount())
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

    /* render the live FFT area with a DC signal of zero */
    Render(); /* I don't have a good idea why this is needed, but I guess it has something to do with double buffering */
    this->renderer_.RenderFFTArea(this->fft_area_);
    this->renderer_.RenderLiveFFT(RealWindow(conf.GetWidth()));
}

std::vector<uint8_t>
LiveOutput::AddWindow(const RealWindow& win_values)
{
    auto window = this->renderer_.RenderLiveFFT(win_values);
    std::size_t wlen_bytes = this->width_ * 4;
    if (window.size() != wlen_bytes) {
        throw std::runtime_error("input window size differs from live window size");
    }

    /* scroll down one window */
    assert(this->fft_area_.size() >= wlen_bytes);
    std::memmove(reinterpret_cast<void *>(this->fft_area_.data() + wlen_bytes),
                 reinterpret_cast<const void *>(this->fft_area_.data()),
                 fft_area_.size() - wlen_bytes);
    /* copy new window */
    std::memcpy(reinterpret_cast<void *>(this->fft_area_.data()),
                reinterpret_cast<const void *>(window.data()),
                wlen_bytes);

    /* update renderer texture */
    this->renderer_.RenderFFTArea(this->fft_area_);
    return window;
}

bool
LiveOutput::HandleEvents()
{
    sf::Event event;
    while (this->window_.pollEvent(event)) {
        if ((event.type == sf::Event::Closed)
            || (event.type == sf::Event::KeyPressed
                && event.key.code == sf::Keyboard::Escape)) {
            this->window_.close();
        }
    }
    return this->window_.isOpen();
}

void
LiveOutput::Render()
{
    /* draw renderer output to window */
    sf::Texture canvas_texture = this->renderer_.GetCanvas();
    sf::Sprite canvas_sprite(canvas_texture);
    if (this->is_horizontal_) {
        canvas_sprite.setRotation(-90.0f);
        canvas_sprite.setPosition(0.0f, canvas_texture.getSize().x);
    }

    this->window_.draw(canvas_sprite);
    this->window_.display();
}
