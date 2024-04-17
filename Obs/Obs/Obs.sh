#!/bin/bash

export PATH=$PATH:/home/anders/nomad/packages/obs/DataProcessor/src/:/home/anders/nomad/packages/obs/FringeTracker/src:/home/anders/nomad/packages/obs/DelayController/src:/home/anders/nomad/packages/obs/TrackerController/TrackerController:/home/anders/nomad/packages/obs/FringeTrackerViewer/FringeTrackerViewer:/home/anders/nomad/packages/obs/FrameViewer/FrameViewer:/home/anders/nomad/packages/obs/DelaylineViewer/DelaylineViewer:/home/anders/nomad/packages/obs/PowerSpectrumViewer/PowerSpectrumViewer:/home/anders/nomad/packages/obs/PhasorViewer/PhasorViewer:/home/anders/nomad/packages/obs/Simulator/src

export LD_LIBRARY_PATH=/opt/amj/lib:/opt/amjCom/lib:/opt/QCustomPlot/lib

/home/anders/nomad/packages/obs/Obs/Obs/Obs
