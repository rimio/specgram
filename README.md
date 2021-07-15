# specgram

![build](https://github.com/rimio/specgram/workflows/build/badge.svg)

Small program that computes and plots spectrograms, either in a live window or to disk, with support for stdin input.

## Preview

![example](resources/readme_images/example.png?raw=true "Example")

## Build and install

If you are using ArchLinux you can grab the latest release from the AUR package [specgram](https://aur.archlinux.org/packages/specgram/), or get the main branch with [specgram-git](https://aur.archlinux.org/packages/specgram-git/).

Otherwise, you can build and install the program from source:
```bash
# Clone the repo
git clone https://github.com/rimio/specgram.git
cd specgram && mkdir build && cd build

# Build
cmake ..
make

# Install
sudo make install
```

## Dependencies

This program dynamically links against [FFTW](http://www.fftw.org/), [SFML 2.5](https://www.sfml-dev.org/), and [spdlog](https://github.com/gabime/spdlog).

The source code of [Taywee/args](https://github.com/Taywee/args) is embedded in the program (see ```src/args.hxx```).

*NOTE: SFML 2.5 is part of the official Ubuntu 20.04 package repository. Users using older versions of Ubuntu will have to compile SFML 2.5 from source.*

## Usage

For a complete description of the program functionality please see the [manpage](man/specgram.1.pdf).

### Input and output modes

**specgram** has two mutually exclusive input methods: from standard input and from file.

In order to generate a spectrogram from an input file ```infile``` and write the output to ```output.png```:

```bash
specgram -i infile outfile.png
```

If no input file is specified, then the default behaviour is to read input data *indefinitely* from standard input.
For example, we can query PulseAudio for audio sources:

```bash
$ pactl list sources short
1	alsa_output.usb-BEHRINGER_UMC204HD_192k-00.analog-surround-40.monitor	module-alsa-card.c	s16le 4ch 44100Hz	IDLE
2	alsa_input.usb-BEHRINGER_UMC204HD_192k-00.analog-stereo	module-alsa-card.c	s32le 2ch 44100Hz	SUSPENDED
3	alsa_output.pci-0000_00_1f.3.iec958-stereo.monitor	module-alsa-card.c	s16le 2ch 44100Hz	SUSPENDED
4	stereo.A.monitor	module-remap-sink.c	s16le 2ch 44100Hz	IDLE
5	stereo.B.monitor	module-remap-sink.c	s16le 2ch 44100Hz	SUSPENDED
11	alsa_output.pci-0000_01_00.1.hdmi-stereo.monitor	module-alsa-card.c	s16le 2ch 44100Hz	SUSPENDED

$ export PASOURCE="stereo.A.monitor"
```

In my case the default sink is ```stereo.A```, and I use the monitor source ```stereo.A.monitor``` to capture what I'm hearing in my headphones:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram outfile.png
[2020-12-27 15:56:15.058] [info] Creating 1024-wide FFTW plan
[2020-12-27 15:56:15.058] [info] Input stream: signed 16bit integer at 44100Hz
```

The program will keep reading data and computing FFTs until end of times or until it receives a ```SIGINT```.
In the Linux terminal this can be achieved by pressing ```CTRL+C```.

Once the signal is received, the program stops reading data from input and writes to ```outfile.png``` whatever it cached so far:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram outfile.png
[2020-12-27 15:57:29.967] [info] Creating 1024-wide FFTW plan
[2020-12-27 15:57:29.967] [info] Input stream: signed 16bit integer at 44100Hz
^C[2020-12-27 15:57:31.813] [info] Terminating ...
```

It is sometimes useful to see the spectrogram in real time. Live mode can be enabled with the ```-l, --live``` flag:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l outfile.png
```

The spectrogram will now be displayed as it is being compute from standard input.
When either ```SIGINT``` is received or the live window is closed, the program will terminate and write ```outfile.png```.

If file output is not desired, only live mode can be used, and nothing is written upon termination:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l
```

For obvious reasons, live mode cannot be used with file input.

### Input options

In the above examples we assumed that the program input is 16-bit signed integer at 44.1kHz, which happens to be what my sound card (and many others) outputs by default.

We can, however, specify any other rate with ```-r, --rate``` and datatype with ```-d, --datatype```:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw --format=float32 --rate=48000 | specgram -l -r 48000 -d f32
```

The above example will read 32-bit floating point input at 48kHz.
For a full list of supported data types see the [manpage](man/specgram.1.pdf).

**NOTE:** The specified rate is used only for display purposes and for interpreting other command line parameters.
There's nothing stopping us from using a different rate than the actual device rate, going down to nanohertz:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -r 1e-8
```

### FFT options

The FFT window width can be specified with ```-f, --fft_width``` and the stride, that is the distance between the beginning of two subsequent FFT windows, can be specified with ```-g, --fft_stride```:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -f 2048 -g 1024 
```

The above will compute 2048 elements wide FFT windows with a stride of 1024 elements, that is with a 50% overlap between windows.

Usually, a larger FFT window will give better frequency resolution but worse time resolution (i.e. it will be harder to locate signals in the time domain).

A smaller stride will give a *smoother* and richer output, but will strain the CPU more.

**NOTE:** You will notice that there isn't much difference between the output of the above command and the others.
That is because the display width is different from the FFT window width.
To change the display width, see **Display Options** below.

Lastly, if you encounter high sample rate signals, for which you can't display a wide enough (or often enough) window, you can use window averaging (```-A, --average```).

```bash
$ rx_sdr -d 0 -g 50 -f 97300000 -s 960000 -F CF32 - | ./specgram -lq -r 960000 -d cf32 -A 20 
```

The above example consumes input at 960k samples per second from a RTL-SDR dongle, which at a 1024 wide FFT window would mean displaying over 900 windows per second; a bit much for the average PC, and for the average human to follow.

Averaging 20 windows gives us a much more reasonable 47 windows per second.

### Display options

To change the display width we can use ```-w, --width```:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -f 1024 -w 1024
```

As you will notice, the spectrogram is somewhat blurry, because the program is resampling the 513 element wide positive part of the FFT output to the display width of 1024.
If you need sharp, crisp spectrograms, then you can use ```-q, --no_resampling``` to disable resampling:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -lq -f 1024
```
When not resampling, you can no longer specify the display width, as it is computed from the rest of the parameters.

Another use case is displaying a specific band of frequencies, using ```-x, --fmin``` and ```-y, --fmax``` to set the frequency bounds.
For example, to zoom in on the 500-3000Hz band:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -lq -f 9192 -x 500 -y 3000 
```

The colormap can be specified with ```-c, --colormap```; see image below the example for supported colormaps:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -c orange
```

![colormaps](resources/readme_images/colormaps.png?raw=true "Colormaps")

In order to see how to specify the background, foreground and custom colormap colors, please see the [manpage](man/specgram.1.pdf).

While the live view has both axes and legend enabled by default, file output does not.
To enable them use ```-a, --axes``` or ```-e, --legend```.
Please note that axes are implicit when displaying the legend.

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -le outfile.png
```

Finally, if you'd like to rotate the spectrogram 90 degrees counter-clockwise, so as to read it from left to right, you can use ```-z, --horizontal```:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -lz
```

This flag applies to both the live window and output file.

### Live options

Use ```-k, --count``` to control the number of FFT windows displayed in live view:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -k 128
```

Use ```-t, --title``` to specify the live window title:

```bash
$ parec --channels=1 --device="${PASOURCE}" --raw | specgram -l -t "My spectrogram"
```

## License

Copyright (c) 2020-2021 Vasile Vilvoiu \<vasi@vilvoiu.ro>

**specgram** is free software; you can redistribute it and/or modify it under the terms of the MIT license.
See LICENSE for details.

## Acknowledgements

Taywee/args library by Taylor C. Richberger and Pavel Belikov, released under the MIT license.

Program icon by Flavia Fabian, released under the CC-BY-SA 4.0 license.

Share Tech Mono font by Carrois Type Design, released under Open Font License.

Special thanks to Eugen Stoianovici for code review and various fixes.