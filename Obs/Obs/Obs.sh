#!/bin/bash

export PATH=$PATH:/home/anders/nomad/projects/mroi/fourier_tracking/obs/DataProcessor/src/:/home/anders/nomad/projects/mroi/fourier_tracking/obs/FringeTracker/src:/home/anders/nomad/projects/mroi/fourier_tracking/obs/DelayController/src:/home/anders/nomad/projects/mroi/fourier_tracking/obs/TrackerController/build-TrackerController-Desktop-Debug/:/home/anders/nomad/projects/mroi/fourier_tracking/obs/FringeTrackerViewer/build-FringeTrackerViewer-Desktop-Debug:/home/anders/nomad/projects/mroi/fourier_tracking/obs/FrameViewer/build-FrameViewer-Desktop-Debug:/home/anders/nomad/projects/mroi/fourier_tracking/obs/DelaylineViewer/build-DelaylineViewer-Desktop-Debug/:/home/anders/nomad/projects/mroi/fourier_tracking/obs/PowerSpectrumViewer/build-PowerSpectrumViewer-Desktop-Debug/:/home/anders/nomad/projects/mroi/fourier_tracking/obs/PhasorViewer/build-PhasorViewer-Desktop-Release:/home/anders/nomad/projects/mroi/fourier_tracking/obs/Simulator/src

export LD_LIBRARY_PATH=/opt/amj/lib:/opt/amjCom/lib:/opt/QCustomPlot/lib

/home/anders/nomad/projects/mroi/fourier_tracking/obs/Obs/build-Obs-Desktop-Debug/Obs
