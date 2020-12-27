# specgram
Flexible live spectrogram tool.

## Build and install

If you are using ArchLinux you can grab the latest release from the AUR package [specgram](https://aur.archlinux.org/packages/specgram/), or get the master branch with [specgram-git](https://aur.archlinux.org/packages/specgram-git/).

Otherwise, you can build and install the program from the terminal:
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

## Usage

For a complete description of the program functionality please see the [manpage](man/specgram.1.pdf).

## Colormaps

![colormaps](resources/readme_images/colormaps.png?raw=true "Colormaps")