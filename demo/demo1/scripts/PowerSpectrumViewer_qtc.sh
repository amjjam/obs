#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/QCustomPlot/lib

../../PowerSpectrumViewer/build-PowerSpectrumViewer-Desktop-Debug/PowerSpectrumViewer \
--receiver-psec 127.0.0.1:27006
