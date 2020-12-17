/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "renderer.hpp"
#include "share-tech-mono.hpp"

#include <cassert>
#include <cstring>

Renderer::Renderer(const Configuration& conf, std::size_t fft_count) : configuration_(conf), fft_count_(fft_count)
{
    /* load font */
    if (!this->font_.loadFromMemory(ShareTechMono_Regular_ttf, ShareTechMono_Regular_ttf_len)) {
        assert(false);
    }

    /* compute various sizes */
    this->width_ = conf.GetWidth();
    this->width_ += conf.HasAxes() ? 2 * conf.GetMarginSize() : 0;

    this->height_ = 0;

    /* compute origins */
    this->legend_origin_ = sf::Vector2f(0.0f, 0.0f);
    if (this->configuration_.HasLegend()) {
        if (this->configuration_.HasAxes()) {
            this->legend_origin_ += sf::Vector2f(conf.GetMarginSize(), conf.GetMarginSize());
            this->height_ += conf.GetMarginSize();
        }
        this->height_ += conf.GetLegendHeight();
    }

    this->fft_live_origin_ = sf::Vector2f(0.0f, this->height_);
    if (conf.HasLiveWindow()) {
        if (this->configuration_.HasAxes()) {
            this->fft_live_origin_ += sf::Vector2f(conf.GetMarginSize(), conf.GetMarginSize());
            this->height_ += conf.GetMarginSize();
        }
        this->height_ += conf.GetLiveFFTHeight();
    }

    this->fft_area_origin_ = sf::Vector2f(0.0f, this->height_);
    if (conf.HasAxes()) {
        this->fft_area_origin_ += sf::Vector2f(conf.GetMarginSize(), conf.GetMarginSize());
        this->height_ += conf.GetMarginSize() * 2;
    }
    this->height_ += this->fft_count_;

    /* allocate canvas render texture */
    this->canvas_.create(this->width_, this->height_);
    this->canvas_.clear(sf::Color(0, 0, 0, 255));

    /* allocate FFT area texture */
    this->fft_area_texture_.create(conf.GetWidth(), fft_count);
}

void
Renderer::RenderAxis(sf::RenderTexture& texture,
                     const sf::Vector2f& origin, float rot, float length,
                     float v_min, float v_max, std::string v_unit,
                     float est_tick_dist)
{
    assert(v_min < v_max);

    /* find factor */
    float v_diff = v_max - v_min;
    float px_per_unit = length / v_diff;
    float units_per_tick = est_tick_dist / px_per_unit;

    float factor = 1.0;

    sf::Text text;
    text.setCharacterSize(this->configuration_.GetLegendFontSize());
    text.setFont(this->font_);
    text.setString("Hello!");
    this->canvas_.draw(text);
}

void
Renderer::RenderUserInterface()
{
    if (this->configuration_.HasLegend()) {
        /* legend box */
        sf::RectangleShape legend_box(sf::Vector2f(this->configuration_.GetWidth(),
                                                   this->configuration_.GetLegendHeight()));
        legend_box.setFillColor(this->configuration_.GetBackgroundColor());
        legend_box.setOutlineColor(this->configuration_.GetAxesColor());
        legend_box.setOutlineThickness(1);
        legend_box.setPosition(this->legend_origin_);
        this->canvas_.draw(legend_box);
    }

    if (this->configuration_.HasAxes()) {
        /* FFT area box */
        sf::RectangleShape fft_area_box(sf::Vector2f(this->configuration_.GetWidth(), this->fft_count_));
        fft_area_box.setFillColor(this->configuration_.GetBackgroundColor());
        fft_area_box.setOutlineColor(this->configuration_.GetAxesColor());
        fft_area_box.setOutlineThickness(1);
        fft_area_box.setPosition(this->fft_area_origin_);
        this->canvas_.draw(fft_area_box);

        /* Frequency axis */
        this->RenderAxis(this->canvas_, this->fft_area_origin_, 0.0f, this->configuration_.GetWidth(),
                         this->configuration_.GetMinFreq(), this->configuration_.GetMaxFreq(),
                         "Hz", 50.0f);
    }

    this->canvas_.display();
}

void
Renderer::RenderFFTArea(const std::vector<uint8_t>& memory)
{
    assert(memory.size() == configuration_.GetWidth() * this->fft_count_ * 4);

    /* update FFT area texture */
    this->fft_area_texture_.update(reinterpret_cast<const uint8_t *>(memory.data()));

    /* render FFT area on canvas */
    sf::Sprite fft_area_sprite(this->fft_area_texture_);
    fft_area_sprite.setPosition(this->fft_area_origin_);

    this->canvas_.draw(fft_area_sprite);
    this->canvas_.display();
}

void
Renderer::RenderFFTArea(const std::list<std::vector<uint8_t>>& history)
{
    assert(history.size() == this->fft_count_);

    std::vector<uint8_t> memory;
    memory.resize(this->fft_count_ * this->configuration_.GetWidth() * 4);

    std::size_t i = 0;
    for (auto& win : history) {
        std::memcpy(reinterpret_cast<void *>(memory.data() + i * this->configuration_.GetWidth() * 4),
                    reinterpret_cast<const void *>(win.data()),
                    this->configuration_.GetWidth() * 4);
        i++;
    }

    return this->RenderFFTArea(memory);
}

void
Renderer::RenderLiveFFT(const std::vector<double>& window, const std::vector<uint8_t>& colors)
{
    assert(window.size() == this->configuration_.GetWidth());

    /* FFT live box (so we overwrite old one */
    sf::RectangleShape fft_live_box(sf::Vector2f(this->configuration_.GetWidth(),
                                                 this->configuration_.GetLiveFFTHeight() + 1.0f));
    fft_live_box.setFillColor(this->configuration_.GetBackgroundColor());
    fft_live_box.setOutlineColor(this->configuration_.GetAxesColor());
    fft_live_box.setOutlineThickness(1);
    fft_live_box.setPosition(this->fft_live_origin_);
    this->canvas_.draw(fft_live_box);

    /* graphic */
    std::vector<sf::Vertex> vertices;
    for (std::size_t i = 0; i < window.size(); i++) {
        float x = this->fft_live_origin_.x + i;
        float y = this->fft_live_origin_.y + (1.0f - window[i]) * this->configuration_.GetLiveFFTHeight();
        vertices.push_back(sf::Vertex(sf::Vector2f(x, y),
                                      sf::Color(colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2])));
    }

    this->canvas_.draw(reinterpret_cast<sf::Vertex *>(vertices.data()), vertices.size(), sf::LineStrip);

    this->canvas_.display();
}

void
Renderer::RenderLegend(const std::vector<uint8_t>& memory)
{
    assert(memory.size() == configuration_.GetWidth() * 4);

    sf::Texture tex;
    tex.create(this->configuration_.GetWidth(), 1);
    tex.update(reinterpret_cast<const uint8_t *>(memory.data()));
    sf::Sprite spr(tex);
    spr.setPosition(this->legend_origin_);
    spr.setScale(sf::Vector2f(1.0f, this->configuration_.GetLegendHeight()));

    this->canvas_.draw(spr);
    this->canvas_.display();
}

sf::Texture
Renderer::GetCanvas() const
{
    return this->canvas_.getTexture();
}