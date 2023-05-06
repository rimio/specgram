/*
 * Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
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

static double compute_error_for_scale(double v, int scale, double v_min, double v_max)
{
    double rsv = v * std::pow(10, scale);
    double sv = std::round(rsv);
    return std::abs(rsv - sv) / std::pow(10, scale) / (v_max - v_min);
}

std::string
Renderer::ValueToShortString(double value, int scale, const std::string& unit)
{
    static const std::vector<std::string> PREFIXES = { "p", "n", "u", "m", "", "k", "M", "G", "T" };
    std::size_t pidx = 4;

    while ((scale >= 3) && (pidx > 0)) {
        scale -= 3;
        pidx--;
        value *= 1000.0f;
    }
    while ((scale <= -3) && (pidx < PREFIXES.size() - 1)) {
        scale += 3;
        pidx++;
        value /= 1000.0f;
    }

    /* round very low values to zero, as to avoid printing "-0" */
    /* theoretically this function should not be asked to print values as small as 1e-9 */
    if (std::abs(value) < 1e-9) {
        value = 0.0;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(scale > 0 ? scale : 0) << value << PREFIXES[pidx] << unit;
    return ss.str();
}

Renderer::Renderer(const Configuration& conf, std::size_t fft_count)
    : configuration_(conf)
    , fft_count_(fft_count)
    , color_map_(ColorMap::Build(conf.GetColorMap(), conf.GetBackgroundColor(),
                                 conf.GetColorMapCustomColor()))
{
    if (color_map_ == nullptr) {
        throw std::runtime_error("failed to build colormap");
    }
    if (fft_count == 0) {
        throw std::runtime_error("positive number of FFT windows required by renderer");
    }

    /* load font */
    if (!this->font_.loadFromMemory(ShareTechMono_Regular_ttf, ShareTechMono_Regular_ttf_len)) {
        throw std::runtime_error("unable to load font");
    }

    /* compute tickmarks */
    this->frequency_ticks_ =
        Renderer::GetNiceTicks(this->configuration_.GetMinFreq(), this->configuration_.GetMaxFreq(),
                               "Hz", this->configuration_.GetWidth(), 50, this->configuration_.IsHorizontal());
    auto time_ticks =
        Renderer::GetNiceTicks(0.0f, (double)fft_count * this->configuration_.GetAverageCount() * this->configuration_.GetFFTStride() / this->configuration_.GetRate(),
                               "s", fft_count, 30, !this->configuration_.IsHorizontal());

    auto vmap = ValueMap::Build(conf.GetScaleType(),
                                conf.GetScaleLowerBound(),
                                conf.GetScaleUpperBound(),
                                conf.GetScaleUnit());
    auto legend_ticks = Renderer::GetNiceTicks(vmap->GetLowerBound(), vmap->GetUpperBound(),
                                               vmap->GetUnit(), this->configuration_.GetWidth(), 50,
                                               this->configuration_.IsHorizontal());
    this->live_ticks_ = Renderer::GetNiceTicks(vmap->GetLowerBound(), vmap->GetUpperBound(),
                                               "", this->configuration_.GetLiveFFTHeight(), 20,
                                               !this->configuration_.IsHorizontal()); /* no unit, keep it short */

    std::list<AxisTick> freq_no_text_ticks;
    for (auto& t : this->frequency_ticks_) {
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
        for (auto &t : this->frequency_ticks_) {
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

        for (auto &t : this->live_ticks_) {
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
            this->height_ += conf.GetMarginSize() + legend_axis_spacing;
        }
        this->height_ += conf.GetLegendHeight();
    }

    this->live_transform_.translate(0.0f, this->height_);
    if (conf.HasLiveWindow()) {
        if (this->configuration_.HasAxes()) {
            this->live_transform_.translate(conf.GetMarginSize() + horizontal_extra_spacing,
                                            conf.GetMarginSize());
            this->height_ += conf.GetLiveMarginSize();
        }
        this->height_ += conf.GetLiveFFTHeight();
    }

    this->spectrogram_transform_.translate(0.0f, this->height_);
    if (conf.HasAxes()) {
        this->spectrogram_transform_.translate(conf.GetMarginSize() + horizontal_extra_spacing,
                                               conf.GetMarginSize() + freq_axis_spacing);
        this->height_ += conf.GetMarginSize() * 2 + freq_axis_spacing;
    }
    this->height_ += this->fft_count_;

    /* allocate canvas render texture */
    this->canvas_.create(this->width_, this->height_);
    this->canvas_.clear(this->configuration_.GetBackgroundColor());

    /* allocate FFT area texture */
    this->spectrogram_texture_.create(conf.GetWidth(), fft_count);

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
        this->canvas_.draw(fft_area_box, this->spectrogram_transform_);

        /* frequency axis */
        this->RenderAxis(this->canvas_, this->spectrogram_transform_,
                         true, this->configuration_.IsHorizontal() ? Orientation::k90CW : Orientation::kNormal,
                         this->configuration_.GetWidth(), this->frequency_ticks_);

        /* time axis */
        this->RenderAxis(this->canvas_, this->spectrogram_transform_ * sf::Transform().rotate(90.0f),
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
        auto memory = color_map_->Gradient(this->configuration_.GetWidth());
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
                         this->live_transform_ * sf::Transform().translate(0.0f, this->configuration_.GetLiveFFTHeight()).rotate(-90.0f),
                         true, this->configuration_.IsHorizontal() ? Orientation::k180 : Orientation::k90CW,
                         this->configuration_.GetLiveFFTHeight(), this->live_ticks_);

        /* frequency axis */
        this->RenderAxis(this->canvas_, this->live_transform_ * sf::Transform().translate(0.0f, this->configuration_.GetLiveFFTHeight()),
                         false, this->configuration_.IsHorizontal() ? Orientation::k90CW : Orientation::kNormal,
                         this->configuration_.GetWidth(), freq_no_text_ticks);
    }
}

[[maybe_unused]] std::list<Renderer::AxisTick>
Renderer::GetLinearTicks(double v_min, double v_max, const std::string& v_unit, unsigned int num_ticks)
{
    if (num_ticks <= 1) {
        throw std::runtime_error("requires at least two ticks");
    }
    if (v_min >= v_max) {
        throw std::runtime_error("minimum and maximum values are not in order");
    }

    /* find a scale that advances each tick value in the single digits */
    int scale = 0;
    double dist = (v_max - v_min) / ((double) num_ticks - 1);
    while (dist >= 10.0f) {
        scale--;
        dist /= 10.0f;
    }
    while (dist < 1.0f) {
        scale++;
        dist *= 10.0f;
    }

    /* still, if the error of this scale is comparable to the input domain, add one more decimal place */
    for (unsigned int i = 0; i < num_ticks; i ++) {
        double k = (double) i / (double)(num_ticks - 1);
        double v = v_min + k * (v_max - v_min);

        if (::compute_error_for_scale(v, scale, v_min, v_max) > 0.01) { /* greater than 1% => one more decimal place */
            scale ++;
            break;
        }
    }

    std::list<AxisTick> ticks;
    for (unsigned int i = 0; i < num_ticks; i ++) {
        double k = (double) i / (double)(num_ticks - 1);
        double v = v_min + k * (v_max - v_min);
        ticks.emplace_back(std::make_tuple(k, ValueToShortString(v, scale, v_unit)));
    }

    return ticks;
}

std::list<Renderer::AxisTick>
Renderer::GetNiceTicks(double v_min, double v_max, const std::string& v_unit, unsigned int length_px,
                       unsigned int min_tick_length_px, bool rotated)
{
    if (v_min >= v_max) {
        throw std::runtime_error("minimum and maximum values are not in order");
    }
    if (length_px == 0) {
        throw std::runtime_error("length in pixels must be positive");
    }
    if (min_tick_length_px == 0) {
        throw std::runtime_error("minimum tick length in pixels must be positive");
    }

    std::list<AxisTick> ticks;

    /* domain span */
    double v_diff = v_max - v_min;

    /* pixels per unit of domain */
    double px_per_v = length_px / v_diff;

    /* finds a factor that brings some span close to est_tick_length to a nice value */
    auto find_factor = [px_per_v](unsigned int min_len) -> double
    {
        double mdist = 1e12, mfact = 1.0;

        constexpr double NICE_FACTORS[] = { 0.15, 0.2, 0.25, 0.3, 0.5, 1.0 };
        for (double f = 1e-15; f < 1e+15; f *= 10.0f) {
            for (double nf : NICE_FACTORS) {
                double factor = f * nf;
                double dist = px_per_v * factor - min_len;
                if (dist < 0) {
                    continue; /* nothing below minimum length allowed */
                }
                if (dist < mdist) {
                    mdist = dist;
                    mfact = factor;
                }
            }
        }
        return mfact;
    };

    /* computes scale */
    auto compute_scale = [](double factor) -> int
    {
        int scale = 0;
        double dist = factor;
        while (dist > 10.0f) {
            scale--;
            dist /= 10.0f;
        }
        while (dist < 1.0f) {
            scale++;
            dist *= 10.0f;
        }
        return scale;
    };

    /* computes text width */
    auto compute_text_size = [this, rotated](const std::string& str) -> double
    {
        sf::Text text;
        text.setCharacterSize(this->configuration_.GetAxisFontSize());
        text.setFont(this->font_);
        text.setString(str);
        return (rotated ? text.getLocalBounds().height : text.getLocalBounds().width);
    };

    /* find the first nice value */
    auto find_first_value = [v_min, v_max](double factor) -> double { ;
        double fval = v_min / factor;
        constexpr double ROUND_EPSILON = 1e-6;
        if (std::abs(fval - std::floor(fval)) > ROUND_EPSILON) {
            fval = std::floor(fval) + 1.0f;
        }
        fval *= factor;

        if ((fval < v_min) || (fval > v_max)) {
            /* there is no nice value here; usually this happens because the
             * axis length is too small */
            return v_min;
        }

        return fval;
    };

    /* find a factor and a precision for the ticks */
    double factor = 1.0;
    int scale = 0;
    unsigned int target_tick_length = min_tick_length_px;

    constexpr std::size_t MAXIMUM_ITERATIONS = 10;
    std::size_t iteration = 0; /* theoretically the below loop will stop; practically, we don't take chances */
    double fval = 0.0; /* first nice value */

    /* adjust upper limit with a slight epsilon so that we have tickmark for v_max in most "nice" cases */
    /* otherwise we might miss it because of representation errors */
    double upper_limit = v_max + (v_max - v_min) * 1e-6;

    do {
        /* compute a nice factor and get the actual tick length */
        factor = find_factor(target_tick_length);
        auto actual_tick_length = px_per_v * factor;
        scale = compute_scale(factor);
        fval = find_first_value(factor);

        /* see if we need another decimal place */
        for (double value = fval; value <= upper_limit; value += factor) {
            if (::compute_error_for_scale(value, scale, v_min, v_max) > 0.005) { /* greater than .5% => one more decimal place */
                scale ++;
                break;
            }
        }

        /* make sure the tick length is higher than the label sizes for this precision */
        constexpr double min_tick_spacing = 5.0;
        auto lb_size = compute_text_size(ValueToShortString(v_min, scale, v_unit)) + min_tick_spacing;
        auto ub_size = compute_text_size(ValueToShortString(v_max, scale, v_unit)) + min_tick_spacing;
        if ((lb_size > actual_tick_length) || (ub_size > actual_tick_length)) {
            target_tick_length = std::max( { static_cast<unsigned int>(lb_size),
                                             static_cast<unsigned int>(ub_size),
                                             target_tick_length } );
        } else {
            break; /* a fitting factor was found */
        }

        iteration++;
    } while (iteration < MAXIMUM_ITERATIONS); /* maybe we should issue warning if we reach max iterations? */

    /* add ticks */
    for (double value = fval; value <= upper_limit; value += factor) {
        double k = (value - v_min) / (v_max - v_min);
        ticks.emplace_back(std::make_tuple(k, ValueToShortString(value, scale, v_unit)));
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
    this->spectrogram_texture_.update(reinterpret_cast<const uint8_t *>(memory.data()));

    /* render FFT area on canvas */
    this->canvas_.draw(sf::Sprite(this->spectrogram_texture_), this->spectrogram_transform_);
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

std::vector<uint8_t>
Renderer::RenderLiveFFT(const RealWindow& window)
{
    if (window.size() != this->configuration_.GetWidth()) {
        throw std::runtime_error("incorrect window size to be rendered");
    }
    if (!this->configuration_.HasLiveWindow()) {
        throw std::runtime_error("asked to render live window for non-live configuration");
    }

    std::vector<uint8_t> colors = this->color_map_->Map(window);

    /* FFT live box (so we overwrite old one */
    sf::RectangleShape fft_live_box(sf::Vector2f(this->configuration_.GetWidth(),
                                                 this->configuration_.GetLiveFFTHeight() + 1.0f));
    fft_live_box.setFillColor(this->configuration_.GetBackgroundColor());
    fft_live_box.setOutlineColor(this->configuration_.GetForegroundColor());
    fft_live_box.setOutlineThickness(1);
    this->canvas_.draw(fft_live_box, this->live_transform_);

    /* horizontal live guidelines */
    for (std::size_t i = 0; i < this->live_ticks_.size(); i ++) {
        sf::RectangleShape hline(sf::Vector2f(this->configuration_.GetWidth(), 1.0f));
        hline.setFillColor(this->configuration_.GetLiveGuidelinesColor());

        sf::Transform tran;
        tran.translate(0.0f, (1.0 - std::get<0>(*std::next(this->live_ticks_.begin(), i))) * (this->configuration_.GetLiveFFTHeight() - 1.0f));
        this->canvas_.draw(hline, this->live_transform_ * tran);
    }

    /* vertical live guidelines */
    for (std::size_t i = 0; i < this->frequency_ticks_.size(); i ++) {
        sf::RectangleShape vline(sf::Vector2f(1.0f, this->configuration_.GetLiveFFTHeight()));
        vline.setFillColor(this->configuration_.GetLiveGuidelinesColor());

        sf::Transform tran;
        tran.translate(std::get<0>(*std::next(this->frequency_ticks_.begin(), i)) * (this->configuration_.GetWidth() - 1.0f), 0.0f);
        this->canvas_.draw(vline, this->live_transform_ * tran);
    }

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
                       sf::LineStrip, this->live_transform_);

    return colors;
}

sf::Texture
Renderer::GetCanvas()
{
    this->canvas_.display();
    return this->canvas_.getTexture();
}
