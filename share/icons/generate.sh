#!/bin/sh

mkdir -p hicolor/16x16/apps
mkdir -p hicolor/24x24/apps
mkdir -p hicolor/32x32/apps
mkdir -p hicolor/48x48/apps
mkdir -p hicolor/64x64/apps
mkdir -p hicolor/96x96/apps
mkdir -p hicolor/128x128/apps

convert -geometry 16x high_res.png hicolor/16x16/apps/specgram.png
convert -geometry 24x high_res.png hicolor/24x24/apps/specgram.png
convert -geometry 32x high_res.png hicolor/32x32/apps/specgram.png
convert -geometry 48x high_res.png hicolor/48x48/apps/specgram.png
convert -geometry 64x high_res.png hicolor/64x64/apps/specgram.png
convert -geometry 96x high_res.png hicolor/96x96/apps/specgram.png
convert -geometry 128x high_res.png hicolor/128x128/apps/specgram.png