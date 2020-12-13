/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "configuration.hpp"
#include "input.hpp"
#include "fft.hpp"

#include <spdlog/spdlog.h>
#include <iostream>
#include <csignal>

/* main loop exit condition */
volatile bool main_loop_running = true;

/*
 * SIGINT handler
 */
void
sigint_handler(int)
{
    main_loop_running = false;
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
    FFT fft(conf.GetFFTWidth());

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

    /* main loop */
    while (main_loop_running) {
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
        input->RemoveValues(std::max<std::size_t>(conf.GetFFTWidth(), conf.GetFFTStride()));

        /* compute FFT on fetched window */
        auto fft_values = fft.Compute(window_values);

        /* TODO: scale FFT to display width */

        /* TODO: colorize FFT */
    }
    spdlog::info("Terminating ...");

    /* TODO: save file */

    /* all ok */
    return 0;
}