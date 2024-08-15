#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/QCustomPlot/lib:/opt/amjChart/lib:/opt/amj/lib

../../../DelaylineViewer/DelaylineViewer/DelaylineViewer \
    --ndelaylines 6 --receiver-delaylines-delays 127.0.0.1:27007
