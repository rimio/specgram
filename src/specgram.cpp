/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "configuration.hpp"
#include "input.hpp"
#include "fft.hpp"
#include "live.hpp"
#include "colormap.hpp"

#include <spdlog/spdlog.h>
#include <iostream>
#include <csignal>
#include <list>

/* main loop exit condition */
volatile bool main_loop_running = true;

/*
 * SIGINT handler
 */
void
sigint_handler(int)
{
    /* no longer loop */
    main_loop_running = false;

    /* uninstall handler in case user REALLY does not want to wait for wrap-up */
    std::signal(SIGINT, nullptr);
}

/*
 * entry point
 */
int
main(int argc, char** argv)
{
    /* parse command line arguments into global settings */
    auto [conf, conf_rc, conf_must_exit] = Configuration::FromArgs(argc, argv);
    if (conf_must_exit) {
        return conf_rc;
    }

    /* create FFT */
    FFT fft(conf.GetFFTWidth(), conf.GetWindowFunction());

    /* create value map */
    std::unique_ptr<ValueMap> value_map = nullptr;
    if (conf.GetScale() == kdBFS) {
        /* TODO: configurable lower bound */
        value_map = std::make_unique<dBFSValueMap>(-120);
    } else {
        assert(false);
        spdlog::error("Internal error: unknown scale");
        return 1;
    }

    /* create color map */
    auto color_map = ColorMap::FromType(conf.GetColorMap());

    /* create live window */
    std::unique_ptr<LiveOutput> live = nullptr;
    if (conf.IsLive()) {
        live = std::make_unique<LiveOutput>(conf, *color_map);
    }

    /* create input parser */
    auto input = InputParser::FromDataType(conf.GetDataType());
    if (input == nullptr) {
        return 1;
    }

    /* display initialization info */
    if (input->IsComplex()) {
        spdlog::info("Input stream: {}bit complex at {}Hz", input->GetDataTypeSize() * 8, conf.GetRate());
    } else if (input->IsFloatingPoint()) {
        spdlog::info("Input stream: {}bit floating point at {}Hz", input->GetDataTypeSize() * 8, conf.GetRate());
    } else {
        spdlog::info("Input stream: {} {}bit integer at {}Hz",
                     input->IsSigned() ? "signed" : "unsigned",
                     input->GetDataTypeSize() * 8, conf.GetRate());
    }

    /* create input reader */
    InputReader reader(std::cin, input->GetDataTypeSize() * conf.GetBlockSize());

    /* install SIGINT handler for CTRL+C */
    std::signal(SIGINT, sigint_handler);

    /* FFT window history */
    std::list<std::vector<uint8_t>> history;

    /* main loop */
    while (main_loop_running) {
        /* check for window events (if necessary) */
        if (live != nullptr) {
            if (!live->HandleEvents()) {
                /* exited by closing window */
                main_loop_running = false;
                /* uninstall signal so that reader thread can exit successfully */
                std::signal(SIGINT, nullptr);
            }
        }

        /* check for a complete block */
        auto block = reader.GetBlock();
        if (!block) {
            /* block not finished yet */
            continue;
        }

        /* take whatever is available from input stream */
        auto pvc = input->ParseBlock(*block);
        assert(pvc == block->size() / input->GetDataTypeSize());

        /* check if we have enough for a new FFT window */
        if ((input->GetBufferedValueCount() < conf.GetFFTWidth())
            || (input->GetBufferedValueCount() < conf.GetFFTStride())) {
            /* wait until we get enough values for a window and the spacing between windows */
            continue;
        }

        /* retrieve window and remove values that won't be used further */
        auto window_values = input->PeekValues(conf.GetFFTWidth());
        input->RemoveValues(conf.GetFFTStride());

        /* compute FFT on fetched window */
        auto fft_values = fft.Compute(window_values, conf.IsAliasingNegativeFrequencies());

        /* map FFT to [0..1] domain */
        auto fft_normalized_values = value_map->Map(fft_values);

        if (conf.CanResample()) {
            /* resample FFT to display width */
            fft_normalized_values = FFT::Resample(fft_normalized_values, conf.GetRate(), conf.GetWidth(),
                                                  conf.GetMinFreq(), conf.GetMaxFreq());
        } else {
            /* crop FFT to display width */
            fft_normalized_values = FFT::Crop(fft_normalized_values, conf.GetRate(),
                                              conf.GetMinFreq(), conf.GetMaxFreq());
        }

        /* colorize FFT */
        auto fft_colorized = color_map->Map(fft_normalized_values);

        /* add to live */
        if (live != nullptr) {
            live->AddWindow(fft_colorized, fft_normalized_values);
        }

        /* add to history */
        if (conf.GetFilename().has_value()) {
            history.push_back(fft_colorized);
        }
    }
    spdlog::info("Terminating ...");

    /* save file */
    if (conf.GetFilename().has_value()) {
        Renderer file_renderer(conf, history.size());

        /* render UI (axes etc) */
        file_renderer.RenderUserInterface();

        /* render legend */
        if (conf.HasLegend()) {
            std::vector<double> legend_values;
            for (std::size_t i = 0; i < conf.GetWidth(); i++) {
                legend_values.push_back((double) i / (double) (conf.GetWidth() - 1));
            }
            auto legend_colors = color_map->Map(legend_values);
            file_renderer.RenderLegend(legend_colors);
        }

        /* render windows */
        file_renderer.RenderFFTArea(history);

        file_renderer.GetCanvas().copyToImage().saveToFile(*conf.GetFilename());
    }

    /* all ok */
    return 0;
}