#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/QCustomPlot/lib

../../PhasorViewer/build-PhasorViewer-Desktop-Release/PhasorViewer \
    --receiver-phasors 127.0.0.1:27002 \
    --baseline baseline1 256 1 2.5 \
    --baseline baseline2 256 1 2.5
