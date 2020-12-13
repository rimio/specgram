/*
 * Copyright (c) 2020 Vasile Vilvoiu <vasi.vilvoiu@gmail.com>
 *
 * specgram is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "specgram.hpp"
#include "configuration.hpp"
#include "input.hpp"

#include <spdlog/spdlog.h>
#include <iostream>
#include <csignal>

/* main loop exit condition */
bool main_loop_running = true;

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

    /* create input parser */
    auto input = Input::FromDataType(conf.GetDataType(), conf.GetBlockSize());
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

    /* install SIGINT handler for CTRL+C */
    std::signal(SIGINT, sigint_handler);

    /* main loop */
    while (main_loop_running) {
        /* take whatever is available from input stream */
        auto r = input->ConsumeStream(std::cin);
        if (r > 0) {
            spdlog::info("Read {} bytes", r);
        }

        /* check if we have enough for a new FFT window */
        if ((input->GetBufferedValueCount() < conf.GetFFTWidth())
            || (input->GetBufferedValueCount() < conf.GetFFTStride())) {
            /* wait until we get enough values for a window and the spacing between windows */
            continue;
        }

        /* TODO: remove values that won't be used further */

        /* TODO: compute FFT */

        /* TODO: scale FFT to display width */

        /* TODO: colorize FFT */
    }
    spdlog::info("Exiting ...");

    /* TODO: save file */

    /* all ok */
    return 0;
}