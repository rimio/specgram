#!/bin/sh
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_jet.png -c jet
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_gray.png -c gray
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_purple.png -c purple
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_blue.png -c blue
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_green.png -c green
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_orange.png -c orange
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_red.png -c red

montage \
-label "jet" colormap_jet.png \
-label "gray" colormap_gray.png \
-label "purple" colormap_purple.png \
-label "blue" colormap_blue.png \
-label "green" colormap_green.png \
-label "orange" colormap_orange.png \
-label "red" colormap_red.png \
-geometry 200x colormaps.png

rm -rf colormap_*.png

specgram -eq -f 8192 -y 4000 --bg-color=00000000 --fg-color=000000ff -i ../clips/rammstein_dalai_lama example_file.png

# Following command should be used for screenshot acquisition
# cat ../clips/rammstein_dalai_lama | specgram -eql -f 8192 -y 4000 -k 440

montage \
-label "Live view" example_live.png \
-label "File output" example_file.png \
-geometry 400x example.png