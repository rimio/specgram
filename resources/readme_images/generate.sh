#!/bin/sh
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_jet.png -c jet
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_gray.png -c gray
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_purple.png -c purple
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_blue.png -c blue
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_green.png -c green
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_orange.png -c orange
specgram -eqz --bg-color=00000000 --fg-color=000000ff -i ../clips/norah_jones_dont_know_why colormap_red.png -c red

for f in colormap_*.png
do
	convert -geometry 300x ${f} ${f}
done