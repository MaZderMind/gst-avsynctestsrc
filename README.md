AV Sync-Test Src
================
URL: https://github.com/MaZderMind/gst-avsynctestsrc
Generates an Audio and a Video-Signal that have distinctive features happening at the exact same point in time, useful to detect Problems in the A/V Sync of long running Systems.

Boilerplate generated with https://gstreamer.mazdermind.de/

AV Sync-Test Video Src
----------------------
Generates the Video-Portion of the AV Sync-Test Signal.

AV Sync-Test Audio Src
----------------------
Generates the Audio-Portion of the AV Sync-Test Signal.

Install Build-Dependencies
--------------------------
```
apt install -y build-essential autoconf pkg-config libtool gstreamer1.0-tools libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libcairo2-dev
```

Getting-Started
---------------
```
cd avsynctestsrc
git init
git submodules init
./autogen.sh

cd examples
./inspect.sh
./run.sh
```
