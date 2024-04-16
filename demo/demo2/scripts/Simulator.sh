#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/amj/lib

../../../Simulator/src/Simulator \
    --sender-frames /frames:2:10000 \
    --sender-frames2 /frameviewer:2:10000 100\
    --receiver-delaylines 127.0.0.1:27003 \
    --framesize 64 64 \
    --baseline baseline1 1 2 200 1 2.5 1 1 1 \
    --baseline baseline2 2 3 250 1.5 3 1 1 2 \
    --externaldelaymodel 2 square 50 30 \
    --externaldelaymodel 3 cos 50 30
