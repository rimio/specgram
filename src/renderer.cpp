/*
 * Copyright (c) 2020-2021 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "renderer.hpp"
#include "share-tech-mono.hpp"
#include "fft.hpp"

#include <iomanip>
#include <sstream>
#include <cassert>
#include <cstring>

static std::string
ValueToShortString(double value, int prec, const std::string& unit)
{
    static const std::vector<std::string> PREFIXES = { "p", "n", "u", "m", "", "k", "M", "G", "T" };
    std::size_t pidx = 4;

    while ((prec >= 3) && (pidx > 0)) {
        prec -= 3;
        pidx--;
        value *= 1000.0f;
    }
    while ((prec <= -3) && (pidx < PREFIXES.size() - 1)) {
        prec += 3;
        pidx++;
        value /= 1000.0f;
    }

    prec++;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(prec > 0 ? prec : 0) << value << PREFIXES[pidx] << unit;
    return ss.str();
}

Renderer::Renderer(const Configuration& conf, const ColorMap& cmap, const ValueMap& vmap, std::size_t fft_count)
    : configuration_(conf), fft_count_(fft_count)
{
    if (fft_count == 0) {
        throw std::runtime_error("positive number of FFT windows required by rendere");
    }

    /* load font */
    if (!this->font_.loadFromMemory(ShareTechMono_Regular_ttf, ShareTechMono_Regular_ttf_len)) {
        throw std::runtime_error("unable to load font");
    }

    /* compute tickmarks */
    auto freq_ticks = this->GetNiceTicks(this->configuration_.GetMinFreq(), this->configuration_.GetMaxFreq(),
                                         "Hz", this->configuration_.GetWidth(), 75);
    auto time_ticks = this->GetNiceTicks(0.0f, (double)fft_count * this->configuration_.GetFFTStride() / this->configuration_.GetRate(),
                                         "s", fft_count, 50);

    std::list<AxisTick> legend_ticks, live_ticks;
    if (vmap.GetName() == "dBFS") {
        unsigned int lticks = 1 + this->configuration_.GetWidth() / 60;
        lticks = std::clamp<unsigned int>(lticks, 2, 13); /* at maximum 10dBFS spacing */
        legend_ticks = this->GetLinearTicks(vmap.GetLowerBound(), vmap.GetUpperBound(), vmap.GetUnit(), lticks);
        live_ticks = this->GetLinearTicks(vmap.GetLowerBound(), vmap.GetUpperBound(), "", 5); /* no unit, keep it short */
    } else {
        legend_ticks = this->GetNiceTicks(vmap.GetLowerBound(), vmap.GetUpperBound(),
                                            vmap.GetUnit(), this->configuration_.GetWidth(), 60);
        live_ticks = this->GetNiceTicks(vmap.GetLowerBound(), vmap.GetUpperBound(),
                                          "", this->configuration_.GetLiveFFTHeight(), 30); /* no unit, keep it short */
    }

    typeof(freq_ticks) freq_no_text_ticks;
    for (auto& t : freq_ticks) {
        freq_no_text_ticks.emplace_back(std::make_tuple(std::get<0>(t), ""));
    }

    /* get maximum text widths */
    double max_freq_ticks_width = 0.0f;
    double max_freq_ticks_height = 0.0f;

    double max_time_ticks_width = 0.0f;
    double max_time_ticks_height = 0.0f;

    double max_legend_ticks_width = 0.0f;
    double max_legend_ticks_height = 0.0f;

    double max_live_ticks_width = 0.0f;
    double max_live_ticks_height = 0.0f;

    if (this->configuration_.HasAxes()) {
        for (auto &t : freq_ticks) {
            sf::Text text;
            text.setCharacterSize(this->configuration_.GetAxisFontSize());
            text.setFont(this->font_);
            text.setString(std::get<1>(t));
            if (text.getLocalBounds().width > max_freq_ticks_width) {
                max_freq_ticks_width = text.getLocalBounds().width;
            }
            if (text.getLocalBounds().height > max_freq_ticks_height) {
                max_freq_ticks_height = text.getLocalBounds().height;
            }
        }

        for (auto &t : time_ticks) {
            sf::Text text;
            text.setCharacterSize(this->configuration_.GetAxisFontSize());
            text.setFont(this->font_);
            text.setString(std::get<1>(t));
            if (text.getLocalBounds().width > max_time_ticks_width) {
                max_time_ticks_width = text.getLocalBounds().width;
            }
            if (text.getLocalBounds().height > max_time_ticks_height) {
                max_time_ticks_height = text.getLocalBounds().height;
            }
        }

        for (auto &t : legend_ticks) {
            sf::Text text;
            text.setCharacterSize(this->configuration_.GetAxisFontSize());
            text.setFont(this->font_);
            text.setString(std::get<1>(t));
            if (text.getLocalBounds().width > max_legend_ticks_width) {
                max_legend_ticks_width = text.getLocalBounds().width;
            }
            if (text.getLocalBounds().height > max_legend_ticks_height) {
                max_legend_ticks_height = text.getLocalBounds().height;
            }
        }

        for (auto &t : live_ticks) {
            sf::Text text;
            text.setCharacterSize(this->configuration_.GetAxisFontSize());
            text.setFont(this->font_);
            text.setString(std::get<1>(t));
            if (text.getLocalBounds().width > max_live_ticks_width) {
                max_live_ticks_width = text.getLocalBounds().width;
            }
            if (text.getLocalBounds().height > max_live_ticks_height) {
                max_live_ticks_height = text.getLocalBounds().height;
            }
        }
    }

    double freq_axis_spacing = (this->configuration_.IsHorizontal() ? max_freq_ticks_width : max_freq_ticks_height);
    double time_axis_spacing = (this->configuration_.IsHorizontal() ? max_time_ticks_height : max_time_ticks_width);
    double legend_axis_spacing = (this->configuration_.IsHorizontal() ? max_legend_ticks_width : max_legend_ticks_height);
    double live_axis_spacing = (this->configuration_.IsHorizontal() ? max_live_ticks_height : max_live_ticks_width);

    double horizontal_extra_spacing = std::max({ time_axis_spacing, live_axis_spacing });
    horizontal_extra_spacing =
        std::max<double>({ 0.0f, horizontal_extra_spacing - this->configuration_.GetMarginSize() + this->configuration_.GetMinimumMarginSize() });

    /* compute various sizes */
    this->width_ = conf.GetWidth();
    this->width_ += conf.HasAxes() ? 2 * conf.GetMarginSize() + static_cast<int>(horizontal_extra_spacing) : 0;
    this->height_ = 0;

    /* compute transforms */
    if (this->configuration_.HasLegend()) {
        if (this->configuration_.HasAxes()) {
            this->legend_transform_.translate(conf.GetMarginSize() + horizontal_extra_spacing,
                                              conf.GetMarginSize() + legend_axis_spacing);
            this->height_ += conf.GetMarginSize();
        }
        this->height_ += conf.GetLegendHeight() + legend_axis_spacing;
    }

    this->fft_live_transform_.translate(0.0f, this->height_);
    if (conf.HasLiveWindow()) {
        if (this->configuration_.HasAxes()) {
            this->fft_live_transform_.translate(conf.GetMarginSize() + horizontal_extra_spacing,
                                                conf.GetMarginSize());
            this->height_ += conf.GetMarginSize();
        }
        this->height_ += conf.GetLiveFFTHeight();
    }

    this->fft_area_transform_.translate(0.0f, this->height_);
    if (conf.HasAxes()) {
        this->fft_area_transform_.translate(conf.GetMarginSize() + horizontal_extra_spacing,
                                            conf.GetMarginSize() + freq_axis_spacing);
        this->height_ += conf.GetMarginSize() * 2 + freq_axis_spacing;
    }
    this->height_ += this->fft_count_;

    /* allocate canvas render texture */
    this->canvas_.create(this->width_, this->height_);
    this->canvas_.clear(this->configuration_.GetBackgroundColor());

    /* allocate FFT area texture */
    this->fft_area_texture_.create(conf.GetWidth(), fft_count);

    /*
     * render UI
     */

    /* render FFT area axes */
    if (this->configuration_.HasAxes()) {
        /* FFT area box */
        sf::RectangleShape fft_area_box(sf::Vector2f(this->configuration_.GetWidth(), this->fft_count_));
        fft_area_box.setFillColor(this->configuration_.GetBackgroundColor());
        fft_area_box.setOutlineColor(this->configuration_.GetForegroundColor());
        fft_area_box.setOutlineThickness(1);
        this->canvas_.draw(fft_area_box, this->fft_area_transform_);

        /* frequency axis */
        this->RenderAxis(this->canvas_, this->fft_area_transform_,
                         true, this->configuration_.IsHorizontal() ? Orientation::k90CW : Orientation::kNormal,
                         this->configuration_.GetWidth(), freq_ticks);

        /* time axis */
        this->RenderAxis(this->canvas_, this->fft_area_transform_ * sf::Transform().rotate(90.0f),
                         false, this->configuration_.IsHorizontal() ? Orientation::kNormal : Orientation::k90CCW,
                         fft_count, time_ticks);
    }

    if (this->configuration_.HasLegend()) {
        /* legend box */
        sf::RectangleShape legend_box(sf::Vector2f(this->configuration_.GetWidth(),
                                                   this->configuration_.GetLegendHeight()));
        legend_box.setFillColor(this->configuration_.GetBackgroundColor());
        legend_box.setOutlineColor(this->configuration_.GetForegroundColor());
        legend_box.setOutlineThickness(1);
        this->canvas_.draw(legend_box, this->legend_transform_);

        /* legend gradient */
        auto memory = cmap.Gradient(this->configuration_.GetWidth());
        sf::Texture tex;
        tex.create(this->configuration_.GetWidth(), 1);
        tex.update(reinterpret_cast<const uint8_t *>(memory.data()));
        this->canvas_.draw(sf::Sprite(tex), this->legend_transform_ * sf::Transform().scale(1.0f, this->configuration_.GetLegendHeight()));

        if (this->configuration_.HasAxes()) {
            this->RenderAxis(this->canvas_, this->legend_transform_,
                             true, this->configuration_.IsHorizontal() ? Orientation::k90CW : Orientation::kNormal,
                             this->configuration_.GetWidth(), legend_ticks);
        }
    }

    if (this->configuration_.HasLiveWindow() && this->configuration_.HasAxes()) {
        /* value axis */
        this->RenderAxis(this->canvas_,
                         this->fft_live_transform_ * sf::Transform().translate(0.0f, this->configuration_.GetLiveFFTHeight()).rotate(-90.0f),
                         true, this->configuration_.IsHorizontal() ? Orientation::k180 : Orientation::k90CW,
                         this->configuration_.GetLiveFFTHeight(), live_ticks);

        /* frequency axis */
        this->RenderAxis(this->canvas_, this->fft_live_transform_ * sf::Transform().translate(0.0f, this->configuration_.GetLiveFFTHeight()),
                         false, this->configuration_.IsHorizontal() ? Orientation::k90CW : Orientation::kNormal,
                         this->configuration_.GetWidth(), freq_no_text_ticks);
    }
}

std::list<AxisTick>
Renderer::GetLinearTicks(double v_min, double v_max, const std::string& v_unit, unsigned int num_ticks)
{
    if (num_ticks <= 1) {
        throw std::runtime_error("GetLinearTicks() requires at least two ticks");
    }
    if (v_min >= v_max) {
        throw std::runtime_error("minimum and maximum values are not in order");
    }

    int prec = 0;
    double dist = (v_max - v_min) / ((double) num_ticks - 1);
    while (dist >= 10.0f) {
        prec--;
        dist /= 10.0f;
    }
    while (dist < 1.0f) {
        prec++;
        dist *= 10.0f;
    }

    std::list<AxisTick> ticks;
    for (unsigned int i = 0; i < num_ticks; i ++) {
        double k = (double) i / (double)(num_ticks - 1);
        double v = v_min + k * (v_max - v_min);
        ticks.emplace_back(std::make_tuple(k, ::ValueToShortString(v, prec, v_unit)));
    }

    return ticks;
}

std::list<AxisTick>
Renderer::GetNiceTicks(double v_min, double v_max, const std::string& v_unit, unsigned int length_px,
                       unsigned int est_tick_length_px)
{
    if (v_min >= v_max) {
        throw std::runtime_error("minimum and maximum values are not in order");
    }
    if (length_px == 0) {
        throw std::runtime_error("length in pixels must be positive");
    }
    if (est_tick_length_px == 0) {
        throw std::runtime_error("estimate tick length in pixels must be positive");
    }

    std::list<AxisTick> ticks;

    /* find a factor that brings some span close to est_tick_length to a nice value */
    double v_diff = v_max - v_min;
    double px_per_v = length_px / v_diff;

    double mdist = 1e12, mfact;

    constexpr double NICE_FACTORS[] = { 0.15f, 0.2f, 0.25f, 0.3f, 0.5f };
    for (double f = 1e-15; f < 1e+15; f *= 10.0f) {
        for (unsigned int k = 0; k < 5; k++) {
            double factor = f * NICE_FACTORS[k];
            double dist = std::abs(est_tick_length_px - px_per_v * factor);
            if (dist < mdist) {
                mdist = dist;
                mfact = factor;
            }
        }
    }

    /* compute precision */
    int prec = 0;
    double dist = mfact;
    while (dist > 10.0f) {
        prec--;
        dist /= 10.0f;
    }
    while (dist < 1.0f) {
        prec++;
        dist *= 10.0f;
    }

    /* find the first nice value */
    double fval = v_min / mfact;
    constexpr double ROUND_EPSILON = 1e-6;
    if (std::abs(fval - std::floor(fval)) > ROUND_EPSILON) {
        fval = std::floor(fval) + 1.0f;
    }
    fval *= mfact;
    assert(v_min <= fval);
    assert(fval <= v_max);

    /* add ticks */
    for (double value = fval; value < v_max; value += mfact) {
        double k = (value - v_min) / (v_max - v_min);
        ticks.emplace_back(std::make_tuple(k, ::ValueToShortString(value, prec, v_unit)));
    }

    return ticks;
}

void
Renderer::RenderAxis(sf::RenderTexture& texture,
                     const sf::Transform& t, bool lhs, Orientation orientation, double length,
                     const std::list<AxisTick>& ticks)
{
    if (length <= 0.0f) {
        throw std::runtime_error("positive axis length required for rendering");
    }

    for (auto& tick : ticks) {
        /* draw tick line */
        double x = (length - 1) * std::get<0>(tick);
        sf::RectangleShape tick_shape(sf::Vector2f(1.0, (lhs ? -5.0f : 5.0f)));
        tick_shape.setFillColor(this->configuration_.GetForegroundColor());
        texture.draw(tick_shape, t * sf::Transform().translate(x, 0.0f));

        /* draw text */
        sf::Text text;
        text.setFillColor(this->configuration_.GetForegroundColor());
        text.setCharacterSize(this->configuration_.GetAxisFontSize());
        text.setFont(this->font_);
        text.setString(std::get<1>(tick));

        sf::Vector2f pos;
        switch (orientation) {
            case Orientation::k90CCW:
                pos = sf::Vector2f(sf::Vector2f(length * std::get<0>(tick) - text.getLocalBounds().height,
                                                (lhs ? -10.0f : text.getLocalBounds().width + 10.0f)));
                text.setRotation(-90.0f);
                break;

            case Orientation::k90CW:
                pos = sf::Vector2f(sf::Vector2f(length * std::get<0>(tick) + text.getLocalBounds().height,
                                                (lhs ? -text.getLocalBounds().width - 10.0f : 10.0f)));
                text.setRotation(90.0f);
                break;

            case Orientation::kNormal:
                pos = sf::Vector2f(sf::Vector2f(length * std::get<0>(tick) - text.getLocalBounds().width / 2,
                                                (lhs ? -2.0f * text.getLocalBounds().height - 3.0f : 3.0f)));
                break;

            case Orientation::k180:
                pos = sf::Vector2f(sf::Vector2f(length * std::get<0>(tick) + text.getLocalBounds().width / 2,
                                                (lhs ? -3.0f : 2.0f * text.getLocalBounds().height + 3.0f)));
                text.setRotation(180.0f);
                break;

            default:
                throw std::runtime_error("unknown orientation");
        }

        text.setPosition(std::round(pos.x), std::round(pos.y)); /* avoid interpolation on text, looks yuck */
        texture.draw(text, t);
    }
}

void
Renderer::RenderFFTArea(const std::vector<uint8_t>& memory)
{
    if (memory.size() != configuration_.GetWidth() * this->fft_count_ * 4) {
        throw std::runtime_error("bad memory size");
    }

    /* update FFT area texture */
    this->fft_area_texture_.update(reinterpret_cast<const uint8_t *>(memory.data()));

    /* render FFT area on canvas */
    this->canvas_.draw(sf::Sprite(this->fft_area_texture_), this->fft_area_transform_);
}

void
Renderer::RenderFFTArea(const std::list<std::vector<uint8_t>>& history)
{
    if (history.size() != this->fft_count_) {
        throw std::runtime_error("bad history size");
    }

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
Renderer::RenderLiveFFT(const RealWindow& window, const std::vector<uint8_t>& colors)
{
    if (window.size() != this->configuration_.GetWidth()) {
        throw std::runtime_error("incorrect window size to be rendered");
    }

    if (!this->configuration_.HasLiveWindow()) {
        /* noop */
        return;
    }

    /* FFT live box (so we overwrite old one */
    sf::RectangleShape fft_live_box(sf::Vector2f(this->configuration_.GetWidth(),
                                                 this->configuration_.GetLiveFFTHeight() + 1.0f));
    fft_live_box.setFillColor(this->configuration_.GetBackgroundColor());
    fft_live_box.setOutlineColor(this->configuration_.GetForegroundColor());
    fft_live_box.setOutlineThickness(1);
    this->canvas_.draw(fft_live_box, this->fft_live_transform_);

    /* plot */
    std::vector<sf::Vertex> vertices;
    vertices.resize(window.size());
    for (std::size_t i = 0; i < window.size(); i++) {
        double x = i;
        double y = (1.0f - window[i]) * this->configuration_.GetLiveFFTHeight();
        vertices[i] = sf::Vertex(sf::Vector2f(x, y),
                                 sf::Color(colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2]));
    }
    this->canvas_.draw(reinterpret_cast<sf::Vertex *>(vertices.data()), vertices.size(),
                       sf::LineStrip, this->fft_live_transform_);

}

sf::Texture
Renderer::GetCanvas()
{
    this->canvas_.display();
    return this->canvas_.getTexture();
}