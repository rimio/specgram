/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "live.hpp"

#include <cassert>
#include <cstring>

LiveOutput::LiveOutput(std::size_t disp_width, std::size_t hist_size, const std::string& title)
    : display_width_(disp_width), history_size_(hist_size), title_(title)
{
    this->window_.create(sf::VideoMode(disp_width, hist_size), title, sf::Style::Close);
    this->window_.setFramerateLimit(0);
    this->window_texture_.create(disp_width, hist_size);
    this->fft_area_texture_.create(disp_width, hist_size);

    this->fft_area_.resize(disp_width * hist_size * 4); /* RGBA pixel array */
}

void
LiveOutput::AddWindow(const std::vector<uint8_t>& window)
{
    std::size_t wlen_bytes = this->display_width_ * 4;
    assert(window.size() == wlen_bytes);
    /* scroll down one window */
    std::memmove(reinterpret_cast<void *>(this->fft_area_.data() + wlen_bytes),
                 reinterpret_cast<const void *>(this->fft_area_.data()),
                 fft_area_.size() - wlen_bytes);
    /* copy new window */
    std::memcpy(reinterpret_cast<void *>(this->fft_area_.data()),
                reinterpret_cast<const void *>(window.data()),
                wlen_bytes);

    /* recreate FFT area texture */
    this->fft_area_texture_.update(this->fft_area_.data());
    /* render window texture again */
    this->window_texture_.draw(sf::Sprite(this->fft_area_texture_));
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
    this->window_.clear();
    this->window_.draw(sf::Sprite(this->window_texture_.getTexture()));
    this->window_.display();
}